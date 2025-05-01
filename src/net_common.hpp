#pragma once

// clang-format off
#include "cereal/archives/portable_binary.hpp"
#include "cereal/types/array.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/unordered_map.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/details/traits.hpp"

#include <cassert>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "enet.h"

// clang-format on

#define SERIALIZABLE(...)       \
  template <typename Archive>   \
  void serialize(Archive& ar) { \
    ar(__VA_ARGS__);            \
  }

namespace mp {

enum class PacketType : std::uint8_t {
  Connect,
  Disconnect,
  PlayerInputUpdate,
  WorldState,
};

class PacketHandler {
 public:
  using Archive = cereal::PortableBinaryInputArchive;
  using FunctionHandler = std::function<void(Archive&)>;

  template <typename T>
    requires(cereal::traits::is_input_serializable<T, Archive>::value)
  void RegisterHandler(const PacketType type,
                       const std::function<void(const T&)>& callback) {
    bindings_[type] = [callback](Archive& ar) {
      T data;
      ar(data);
      callback(data);
    };
  }

  void Handle(const PacketType type, Archive& ar) {
    if (const auto it = bindings_.find(type); it != bindings_.end()) {
      it->second(ar);
    } else {
      throw std::runtime_error("Trying to handle packet on unbind packet type");
    }
  }

  void EraseHandler(const PacketType type) { bindings_.erase(type); }

 private:
  std::unordered_map<PacketType, FunctionHandler> bindings_;
};

inline ENetAddress EnetCreateAddress(const std::uint16_t port,
                                     const char* addr = "127.0.0.1") {
  ENetAddress address{.port = port};
  if (enet_address_set_host_ip(&address, addr) != 0) {
    throw std::runtime_error("Incorrect ip or port");
  }
  return address;
}

inline void EnetInit() {
  if (0 != enet_initialize()) {
    throw std::runtime_error("Failed to init enet");
  }
}

inline auto EnetCreateHost(const ENetAddress* address,
                           const std::size_t numConnections,
                           const std::size_t numChannels) {
  static auto hostDeleter = [](ENetHost* host) { enet_host_destroy(host); };
  return std::unique_ptr<ENetHost, decltype(hostDeleter)>{
      enet_host_create(address, numConnections, numChannels, 0, 0)};
}

template <typename T>
  requires(cereal::traits::is_output_serializable<
           T, cereal::PortableBinaryOutputArchive>::value)
ENetPacket* PreparePacket(PacketType type, T&& data,
                          const std::uint32_t transferType) {
  std::stringstream ss;
  cereal::PortableBinaryOutputArchive ar(ss);

  ar(type);
  ar(std::forward<T>(data));

  const std::string buf = ss.str();
  return enet_packet_create(buf.data(), buf.size(), transferType);
}

template <typename T>
  requires(cereal::traits::is_output_serializable<
           T, cereal::PortableBinaryOutputArchive>::value)
void SendPacket(ENetPeer* peer, PacketType type, T&& data,
                const std::uint32_t transferType = ENET_PACKET_FLAG_RELIABLE,
                const std::uint32_t channel = 0) {
  auto* packet = PreparePacket(type, std::forward<T>(data), transferType);
  assert(packet);
  enet_peer_send(peer, channel, packet);
}

template <typename T>
  requires(cereal::traits::is_output_serializable<
           T, cereal::PortableBinaryOutputArchive>::value)
void BroadcastPacket(
    ENetHost* host, PacketType type, T&& data,
    const std::uint32_t transferType = ENET_PACKET_FLAG_RELIABLE,
    const std::uint32_t channel = 0) {
  auto* packet = PreparePacket(type, std::forward<T>(data), transferType);
  assert(packet);
  enet_host_broadcast(host, channel, packet);
}

inline void HandlePacket(const ENetPacket* packet, PacketHandler& handler) {
  std::stringstream ss;
  ss.write(reinterpret_cast<const char*>(packet->data),
           static_cast<long long>(packet->dataLength));
  PacketHandler::Archive ar(ss);

  PacketType type;
  ar(type);

  handler.Handle(type, ar);
}
}  // namespace mp
