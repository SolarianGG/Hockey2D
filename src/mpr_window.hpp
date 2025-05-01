#pragma once

#include "game_data.hpp"
#define NOMINMAX
#include <Windows.h>

#include <string_view>
#include <queue>

namespace Gdiplus {
class Image;
};

namespace mp {

enum class EventType : std::uint8_t {
  Pressed,
  Released,
};

enum class KeyboardKey : std::uint8_t {
  W,
  A,
  S,
  D,
  SPACE,
  ENTER,
};

struct Event {
  EventType type;
  KeyboardKey key;
};

class Window final {
public:
  Window(const Window& other) = delete;
  Window(Window&& other) noexcept = delete;
  Window& operator=(const Window& other) = delete;
  Window& operator=(Window&& other) noexcept = delete;

  Window(HINSTANCE instance, int nShowCmd, UINT windowWidth = 1024,
         UINT windowHeight = 768,
         std::string_view windowTitle = "HockeyMidterm");

  void Clear(COLORREF color = RGB(255, 255, 255));
  void Display();
  void SetWindowTitle(std::string_view title);

  [[nodiscard]]
  HDC GetDC() const {
    return hdc_;
  }
  [[nodiscard]]
  HWND GetWindow() const {
    return window_;
  }

  void DrawImage(Gdiplus::Image* image, mp::Vector2, float degree = 0.0);

  void DrawCircle(float radius, mp::Vector2 pos,
                  COLORREF fillColor = RGB(255, 255, 255), COLORREF outlineColor = RGB(0, 0, 0));

  void DrawLabel(std::string_view text, mp::Vector2 pos, COLORREF c = RGB(255, 255, 255));

  void DrawField(mp::Vector2 leftRight, mp::Vector2 topBottom,
                 COLORREF c = RGB(255, 255, 255));

  void DrawLine(mp::Vector2 start, mp::Vector2 end,
                COLORREF c = RGB(255, 255, 255));

  bool PollEvent(mp::Event& e);

  ~Window();
 private:
  void Transform(float& x, float& y) const;

  void AddEventToQueue(UINT type, WPARAM key);

  void InverseTransform(float& x, float& y) const;
  static LRESULT SWndProc(HWND hwnd, UINT message, WPARAM wParam,
                          LPARAM lParam);
  HINSTANCE hInstance_{nullptr};
  HWND window_{nullptr};
  HDC hdc_{nullptr};
  HBITMAP bitmap_{nullptr};
  int width_{0};
  int height_{0};
  std::queue<Event> eventQueue_;
};
}  // namespace mp