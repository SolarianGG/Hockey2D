#include <algorithm>
#include <chrono>
#include <memory>
#include <random>
#include <thread>

#include "game_data.hpp"
#include "mpr_utility.hpp"
#include "net_common.hpp"
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
namespace {

std::string GetLocalIPv4Address() {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    return {};
  }

  ULONG bufferSize = 15000;
  IP_ADAPTER_ADDRESSES* adapterAddresses =
      (IP_ADAPTER_ADDRESSES*)malloc(bufferSize);
  if (!adapterAddresses) {
    WSACleanup();
    return {};
  }

  DWORD result =
      GetAdaptersAddresses(AF_INET,
                           GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                               GAA_FLAG_SKIP_DNS_SERVER,
                           nullptr, adapterAddresses, &bufferSize);
  if (result != NO_ERROR) {
    free(adapterAddresses);
    WSACleanup();
    return {};
  }

  std::string localIP;

  for (IP_ADAPTER_ADDRESSES* adapter = adapterAddresses; adapter != nullptr;
       adapter = adapter->Next) {
    if (adapter->OperStatus != IfOperStatusUp) continue;

    for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress;
         unicast != nullptr; unicast = unicast->Next) {
      sockaddr_in* sa_in =
          reinterpret_cast<sockaddr_in*>(unicast->Address.lpSockaddr);
      DWORD ip = ntohl(sa_in->sin_addr.S_un.S_addr);

      if ((ip >> 24) == 127 || (ip >> 16) == 0xA9FE) continue;

      char ipStr[INET_ADDRSTRLEN];
      if (inet_ntop(AF_INET, &sa_in->sin_addr, ipStr, sizeof(ipStr))) {
        localIP = ipStr;
        break;
      }
    }

    if (!localIP.empty()) break;
  }

  free(adapterAddresses);
  WSACleanup();
  return localIP;
}

constexpr float kBaseSpeed = .005f;

constexpr float kFrictionCoefficient = 0.01f;

bool IsColliding(const mp::Vector2 aPos, const float aRadius,
                 const mp::Vector2 bPos, const float bRadius) {
  return (aPos - bPos).Length() < (aRadius + bRadius);
}

void CalculateCollisionResponse(mp::MoveableObject& lhs,
                                mp::MoveableObject& rhs) {
  const mp::Vector2 normal = (rhs.pos - lhs.pos).Normalize();
  const float relativeVelocity =
      (rhs.velocity - lhs.velocity).DotProduct(normal);
  if (relativeVelocity > 0) return;

  constexpr float restitution = 0.9f;

  const float impulseMagnitude =
      -(1 + restitution) * relativeVelocity / (1 / lhs.mass + 1 / rhs.mass);

  const mp::Vector2 impulse = impulseMagnitude * normal;

  lhs.velocity -= impulse / lhs.mass;
  rhs.velocity += impulse / rhs.mass;
}

void HandleCollisionWithBorder(mp::MoveableObject& c,
                               const mp::Vector2& leftRight,
                               const mp::Vector2& topBottom) {
  if (c.pos.x - c.radius < leftRight.x) {
    c.pos.x = leftRight.x + c.radius;
    c.velocity.x = -c.velocity.x;
  } else if (c.pos.x + c.radius > leftRight.y) {
    c.pos.x = leftRight.y - c.radius;
    c.velocity.x = -c.velocity.x;
  }

  if (c.pos.y - c.radius < topBottom.x) {
    c.pos.y = topBottom.x + c.radius;
    c.velocity.y = -c.velocity.y;
  } else if (c.pos.y + c.radius > topBottom.y) {
    c.pos.y = topBottom.y - c.radius;
    c.velocity.y = -c.velocity.y;
  }
}
void ResetPlayerPos(mp::Player& newPlayer,
                    std::uniform_real_distribution<>& distXCoordinate,
                    std::uniform_real_distribution<>& distYCoordinate,
                    std::mt19937& gen,
                    const std::vector<mp::Player>& players) {
  bool bFoundPos = false;
  newPlayer.transform.velocity = {0.0f, 0.0f};
  while (!bFoundPos) {
    const float x = distXCoordinate(gen);
    const float y = distYCoordinate(gen);
    const mp::Vector2 pos{x, y};
    bFoundPos = true;
    for (const auto& player : players) {
      if (IsColliding(pos, mp::Puck::baseRadius, player.transform.pos,
                      player.transform.radius)) {
        bFoundPos = false;
        break;
      }
    }
    newPlayer.transform.pos = pos;
  }
}
}  // namespace

