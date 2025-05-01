// clang-format off
#include "net_common.hpp"
#include "game_data.hpp"
#include "mpr_utility.hpp"
#include "mpr_window.hpp"
#include "connect_dialog.hpp"

#include <Windows.h>
#include <gdiplus.h>
#include <windowsx.h>

#include <memory>
#include <algorithm>
#include <format>
#include <iostream>

// clang-format on

namespace {
constexpr auto kWindowWidth{800};
constexpr auto kWindowHeight{600};

void DrawCircleObject(mp::Window& window, const auto& p,
                      const COLORREF c = RGB(255, 255, 255),
                      const COLORREF o = RGB(0, 0, 0)) {
  window.DrawCircle(p.transform.radius, p.transform.pos, c, o);
}

}  // namespace

int APIENTRY wWinMain(_In_ const HINSTANCE hInstance,
                      _In_opt_ const HINSTANCE hPrevInstance,
                      _In_ const LPWSTR lpCmdLine,
                      _In_ const int nShowCmd) try {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  mp::PacketHandler packetHandler;
  mp::WorldState worldState;
  std::uint32_t thisPlayerId{~0u};
  packetHandler.RegisterHandler<mp::Player>(
      mp::PacketType::Connect,
      [&thisPlayerId, &worldState](const mp::Player& data) {
        thisPlayerId = data.id;
        worldState.players.push_back(data);
        std::cout << "Handled connect packet, our player id: " << data.id
                  << " team id: " << data.teamId << "\n";
      });
  packetHandler.RegisterHandler<mp::WorldState>(
      mp::PacketType::WorldState,
      [&worldState](const mp::WorldState& world) { worldState = world; });

  mp::EnetInit();
  assert(0 == atexit(enet_deinitialize));
  auto host = mp::EnetCreateHost(nullptr, 1, 2);
  assert(host.get());

  bool bIsReady = false;
  ENetEvent event;
  ENetPeer* peer = nullptr;
  mp::ConnectionInfo connection;
  while (!bIsReady) {
    try {
      if (!mp::ShowConnectionDialog(hInstance, nullptr, connection)) {
        return 0;
      }
      bIsReady = true;
      const ENetAddress serverAddr =
          mp::EnetCreateAddress(connection.port, connection.ip.c_str());
      peer = enet_host_connect(host.get(), &serverAddr, 2, 0);
      assert(peer);
      if (enet_host_service(host.get(), &event, 5000) > 0 &&
          event.type == ENET_EVENT_TYPE_CONNECT) {
        MessageBoxA(nullptr, "Connection successful!", "Ok!", MB_OK);
      } else {
        enet_peer_reset(peer);
        MessageBoxA(nullptr, "Connection failed, try again!", "Ooops!", MB_OK);
        bIsReady = false;
      }
    } catch (const std::exception& e) {
      MessageBoxA(nullptr, e.what(), "Error", MB_OK);
      bIsReady = false;
    }
  }

  if (enet_host_service(host.get(), &event, 5000) > 0 &&
      event.type == ENET_EVENT_TYPE_RECEIVE) {
    mp::HandlePacket(event.packet, packetHandler);
    enet_packet_destroy(event.packet);
    std::cout << "Got a required packet\n";
  } else {
    throw std::runtime_error("Unexpected packet type");
  }
  const Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

  mp::Window window(hInstance, nShowCmd, kWindowWidth, kWindowHeight);
  bool bIsRunning = true;
  bool bNeedToDisconnect = true;

  MSG msg{};
  while (bIsRunning) {
    // poll events
    auto& currentPlayer = *mp::FindRequiredPlayer(
        worldState.players.begin(), worldState.players.end(), thisPlayerId);
    bool bHasPlayerModified = false;
    if (msg.message == WM_QUIT) bIsRunning = false;
    if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    mp::Event e;
    while (window.PollEvent(e)) {
      if (e.type == mp::EventType::Released) {
        if (e.key == mp::KeyboardKey::A) {
          currentPlayer.transform.velocity.x -= 1.f;
        }
        if (e.key == mp::KeyboardKey::D) {
          currentPlayer.transform.velocity.x += 1.f;
        }
        if (e.key == mp::KeyboardKey::W) {
          currentPlayer.transform.velocity.y -= 1.f;
        }
        if (e.key == mp::KeyboardKey::S) {
          currentPlayer.transform.velocity.y += 1.f;
        }

        if (currentPlayer.transform.velocity.LengthDoubled() > 0.0f)
          currentPlayer.transform.velocity =
              currentPlayer.transform.velocity.Normalize();
        bHasPlayerModified = true;
      }
    }
    // Send Input to the server
    if (bHasPlayerModified) {
      mp::SendPacket(peer, mp::PacketType::PlayerInputUpdate, currentPlayer, 0,
                     1);
    }
    // get world state
    while (enet_host_service(host.get(), &event, 0) > 0) {
      switch (event.type) {
        case ENET_EVENT_TYPE_NONE:
          std::cout << "none\n";
          break;
        case ENET_EVENT_TYPE_CONNECT:
          std::cout << "connect from server\n";
          break;
        case ENET_EVENT_TYPE_DISCONNECT:
          std::cout << "Server dropped its connection\n";
          bIsRunning = false;
          bNeedToDisconnect = false;
          break;
        case ENET_EVENT_TYPE_RECEIVE:
          mp::HandlePacket(event.packet, packetHandler);
          enet_packet_destroy(event.packet);
          break;
      }
    }

    // draw
    window.Clear(RGB(34, 66, 99));

    window.DrawLine(
        {mp::WorldState::leftRightLines.x, mp::WorldState::teamsGoalsY.x},
        {mp::WorldState::leftRightLines.y, mp::WorldState::teamsGoalsY.x},
        RGB(0, 0, 255));
    window.DrawLine(
        {mp::WorldState::leftRightLines.x, mp::WorldState::teamsGoalsY.y},
        {mp::WorldState::leftRightLines.y, mp::WorldState::teamsGoalsY.y},
        RGB(255, 0, 0));

    window.DrawField(mp::WorldState::fieldBorders[0],
                     mp::WorldState::fieldBorders[1]);

    for (const auto& player : worldState.players) {
      COLORREF fillColor;
      if (player.teamId == 1) {
        fillColor = RGB(255, 0, 0);
      } else {
        fillColor = RGB(0, 0, 255);
      }

      COLORREF outlineColor = RGB(0, 0, 0);
      if (player.id == thisPlayerId) {
        outlineColor = RGB(0, 255, 0);
      }
      DrawCircleObject(window, player, fillColor, outlineColor);
    }
    DrawCircleObject(window, worldState.puck, RGB(0, 255, 0), RGB(255, 255, 0));

    window.DrawLabel(
        std::format("{}:{}", worldState.goals[0], worldState.goals[1]),
        {-0.05f, -1.0f});

    window.Display();

    // --- end frame
  }

  // disconnect if server is alive, otherwise just exit
  if (bNeedToDisconnect) {
    std::cout << "Disconnecting\n";
    mp::SendPacket(peer, mp::PacketType::Disconnect, thisPlayerId);
    enet_host_flush(host.get());
    enet_host_service(host.get(), &event, 5000);
    enet_peer_disconnect(peer, 0);
    do {
      enet_host_service(host.get(), &event, 5000);
    } while (event.type != ENET_EVENT_TYPE_DISCONNECT);
  }

  return 0;
} catch (const std::exception& e) {
  MessageBoxA(nullptr, e.what(), "Error", MB_OK);
}
