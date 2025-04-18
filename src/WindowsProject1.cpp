// WindowsProject1.cpp : Defines the entry point for the application.
//

#include <Windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <MMSystem.h>

#include <cmath>
#include <ctime>
#include <functional>
#include <numeric>
#include <vector>
#include <numbers>

#include "KComplex.h"
#include "KMatrix2.h"
#include "KMatrix3.h"
#include "KSpriteAnimator.h"
#include "KTileManager.h"
#include "KVector2.h"
#include "mpr_timer.h"

using namespace Gdiplus;
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdiplus.lib")

namespace {

constexpr auto kMaxLoadString = 100;

// Global Bullet struct and container
struct Bullet {
  KVector2 position;
  KVector2 velocity;
};

// Global Variables:
HINSTANCE hInst;                      // current instance

POINT g_center;
int g_pixelPerUnit = 10;
HWND g_hwnd = nullptr;
HDC g_hdc = nullptr;
HBITMAP g_hBitmap = nullptr;
RECT g_clientRect;
Image* g_image = nullptr;
Image* g_bulletImage = nullptr;
int g_mouseLButtonDown = 0;
KVector2 g_worldPoint = KVector2::Zero();
double g_angle = 0.0;
KTileManager* g_tileManager = nullptr;
std::vector<Bullet> g_bullets;
KSpriteAnimator g_animator;
KVector2 g_playerPos = KVector2::Zero();
bool g_isPaused = false;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void OnPaint(HDC hdc);
void DrawFunction(HDC hdc, const std::function<double(double)>& myFunction,
                  double begin, double end, COLORREF c, double step);
void OnSize(HWND hwnd);
void Transform(double* x, double* y);
void InverseTransform(double* x, double* y);
double GetStdDeviation(double base_, double beginX, double endX, double xstep);
void DrawLine(HDC hdc, double x1, double y1, double x2, double y2,
              COLORREF c = RGB(0, 0, 0), int penStyle = PS_SOLID,
              int lineWidth = 1);
void DrawVector(HDC hdc, KVector2 p1, KVector2 p2, COLORREF c,
                int penStyle = PS_SOLID, int lineWidth = 1);
void DrawVector(HDC hdc, KComplex c1, KComplex c2, COLORREF c,
                int penStyle = PS_SOLID, int lineWidth = 1);
void DrawImage(HDC hdc, Image* image, KVector2 p);
void Initialize();
void Finalize();
void OnIdle();
void LButtonDown(int x, int y);
void OnLButtonDown(int x, int y);
void OnLButtonUp();
}  // namespace

int APIENTRY wWinMain(_In_ const HINSTANCE hInstance,
                      _In_opt_ const HINSTANCE hPrevInstance,
                      _In_ const LPWSTR lpCmdLine, _In_ const int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // TODO: Place code here.

  // Initialize global strings
  MyRegisterClass(hInstance);

  const Gdiplus::GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }

  Initialize();
  MSG msg{};
  // Main message loop:
  TIMER.Reset();
  while (msg.message != WM_QUIT) {
    TIMER.Tick();
    if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    if (g_isPaused) {
      Sleep(30);
      continue;
    }
    OnIdle();
    Sleep(10);
  }  // while

  Finalize();
  Gdiplus::GdiplusShutdown(gdiplusToken);

  return static_cast<int>(msg.wParam);
}

