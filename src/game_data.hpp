#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>

#include "net_common.hpp"

namespace mp {

struct Vector2 {
  float x, y;

  template <typename T>
    requires(std::is_arithmetic_v<T>)
  constexpr Vector2& operator*=(const T val) {
    x *= val;
    y *= val;
    return *this;
  }
  template <typename T>
    requires(std::is_arithmetic_v<T>)
  constexpr Vector2& operator/=(const T val) {
    x /= val;
    y /= val;
    return *this;
  }
  constexpr Vector2& operator+=(const Vector2& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  constexpr Vector2& operator-=(const Vector2& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  [[nodiscard]]
  Vector2 operator-() const {
    return {.x = -x, .y = -y};
  }

  [[nodiscard]]
  float Length() const {
    return std::sqrtf(x * x + y * y);
  }

  [[nodiscard]]
  constexpr float LengthDoubled() const {
    return x * x + y * y;
  }

  [[nodiscard]]
  constexpr float DotProduct(const Vector2& rhs) const {
    return x * rhs.x + y * rhs.y;
  }

  [[nodiscard]]
  Vector2 Normalize() const {
    const float length = Length();
    return {.x = x / length, .y = y / length};
  }

  SERIALIZABLE(x, y)
};

template <typename T>
  requires(std::is_arithmetic_v<T>)
constexpr Vector2 operator*(Vector2 lhs, const T rhs) {
  lhs *= rhs;
  return lhs;
}

template <typename T>
  requires(std::is_arithmetic_v<T>)
constexpr Vector2 operator/(Vector2 lhs, const T rhs) {
  lhs /= rhs;
  return lhs;
}
template <typename T>
  requires(std::is_arithmetic_v<T>)
constexpr Vector2 operator*(const T rhs, Vector2 lhs) {
  lhs *= rhs;
  return lhs;
}

template <typename T>
  requires(std::is_arithmetic_v<T>)
constexpr Vector2 operator/(const T rhs, Vector2 lhs) {
  lhs /= rhs;
  return lhs;
}
[[nodiscard]] constexpr Vector2 operator+(Vector2 rhs, Vector2 lhs) {
  rhs += lhs;
  return rhs;
}

[[nodiscard]] constexpr Vector2 operator-(Vector2 rhs, Vector2 lhs) {
  rhs -= lhs;
  return rhs;
}

struct MoveableObject {
  Vector2 pos{0.0f, 0.0f};
  Vector2 velocity{0.0f, 0.0f};
  float radius{.05f};
  float mass{.05f};
  SERIALIZABLE(pos, velocity, radius, mass)
};

struct Player {
  static constexpr float baseRadius = 0.05f;
  std::uint32_t id{~0u};
  std::uint32_t teamId{~0u};
  MoveableObject transform{.radius = baseRadius, .mass = .025f};

  SERIALIZABLE(id, teamId, transform)
};

struct Puck {
  static constexpr float baseRadius = 0.02f;
  MoveableObject transform{.radius = baseRadius, .mass = .01f};
  SERIALIZABLE(transform)
};

struct WorldState {
  std::vector<Player> players;
  Puck puck{};
  std::uint32_t goals[2]{};
  static constexpr mp::Vector2 leftRightLines{-.98f, .95f};
  static constexpr mp::Vector2 teamsGoalsY{-0.78f, 0.78f};
  static constexpr mp::Vector2 fieldBorders[2]{leftRightLines, {-.98f, .88f}};

  SERIALIZABLE(players, puck, goals)
};
}  // namespace mp