int main(int argc, char* argv[]) {
  using namespace std::chrono_literals;
  // ranges for spawning players
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> distXCoordinate(
      mp::WorldState::leftRightLines.x + mp::Player::baseRadius,
      mp::WorldState::leftRightLines.y - mp::Player::baseRadius);
  std::uniform_real_distribution<> teamOneYDist(
      mp::WorldState::teamsGoalsY.x + mp::Player::baseRadius, -0.4f);
  std::uniform_real_distribution<> teamTwoYDist(
      0.4f, mp::WorldState::teamsGoalsY.y - mp::Player::baseRadius);
  std::array teamsYDistances{teamOneYDist, teamTwoYDist};
  // ---

  mp::EnetInit();
  assert(0 == atexit(enet_deinitialize));

  constexpr int kMaxPlayers = 10;
  constexpr int kMaxChannels = 2;
  constexpr std::uint16_t kPort = 5000;

  const std::string localIp = GetLocalIPv4Address();
  const ENetAddress address = mp::EnetCreateAddress(kPort, localIp.c_str());
  auto host = mp::EnetCreateHost(&address, kMaxPlayers, kMaxChannels);
  assert(host.get());
  ENetEvent event;

  mp::WorldState worldState;
  mp::PacketHandler packetHandler;

  packetHandler.RegisterHandler<mp::Player>(
      mp::PacketType::PlayerInputUpdate, [&worldState](const mp::Player& data) {
        auto& player = *mp::FindRequiredPlayer(
            worldState.players.begin(), worldState.players.end(), data.id);
        player.transform.velocity = data.transform.velocity;
      });
  packetHandler.RegisterHandler<std::uint32_t>(
      mp::PacketType::Disconnect, [&worldState](const std::uint32_t& id) {
        auto it = mp::FindRequiredPlayer(worldState.players.begin(),
                                         worldState.players.end(), id);
        assert(it != worldState.players.end());
        *it = worldState.players.back();
        worldState.players.pop_back();
        std::cout << "On disconnect, id: " << id << '\n';
      });

  bool bIsRunning = true;
  std::uint32_t currentPlayerId = 0;
  std::cout << "Server is running, ip: " << localIp << ", port: " << kPort << "\n";
  while (bIsRunning) {
    // Receive input from players
    while (enet_host_service(host.get(), &event, 0) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_NONE: {
          std::cout << "None\n";
        } break;
        case ENET_EVENT_TYPE_CONNECT: {
          std::cout << "OnConnect\n";
          mp::Player newPlayer{
              .id = currentPlayerId,
              .teamId = currentPlayerId % 2,
          };
          // Find appropriate pos for new player
          ResetPlayerPos(newPlayer, distXCoordinate,
                         teamsYDistances[newPlayer.teamId], gen,
                         worldState.players);
          currentPlayerId++;
          worldState.players.push_back(newPlayer);
          mp::SendPacket(event.peer, mp::PacketType::Connect, newPlayer);
        } break;
        case ENET_EVENT_TYPE_DISCONNECT: {
          std::cout << "OnDisconnect\n";
        } break;
        case ENET_EVENT_TYPE_RECEIVE: {
          std::cout << "OnReceive\n";
          mp::HandlePacket(event.packet, packetHandler);
          enet_packet_destroy(event.packet);
        } break;
      }
    }

    // Update
    for (auto& player : worldState.players) {
      // colliding with puck
      if (IsColliding(player.transform.pos, player.transform.radius,
                      worldState.puck.transform.pos,
                      worldState.puck.transform.radius)) {
        CalculateCollisionResponse(player.transform, worldState.puck.transform);
      }

      // colliding with other players
      for (auto& otherPlayer : worldState.players) {
        if (otherPlayer.id > player.id &&
            IsColliding(player.transform.pos, player.transform.radius,
                        otherPlayer.transform.pos,
                        otherPlayer.transform.radius)) {
          CalculateCollisionResponse(player.transform, otherPlayer.transform);
        }
      }

      // Check and update collision with the border
      HandleCollisionWithBorder(player.transform,
                                mp::WorldState::fieldBorders[0],
                                mp::WorldState::fieldBorders[1]);

      // update pos
      player.transform.pos += player.transform.velocity * kBaseSpeed;

      // calculate friction
      player.transform.velocity *= 0.99f;
    }

    // check for the goal
    bool bIsGoal = false;
    if (worldState.puck.transform.pos.y < mp::WorldState::teamsGoalsY.x) {
      worldState.goals[1]++;
      bIsGoal = true;
    } else if (worldState.puck.transform.pos.y >
               mp::WorldState::teamsGoalsY.y) {
      worldState.goals[0]++;
      bIsGoal = true;
    }
    if (bIsGoal) {
      worldState.puck.transform.pos = {0.0f, 0.0f};
      worldState.puck.transform.velocity = {0.0f, 0.0f};
      for (auto& player : worldState.players) {
        ResetPlayerPos(player, distXCoordinate, teamsYDistances[player.teamId],
                       gen, worldState.players);
      }
    }

    // Check and update collision with the border
    HandleCollisionWithBorder(worldState.puck.transform,
                              mp::WorldState::fieldBorders[0],
                              mp::WorldState::fieldBorders[1]);

    // update puck pos
    worldState.puck.transform.pos +=
        worldState.puck.transform.velocity * kBaseSpeed;
    // update puck friction
    worldState.puck.transform.velocity *= 0.99f;

    // replicate world state
    mp::BroadcastPacket(host.get(), mp::PacketType::WorldState, worldState);
    std::this_thread::sleep_for(10ms);
  }

  return 0;
}
