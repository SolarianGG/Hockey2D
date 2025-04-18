#pragma once
struct KComplex {
  double r;
  double i;

  explicit constexpr KComplex(const double tr = 0.0, const double ti = 0.0)
      : r(tr), i(ti) {}
  [[nodiscard]]
  double Length() const {
    return sqrt(r * r + i * i);
  }
  [[nodiscard]] KComplex Normalize() const {
    const double length = Length();
    const KComplex temp(r / length, i / length);
    return temp;
  }
};

constexpr KComplex operator+(const KComplex a, const KComplex b) {
  KComplex t;
  t.r = a.r + b.r;
  t.i = a.i + b.i;
  return t;
}

constexpr KComplex operator+(const double r, const KComplex b) {
  KComplex t;
  t.r = r + b.r;
  t.i = 0.0 + b.i;
  return t;
}

constexpr KComplex operator+(const KComplex a, const double r) {
  KComplex t;
  t.r = a.r + r;
  t.i = a.i + 0.0;
  return t;
}

constexpr KComplex operator-(const KComplex a) {
  KComplex t;
  t.r = -a.r;
  t.i = -a.i;
  return t;
}

constexpr KComplex operator-(const KComplex a, const KComplex b) {
  KComplex t;
  t.r = a.r - b.r;
  t.i = a.i - b.i;
  return t;
}

constexpr KComplex operator-(const double r, const KComplex b) {
  KComplex t;
  t.r = r - b.r;
  t.i = 0.0 - b.i;
  return t;
}

constexpr KComplex operator-(const KComplex a, const double r) {
  KComplex t;
  t.r = a.r - r;
  t.i = a.i - 0.0;
  return t;
}

constexpr KComplex operator*(const KComplex a, const KComplex b) {
  KComplex t;
  t.r = a.r * b.r - a.i * b.i;
  t.i = a.r * b.i + a.i * b.r;
  return t;
}

constexpr KComplex operator*(const double r, const KComplex b) {
  KComplex t;
  t.r = r * b.r - 0.0 * b.i;
  t.i = r * b.i + 0.0 * b.r;
  return t;
}

constexpr KComplex operator*(const KComplex a, const double r) {
  KComplex t;
  t.r = a.r * r - a.i * 0.0;
  t.i = a.r * 0.0 + a.i * r;
  return t;
}
