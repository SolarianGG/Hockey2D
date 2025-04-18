#pragma once
#include <math.h>
#include <stdlib.h>

class KVector2 {
 public:
  static constexpr KVector2 Zero() { return {0, 0}; };
  static constexpr KVector2 One() { return {1, 1}; };
  static constexpr KVector2 Right() { return {1, 0}; };
  static constexpr KVector2 Up() { return {0, 1}; }

  [[nodiscard]]
  static constexpr KVector2 Lerp(const KVector2& begin, const KVector2& end,
                                 const float ratio_) {
    const float ratio = __min(1, __max(0, ratio_));
    KVector2 temp;
    temp.x = begin.x + (end.x - begin.x) * ratio;
    temp.y = begin.y + (end.y - begin.y) * ratio;
    return temp;
  }

 public:
  double x;
  double y;

 public:
  explicit constexpr KVector2(const double tx = 0.0, const double ty = 0.0) {
    x = tx;
    y = ty;
  }
  constexpr KVector2(const int tx, const int ty) {
    x = static_cast<double>(tx);
    y = static_cast<double>(ty);
  }
  [[nodiscard]] double Length() const { return sqrt(x * x + y * y); }
  [[nodiscard]] KVector2 Normalize() const {
    const double length = Length();
    const KVector2 temp(x / length, y / length);
    return temp;
  }
};

constexpr KVector2 operator+(const KVector2& lhs, const KVector2& rhs) {
  const KVector2 temp(lhs.x + rhs.x, lhs.y + rhs.y);
  return temp;
}

constexpr KVector2 operator-(const KVector2& lhs, const KVector2& rhs) {
  const KVector2 temp(lhs.x - rhs.x, lhs.y - rhs.y);
  return temp;
}

constexpr KVector2 operator*(const double scalar, const KVector2& rhs) {
  const KVector2 temp(scalar * rhs.x, scalar * rhs.y);
  return temp;
}

constexpr KVector2 operator*(const KVector2& lhs, const double scalar) {
  const KVector2 temp(scalar * lhs.x, scalar * lhs.y);
  return temp;
}

constexpr KVector2& operator+=(KVector2& lhs, const KVector2& rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}

constexpr KVector2& operator-=(KVector2& lhs, const KVector2& rhs) {
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  return lhs;
}