namespace {

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(const HINSTANCE hInstance) {
  const WNDCLASS wc{
      .style = CS_HREDRAW | CS_VREDRAW,
      .lpfnWndProc = WndProc,
      .cbClsExtra = 0,
      .cbWndExtra = 0,
      .hInstance = hInstance,
      .hIcon = LoadIcon(0, IDI_APPLICATION),
      .hCursor = LoadCursor(0, IDC_ARROW),
      .hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)),
      .lpszMenuName = nullptr,
      .lpszClassName = "BasicWNDClass",
  };

  return RegisterClass(&wc);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(const HINSTANCE hInstance, const int nCmdShow) {
  hInst = hInstance;  // Store instance handle in our global variable

  const HWND hWnd = CreateWindowA(
      "BasicWNDClass", "HockeyGame", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
                    0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

  if (!hWnd) {
    return FALSE;
  }

  g_hwnd = hWnd;
  OnSize(hWnd);
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(const HWND hWnd, const UINT message,
                         const WPARAM wParam, const LPARAM lParam) {
  switch (message) {
    case WM_COMMAND: {
    } break;
    case WM_ACTIVATE: {
      if (LOWORD(wParam) == WA_INACTIVE) {
        g_isPaused = true;
        TIMER.Stop();
      } else {
        g_isPaused = false;
        TIMER.Start();
      }
    } break;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);
      // TODO: Add any drawing code that uses hdc here...
      // OnPaint(hdc);
      EndPaint(hWnd, &ps);
    } break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_SIZE:
      OnSize(hWnd);
      break;
    case WM_LBUTTONDOWN:
      OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      break;
    case WM_LBUTTONUP:
      OnLButtonUp();
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

[[maybe_unused]] double Square(const double x) { return x * x; }

[[maybe_unused]] double SqureRoot(const double x) {
  // return std::sqrt(x);
  return std::pow(x, 1 / 2.0);
}

[[maybe_unused]] double Exp(const double x) { return std::exp(x); }

double ExpFunction(const double base_, const double x) {
  return std::pow(base_, x);
}

[[maybe_unused]] double Logistic(const double x) {
  // Logistic Function
  const double L = 1.0;
  const double k = 3.0;
  const double x0 = 0.0;
  return L / (1 + std::exp(-k * (x - x0)));
}

[[maybe_unused]] double Gaussian(const double x) {
  // Gaussian Function
  const double a = 1.0;
  const double b = 0.0;
  const double c = 1.0;
  return a * std::exp(-((x - b) * (x - b)) / (2 * c * c));
}

// double(*CALLBACK)(double x)

[[maybe_unused]] void DrawFunction(const HDC hdc, const std::function<double(double)>& myFunction,
                                   const double begin, const double end, const COLORREF c,
                                   const double step = 0.2) {
  double prevx = begin;
  double prevy = myFunction(prevx);

  for (double x = begin; x <= end; x += step) {
    const double y = myFunction(x);
    DrawLine(hdc, prevx, prevy, x, y, c, PS_SOLID, 2);
    prevx = x;
    prevy = y;
  }  // for
}

[[maybe_unused]] void DrawFunction(const HDC hdc,
                                   const std::function<double(double, double)>& myFunction,
                                   const double base_, const double begin, const double end,
                                   const COLORREF c, const double step = 0.2) {
  double prevx = begin;
  double prevy = myFunction(base_, prevx);

  for (double x = begin; x <= end; x += step) {
    const double y = myFunction(base_, x);
    DrawLine(hdc, prevx, prevy, x, y, c, PS_SOLID, 2);
    prevx = x;
    prevy = y;
  }  // for
}

[[maybe_unused]] double NewtonsDifference(const std::function<double(double)>& f, const double x,
                                          const double dx = 0.0001) {
  const double y0 = f(x);
  const double y1 = f(x + dx);
  return (y1 - y0) / dx;
}

[[maybe_unused]] double SymmetricDifference(const std::function<double(double)>& f, const double x,
                                            const double dx = 0.0001) {
  const double y0 = f(x - dx);
  const double y1 = f(x + dx);
  return (y1 - y0) / (2.0 * dx);
}

double SymmetricDifference(const std::function<double(double, double)>& f,
                           const double base_, const double x,
                           const double dx = 0.0001) {
  const double y0 = f(base_, x - dx);
  const double y1 = f(base_, x + dx);
  return (y1 - y0) / (2.0 * dx);
}

[[maybe_unused]] double GetStdDeviation(const double base_, const double beginX,
                                        const double endX, const double xstep) {
  std::vector<double> vecDiff;

  double x = beginX;
  double ydiff;
  double N = 0;
  while (x < endX) {
    ydiff = ExpFunction(base_, x) - SymmetricDifference(&ExpFunction, base_, x);
    x += xstep;
    N += 1;
    vecDiff.push_back(ydiff);
  }  // while

  double sum = 0;
  for (const double diff : vecDiff) {
    sum += (diff * diff);
  }

  return sqrt(sum / (N - 1));
}

void DrawGrid(const HDC hdc, const int gridCount) {
  const int width = g_clientRect.right - g_clientRect.left;
  const int height = g_clientRect.bottom - g_clientRect.top;

  const double halfWidthUnits = static_cast<double>(width) / (2.0 * g_pixelPerUnit);
  const double halfHeightUnits = static_cast<double>(height) / (2.0 * g_pixelPerUnit);

  for (int i = 1; i <= gridCount; ++i) {
    const double offset = i;

    // Vertical lines 
    DrawLine(hdc, -offset, -halfHeightUnits, -offset, +halfHeightUnits,
             RGB(200, 200, 200), PS_DASH);
    DrawLine(hdc, +offset, -halfHeightUnits, +offset, +halfHeightUnits,
             RGB(200, 200, 200), PS_DASH);

    // Horizontal lines 
    DrawLine(hdc, -halfWidthUnits, -offset, +halfWidthUnits, -offset,
             RGB(200, 200, 200), PS_DASH);
    DrawLine(hdc, -halfWidthUnits, +offset, +halfWidthUnits, +offset,
             RGB(200, 200, 200), PS_DASH);
  }
}

void DrawImage(const HDC hdc, Image* image, KVector2 p, const double degree) {
  if (image != nullptr) {
    const int imageWidth = image->GetWidth();
    const int imageHeight = image->GetHeight();

    Transform(&p.x, &p.y);  // transform to client coordinate

    Graphics graphics(hdc);
    graphics.SetSmoothingMode(SmoothingModeHighQuality);
    graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

    // push transform to the stack
    graphics.TranslateTransform(static_cast<REAL>(p.x), static_cast<REAL>(p.y));
    graphics.RotateTransform(static_cast<REAL>(degree));
    graphics.TranslateTransform(static_cast<REAL>(-imageWidth) / 2,
                                static_cast<REAL>(-imageHeight) / 2);

    graphics.DrawImage(image, 0, 0, imageWidth, imageHeight);

    // reset transform
    graphics.ResetTransform();
  }
}

void OnPaint(const HDC hdc) {
  char buffer[80];
  int len = 0;

  DrawGrid(hdc, 10);
  DrawLine(hdc, -100, 0, +100, 0, RGB(255, 0, 0), PS_SOLID);
  DrawLine(hdc, 0, -100, 0, +100, RGB(0, 255, 0), PS_SOLID);

  DrawVector(hdc, KVector2(0, 0), g_worldPoint, RGB(0, 0, 255), PS_SOLID, 2);

  static double degree = 0.0;
  static double radian = 0.0;
  constexpr double radianToDegree = 180.0 / std::numbers::pi;
  radian += TIMER.DeltaTime();
  degree = -radian * radianToDegree;

  KMatrix3 translation;
  KMatrix3 rotation;
  translation.SetTranslation(5, 0);
  rotation.SetRotation(radian);
  const KVector2 p1 = rotation * translation * KVector2::Zero();
  // KVector2 p1 = translation * rotation * KVector2::zero;
  DrawImage(hdc, g_image, p1, degree);

  // Draw bullets
  for (const Bullet& bullet : g_bullets) {
    DrawImage(hdc, g_bulletImage, bullet.position);
  }

  {
    constexpr float moveSpeed = 5.0f;  // units per second
    const float dt = TIMER.DeltaTime();

    if (GetAsyncKeyState(VK_LEFT) & 0x8000) g_playerPos.x -= moveSpeed * dt;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) g_playerPos.x += moveSpeed * dt;
    if (GetAsyncKeyState(VK_UP) & 0x8000) g_playerPos.y += moveSpeed * dt;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) g_playerPos.y -= moveSpeed * dt;
  }
  KVector2 pos = g_playerPos;
  Transform(&pos.x, &pos.y);
  g_animator.Draw(hdc, 0, pos.x, pos.y, false, 2.0);
  g_animator.Draw(hdc, 0, pos.x + 32, pos.y, false, 2.0);
  g_animator.Draw(hdc, 0, pos.x + 64, pos.y, false, 2.0);
  g_animator.Draw(hdc, 0, pos.x, pos.y + 64, true, 2.0);
  g_animator.Draw(hdc, 0, pos.x + 32, pos.y + 64, true, 2.0);
  g_animator.Draw(hdc, 0, pos.x + 64, pos.y + 64, true, 2.0);

  //// Draw FPS
  // static int prevFps = 0;
  // int fps = (int)(1.0 / g_time.deltaTime);
  // if (std::abs(prevFps - fps) > 10) {
  //     prevFps = fps;
  // }
  // sprintf_s(buffer, "FPS: %d", prevFps);
  //::TextOutA(hdc, 1, 1, buffer, (int)strlen(buffer));
}

