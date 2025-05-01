#include "connect_dialog.hpp"

#include "resource.h"

namespace {
std::string g_ip, g_name;
uint16_t g_port = 0;
}  // namespace
namespace mp {

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  switch (uMsg) {
    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK) {
        char ipBuf[64], portBuf[16], nameBuf[64];
        GetDlgItemTextA(hwndDlg, IDC_IP_EDIT, ipBuf, sizeof(ipBuf));
        GetDlgItemTextA(hwndDlg, IDC_PORT_EDIT, portBuf, sizeof(portBuf));
        GetDlgItemTextA(hwndDlg, IDC_NAME_EDIT, nameBuf, sizeof(nameBuf));

        g_ip = ipBuf;
        g_port = static_cast<uint16_t>(atoi(portBuf));
        g_name = nameBuf;

        EndDialog(hwndDlg, IDOK);
        return TRUE;
      } else if (LOWORD(wParam) == IDCANCEL) {
        EndDialog(hwndDlg, IDCANCEL);
        return TRUE;
      }
      break;
  }
  return FALSE;
}

bool ShowConnectionDialog(HINSTANCE hInstance, HWND hWndParent,
                          ConnectionInfo& info) {
  const int res = DialogBox(hInstance, MAKEINTRESOURCE(IDD_CONNECT_DIALOG),
                            hWndParent, DialogProc);
  if (res == IDOK) {
    info.ip = g_ip;
    info.port = g_port;
    info.playerName = g_name;
  } else {
    return false;
  }

  return true;
}
}  // namespace mp
