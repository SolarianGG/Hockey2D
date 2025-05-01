#pragma once
#include <windows.h>

#include <string>

namespace mp {
struct ConnectionInfo {
  std::string ip;
  uint16_t port;
  std::string playerName;
};

bool ShowConnectionDialog(HINSTANCE hInstance, HWND hWndParent,
                          ConnectionInfo& info);
}