void DrawLine(const HDC hdc, double x1, double y1, double x2, double y2,
              const COLORREF c, const int penStyle, const int lineWidth) {
  const HPEN hPen = CreatePen(penStyle, lineWidth, c);
  const auto hPrevPen = static_cast<HPEN>(SelectObject(hdc, hPen));

  Transform(&x1, &y1);
  MoveToEx(hdc, static_cast<int>(x1), static_cast<int>(y1), nullptr);
  Transform(&x2, &y2);
  LineTo(hdc, static_cast<int>(x2), static_cast<int>(y2));

  SelectObject(hdc, hPrevPen);
  DeleteObject(hPen);
}

void DrawVector(const HDC hdc, const KVector2 p1, const KVector2 p2,
                const COLORREF c, const int penStyle, const int lineWidth) {
  DrawLine(hdc, p1.x, p1.y, p2.x, p2.y, c, penStyle, lineWidth);
}

void DrawVector(const HDC hdc, const KComplex c1, const KComplex c2,
                const COLORREF c, const int penStyle, const int lineWidth) {
  DrawLine(hdc, c1.r, c1.i, c2.r, c2.i, c, penStyle, lineWidth);
}

void DrawImage(const HDC hdc, Image* image, KVector2 p) {
  if (image != nullptr) {
    const int imageWidth = image->GetWidth();
    const int imageHeight = image->GetHeight();
    Graphics graphics(hdc);
    Transform(&p.x, &p.y);
    graphics.DrawImage(image, static_cast<int>(p.x) - imageWidth / 2,
                       static_cast<int>(p.y) - imageHeight / 2);
  }
}

