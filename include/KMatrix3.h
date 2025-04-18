#pragma once
#include <math.h>

#include "KVector2.h"

class KMatrix3 {
 public:
  double _11, _12, _13;
  double _21, _22, _23;
  double _31, _32, _33;

 public:
  static constexpr KMatrix3 Zero() {
    return KMatrix3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  }
  static constexpr KMatrix3 Identity() {
    return KMatrix3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
  }

 public:
  explicit constexpr KMatrix3(const double e11 = 1.0, const double e12 = 0.0,
                              const double e13 = 0.0, const double e21 = 0.0,
                              const double e22 = 1.0, const double e23 = 0.0,
                              const double e31 = 0.0, const double e32 = 0.0,
                              const double e33 = 1.0) {
    _11 = e11;
    _12 = e12;
    _13 = e13;
    _21 = e21;
    _22 = e22;
    _23 = e23;
    _31 = e31;
    _32 = e32;
    _33 = e33;
  }
  constexpr void Set(const double e11, const double e12, const double e13,
                     const double e21, const double e22, const double e23,
                     const double e31, const double e32, const double e33) {
    _11 = e11;
    _12 = e12;
    _13 = e13;
    _21 = e21;
    _22 = e22;
    _23 = e23;
    _31 = e31;
    _32 = e32;
    _33 = e33;
  }

  constexpr void SetIdentity() {
    _11 = 1.0f;
    _12 = 0.0f;
    _13 = 0.0f;
    _21 = 0.0f;
    _22 = 1.0f;
    _23 = 0.0f;
    _31 = 0.0f;
    _32 = 0.0f;
    _33 = 1.0f;
  }

  void SetRotation(const double theta) {
    SetIdentity();
    _11 = cos(theta);
    _12 = -sin(theta);
    _21 = sin(theta);
    _22 = cos(theta);
  }

  constexpr void SetShear(const double shearXParallelToY,
                          const double shearYParallelToX) {
    SetIdentity();
    _11 = 1.0f;
    _12 = shearYParallelToX;
    _21 = shearXParallelToY;
    _22 = 1.0f;
  }

  constexpr void SetScale(const double uniformScale) {
    SetIdentity();
    _11 = uniformScale;
    _22 = uniformScale;
    _33 = uniformScale;
  }

  constexpr void SetTranslation(const double tx, const double ty) {
    SetIdentity();
    _13 = tx;
    _23 = ty;
  }

  constexpr bool GetBasis(KVector2& basis, const int basisIndexFrom0) const {
    if (basisIndexFrom0 == 0) {
      basis.x = _11;
      basis.y = _21;
    } else if (basisIndexFrom0 == 1) {
      basis.x = _12;
      basis.y = _22;
    } else {
      return false;
    }

    return true;
  }
};

constexpr KVector2 operator*(const KVector2& v, const KMatrix3& m) {
  KVector2 temp;
  temp.x = v.x * m._11 + v.y * m._21 + 1.0f * m._31;
  temp.y = v.x * m._12 + v.y * m._22 + 1.0f * m._32;
  const double z = v.x * m._13 + v.y * m._23 + 1.0f * m._33;
  temp.x /= z;  // homogeneous divide
  temp.y /= z;
  return temp;
}

constexpr KVector2 operator*(const KMatrix3& m, const KVector2& v) {
  KVector2 temp;
  temp.x = m._11 * v.x + m._12 * v.y + m._13 * 1.0f;
  temp.y = m._21 * v.x + m._22 * v.y + m._23 * 1.0f;
  const double z = m._31 * v.x + m._32 * v.y + m._33 * 1.0f;
  temp.x /= z;  // homogeneous divide
  temp.y /= z;
  return temp;
}

constexpr KMatrix3 operator*(const double scalar, const KMatrix3& m) {
  KMatrix3 temp;
  temp._11 = scalar * m._11;
  temp._12 = scalar * m._12;
  temp._13 = scalar * m._13;
  temp._21 = scalar * m._21;
  temp._22 = scalar * m._22;
  temp._23 = scalar * m._23;
  temp._31 = scalar * m._31;
  temp._32 = scalar * m._32;
  temp._33 = scalar * m._33;
  return temp;
}

// composition: matrix-matrix multiplication
constexpr KMatrix3 operator*(const KMatrix3& m0, const KMatrix3& m1) {
  KMatrix3 temp;
  temp._11 = m0._11 * m1._11 + m0._12 * m1._21 + m0._13 * m1._31;
  temp._12 = m0._11 * m1._12 + m0._12 * m1._22 + m0._13 * m1._32;
  temp._13 = m0._11 * m1._13 + m0._12 * m1._23 + m0._13 * m1._33;
  temp._21 = m0._21 * m1._11 + m0._22 * m1._21 + m0._23 * m1._31;
  temp._22 = m0._21 * m1._12 + m0._22 * m1._22 + m0._23 * m1._32;
  temp._23 = m0._21 * m1._13 + m0._22 * m1._23 + m0._23 * m1._33;
  temp._31 = m0._31 * m1._11 + m0._32 * m1._21 + m0._33 * m1._31;
  temp._32 = m0._31 * m1._12 + m0._32 * m1._22 + m0._33 * m1._32;
  temp._33 = m0._31 * m1._13 + m0._32 * m1._23 + m0._33 * m1._33;
  return temp;
}
