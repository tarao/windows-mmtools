#include "stdafx.h"
#include <shellapi.h>

#include "resource.h"
#define WM_USER_POPUP WM_USER+100

#include "../common/logger.hpp"
using gnn::log;

#include "../mmwndhook/mmwndhook.h"
#include "mmwnd.h"
#include "profile.h"
#include "notifyicon.h"

gnn::ini_profile& profile(void) {
  static gnn::ini_profile instance;
  return instance;
}

notify_icon& the_notify_icon(void) {
  static notify_icon instance;
  return instance;
}

HWND init(HINSTANCE);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int main(HINSTANCE instance) {
  mmwnd& mm = mmwnd::get();
  if (!mm.load()) return 1;
  if (!mm.install()) return 2;

  HWND hwnd;
  if (!(hwnd=init(instance))) return 3;

  bool use_notify_icon = true;
  profile() >> PROFILE_ENTRY_NOTIFYICON >> use_notify_icon;

  notify_icon& ni = the_notify_icon();
  HICON icon = ::LoadIcon(instance, MAKEINTRESOURCE(IDI_ICON1));
  TCHAR module_path[_MAX_PATH];
  TCHAR module_name[_MAX_FNAME];
  ::GetModuleFileName(instance, module_path, _MAX_PATH);
  _tsplitpath(module_path, NULL, NULL, module_name, NULL);
  ni.init(hwnd, 0, WM_USER_POPUP, icon, module_name);
  if (use_notify_icon && !ni.install()) return 4;

  MSG msg;
  while (::GetMessage(&msg, NULL, 0, 0)) ::DispatchMessage(&msg);

  if (!ni.uninstall()) return 5;

  return 0;
}

int APIENTRY _tWinMain(HINSTANCE instance, HINSTANCE prev,
                       LPTSTR cmd, int show) {
  UNREFERENCED_PARAMETER(prev);
  UNREFERENCED_PARAMETER(cmd);
  UNREFERENCED_PARAMETER(show);

  log().file(instance);
  int status = main(instance);
  log() << _T("exit ") << status << std::endl;
  return status;
}

HWND init(HINSTANCE instance) {
  LPCTSTR class_name = _T("mmwnd.main");

  WNDCLASSEX wcex;
  wcex.cbSize        = sizeof(wcex);
  wcex.style         = CS_DBLCLKS;
  wcex.lpfnWndProc   = WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = instance;
  wcex.hIcon         = NULL;
  wcex.hCursor       = NULL;
  wcex.hbrBackground = NULL;
  wcex.lpszMenuName  = NULL;
  wcex.lpszClassName = class_name;
  wcex.hIconSm       = NULL;
  if (!::RegisterClassEx(&wcex)) return NULL;

  DWORD style = WS_CAPTION | WS_DISABLED | WS_CLIPSIBLINGS;
  HWND hwnd = ::CreateWindow(class_name, class_name, style, 0, 0, 0, 0,
                             NULL, NULL, instance, NULL);
  if (!hwnd) return NULL;
  ::ShowWindow(hwnd, SW_HIDE);

  return hwnd;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM param1, LPARAM param2) {
  switch (msg) {
  case WM_COMMAND:
    switch (LOWORD(param1)) {
    case ID_POPUP_RESTRICTMOVEMENT: {
      bool restrict_movement = true;
      profile() >> PROFILE_ENTRY_RESTRICTMOVEMENT >> restrict_movement;
      restrict_movement = !restrict_movement;
      profile() << PROFILE_ENTRY_RESTRICTMOVEMENT << restrict_movement;
      mmwnd::get().reinstall();
      log() << _T("toggle restrict movement: ")
            << restrict_movement << std::endl;
      break;
    }
    case ID_POPUP_SHOWTRAYICON:
      log("hide tray icon");
      if (the_notify_icon().uninstall()) {
        profile() << PROFILE_ENTRY_NOTIFYICON << false;
      }
      break;
    case ID_POPUP_EXIT:
      log("exit from menu");
      ::PostQuitMessage(0);
      break;
    }
    break;
  case WM_USER_POPUP:
    if (param2 == WM_RBUTTONUP) {
      HINSTANCE instance = ::GetModuleHandle(NULL);
      HMENU menu = ::LoadMenu(instance, MAKEINTRESOURCE(IDR_MENU1));
      HMENU popup;
      if (menu && (popup = ::GetSubMenu(menu, 0))) {
        bool restrict_movement = true;
        profile() >> PROFILE_ENTRY_RESTRICTMOVEMENT >> restrict_movement;
        ::CheckMenuItem(popup, ID_POPUP_RESTRICTMOVEMENT,
                        restrict_movement ? MF_CHECKED : MF_UNCHECKED);

        ::SetForegroundWindow(hwnd);
        POINT pt;
        ::GetCursorPos(&pt);
        ::TrackPopupMenu(popup, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                         pt.x, pt.y, 0, hwnd, NULL);
        ::PostMessage(hwnd, WM_NULL, 0, 0);
      }
    }
    break;
  default:
    return ::DefWindowProc(hwnd, msg, param1, param2);
  }
  return 0;
}