void OnSize(HWND hwnd) {
  Finalize();

  ::GetClientRect(g_hwnd, &g_clientRect);
  const int width = g_clientRect.right - g_clientRect.left + 1;
  const int height = g_clientRect.bottom - g_clientRect.top + 1;
  g_center.x = width / 2;
  g_center.y = height / 2;
  g_pixelPerUnit = 50;

  const HDC hdc = ::GetDC(g_hwnd);
  g_hdc = CreateCompatibleDC(hdc);
  g_hBitmap = CreateCompatibleBitmap(hdc, width, height);
  SelectObject(g_hdc, g_hBitmap);

  Initialize();
}

void Transform(double* x, double* y) {
  *x = g_center.x + (*x) * g_pixelPerUnit;
  *y = g_center.y + -(*y) * g_pixelPerUnit;
}

void InverseTransform(double* x, double* y) {
  *x = ((*x) - g_center.x) / g_pixelPerUnit;
  *y = -((*y) - g_center.y) / g_pixelPerUnit;
}

void Initialize() {
  std::srand(std::time(nullptr));
  if (g_image == nullptr) g_image = new Image(L"Baren.png");

  if (g_bulletImage == nullptr) g_bulletImage = new Image(L"bullet.png");

  if (g_tileManager == nullptr) g_tileManager = new KTileManager();
  g_tileManager->LoadTileSheet(L"Atlas-working.png", 16, 32);

  g_animator.ClearAll();
  g_animator.SetTileMap(g_tileManager);

  const std::vector walkRightFrames {
      KVector2(0, 0),                                 // col=0, row=0
      KVector2(1, 0), KVector2(2, 0), KVector2(3, 0)  // repeat middle for loop
  };

  g_animator.SetAnimation(0, walkRightFrames, 0.15);  // 0.15 sec per frame
}  // Initialize()

void Finalize() {
  if (g_tileManager != nullptr) {
    delete g_tileManager;
    g_tileManager = nullptr;
  }
  if (g_hdc != nullptr) {
    DeleteDC(g_hdc);
    g_hdc = nullptr;
  }  // if
  if (g_hBitmap != nullptr) {
    DeleteObject(g_hBitmap);
    g_hBitmap = nullptr;
  }  // if
  if (g_image != nullptr) {
    delete g_image;
    g_image = nullptr;
  }  // if
  if (g_bulletImage != nullptr) {
    delete g_bulletImage;
    g_bulletImage = nullptr;
  }  // if
}  // Finalize()

void UpdateBullets() {
  const double dt = TIMER.DeltaTime();

  for (auto& bullet : g_bullets) {
    constexpr double speed = 1.0;
    bullet.position =
        bullet.position + bullet.velocity * static_cast<float>(speed * dt);
  }
}

void OnIdle() {
  const int iWidth = g_clientRect.right - g_clientRect.left + 1;
  const int iHeight = g_clientRect.bottom - g_clientRect.top + 1;

  const HDC hdc = ::GetDC(g_hwnd);

  HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
  SelectObject(g_hdc, brush);
  Rectangle(g_hdc, 0, 0, iWidth, iHeight);

  {
    UpdateBullets();
    g_animator.Update(TIMER.DeltaTime());
    OnPaint(g_hdc);
  }

  BitBlt(hdc, 0, 0, iWidth, iHeight, g_hdc, 0, 0, SRCCOPY);
  DeleteObject(brush);

  ::ReleaseDC(g_hwnd, hdc);
}  // OnIdle()

void LButtonDown(const int x, const int y) {
  // (x,y) is already point in client coordinate
  KVector2 mousePoint;
  mousePoint.x = x;
  mousePoint.y = y;
  InverseTransform(&mousePoint.x, &mousePoint.y);  // transform to world space
  g_worldPoint = mousePoint;
  g_angle = 0.0;

  KVector2 dir = g_worldPoint - KVector2::Zero();
  if (dir.Length() > 0.0001) {
    dir = dir.Normalize();
    Bullet bullet;
    bullet.position = KVector2::Zero();
    bullet.velocity = dir * 2;
    g_bullets.push_back(bullet);
  }
}

void OnLButtonDown(const int x, const int y) {
  if (g_mouseLButtonDown != 1) {
    LButtonDown(x, y);
  }
  g_mouseLButtonDown = 1;
}

void OnLButtonUp() { g_mouseLButtonDown = 0; }

double Random() { return static_cast<double>(rand()) / RAND_MAX; }

}  // namespace
