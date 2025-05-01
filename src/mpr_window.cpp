#include "mpr_window.hpp"

#include <MMSystem.h>
#include <gdiplus.h>
#include <windowsx.h>

#include <cassert>
#include <stdexcept>
#include <unordered_map>
using namespace Gdiplus;

namespace {
mp::Window* currentAliveWindow = nullptr;
}
namespace mp {
Window::Window(HINSTANCE instance, int nShowCmd, UINT windowWidth,
               UINT windowHeight, std::string_view windowTitle)
    : hInstance_(instance) {
  const WNDCLASS wc{
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = SWndProc,
      .cbClsExtra = 0,
      .cbWndExtra = 0,
      .hInstance = hInstance_,
      .hIcon = LoadIcon(nullptr, IDI_APPLICATION),
      .hCursor = LoadCursor(nullptr, IDC_ARROW),
      .hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)),
      .lpszMenuName = nullptr,
      .lpszClassName = TEXT("BasicWNDClass"),
  };

  if (!RegisterClass(&wc)) {
    throw std::runtime_error("Failed to register class");
  }

  RECT rc{0, 0, static_cast<LONG>(windowWidth),
          static_cast<LONG>(windowHeight)};
  AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);
  window_ = CreateWindow(TEXT("BasicWNDClass"), windowTitle.data(),
                         WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT,
                         CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
                         nullptr, nullptr, hInstance_, nullptr);
  if (!window_) {
    throw std::runtime_error("Failed to create window");
  }
  width_ = rc.right - rc.left + 1;
  height_ = rc.bottom - rc.top + 1;
  const HDC hdc = ::GetDC(window_);
  hdc_ = CreateCompatibleDC(hdc);
  bitmap_ = CreateCompatibleBitmap(hdc, width_, height_);
  assert(hdc_);
  assert(bitmap_);
  SelectObject(hdc_, bitmap_);

  ShowWindow(window_, nShowCmd);
  UpdateWindow(window_);
  currentAliveWindow = this;
}

void Window::Clear(const COLORREF color) {
  const HBRUSH brush = CreateSolidBrush(color);
  SelectObject(hdc_, brush);
  Rectangle(hdc_, 0, 0, width_, height_);
  DeleteObject(brush);
}

void Window::Display() {
  const HDC hdc = ::GetDC(window_);

  BitBlt(hdc, 0, 0, width_, height_, hdc_, 0, 0, SRCCOPY);
  ::ReleaseDC(window_, hdc);
}

void Window::SetWindowTitle(std::string_view title) {
  SetWindowTextA(window_, title.data());
}

void Window::DrawImage(Gdiplus::Image* image, mp::Vector2 pos,
                       const float degree) {
  if (!image) return;
  const int imageWidth = static_cast<int>(image->GetWidth());
  const int imageHeight = static_cast<int>(image->GetHeight());

  Transform(pos.x, pos.y);

  Graphics graphics(hdc_);
  graphics.SetSmoothingMode(SmoothingModeHighQuality);
  graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

  // push transform to the stack
  graphics.TranslateTransform(static_cast<REAL>(pos.x),
                              static_cast<REAL>(pos.y));
  graphics.RotateTransform(static_cast<REAL>(degree));
  graphics.TranslateTransform(static_cast<REAL>(-imageWidth) / 2,
                              static_cast<REAL>(-imageHeight) / 2);

  graphics.DrawImage(image, 0, 0, imageWidth, imageHeight);

  // reset transform
  graphics.ResetTransform();
}

void Window::DrawCircle(float radius, mp::Vector2 pos, COLORREF fillColor, COLORREF outlineColor) {
  HPEN hPen = CreatePen(PS_SOLID, 3, outlineColor);
  HPEN hOldPen = static_cast<HPEN>(SelectObject(hdc_, hPen));
  const HBRUSH brush = CreateSolidBrush(fillColor);
  SelectObject(hdc_, brush);
  const float kPixelPerUnitX{width_ / 2.0f};
  radius *= kPixelPerUnitX;
  Transform(pos.x, pos.y);
  Ellipse(hdc_, pos.x - radius, pos.y - radius, pos.x + radius, pos.y + radius);
  SelectObject(hdc_, hOldPen);
  DeleteObject(brush);
  DeleteObject(hPen);
}

void Window::DrawLabel(std::string_view text, mp::Vector2 pos, COLORREF c) {
  const HBRUSH brush = CreateSolidBrush(c);
  SelectObject(hdc_, brush);
  Transform(pos.x, pos.y);
  TextOutA(hdc_, pos.x, pos.y, text.data(), 4);
  DeleteObject(brush);
}

void Window::DrawField(mp::Vector2 leftRight, mp::Vector2 topBottom,
                       COLORREF c) {
  const HPEN pen = CreatePen(PS_SOLID, 4, c);
  auto oldPen = SelectObject(hdc_, pen);
  SelectObject(hdc_, GetStockObject(NULL_BRUSH));
  Transform(leftRight.x, topBottom.x);
  Transform(leftRight.y, topBottom.y);

  RoundRect(hdc_, leftRight.x, topBottom.x, leftRight.y, topBottom.y, 40, 40);

  SelectObject(hdc_, oldPen);
  DeleteObject(pen);
}

void Window::DrawLine(mp::Vector2 start, mp::Vector2 end, COLORREF c) {
  HPEN pen = CreatePen(PS_SOLID, 2, c);
  HPEN oldPen = static_cast<HPEN>(SelectObject(hdc_, pen));
  Transform(start.x, start.y);
  Transform(end.x, end.y);

  MoveToEx(hdc_, start.x, start.y, nullptr);

  LineTo(hdc_, end.x, end.y);
  SelectObject(hdc_, oldPen);
  DeleteObject(pen);
}

bool Window::PollEvent(mp::Event& e) {
  if (eventQueue_.empty()) return false;
  e = eventQueue_.back();
  eventQueue_.pop();
  return true;
}

Window::~Window() { currentAliveWindow = nullptr; }

inline void Window::Transform(float& x, float& y) const {
  x = (x + 1) * (static_cast<float>(width_) / 2);
  y = (y + 1) * (static_cast<float>(height_) / 2);
}

void Window::AddEventToQueue(UINT type, WPARAM key) {
  static const std::unordered_map<UINT, EventType> winTypesToGame{
      {WM_KEYDOWN, mp::EventType::Pressed},
      {WM_KEYUP, mp::EventType::Released},
  };
  static const std::unordered_map<WPARAM, KeyboardKey> winKeysToGame{
      {VK_SPACE, mp::KeyboardKey::SPACE}, {VK_ACCEPT, mp::KeyboardKey::ENTER},
      {'S', mp::KeyboardKey::S},          {'W', mp::KeyboardKey::W},
      {'A', mp::KeyboardKey::A},          {'D', mp::KeyboardKey::D},
  };

  const auto typesIt = winTypesToGame.find(type);
  const auto keysIt = winKeysToGame.find(key);

  if (typesIt != winTypesToGame.end() && keysIt != winKeysToGame.end()) {
    eventQueue_.emplace(typesIt->second, keysIt->second);
  }
}

void Window::InverseTransform(float& x, float& y) const {
  x = x / (static_cast<float>(width_) / 2) - 1;
  y = y / (static_cast<float>(height_) / 2) - 1;
}

LRESULT Window::SWndProc(HWND hwnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  switch (message) {
    case WM_KEYDOWN:
    case WM_KEYUP:
      currentAliveWindow->AddEventToQueue(message, wParam);
      break;
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    } break;
    default:
      break;
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}
}  // namespace mp