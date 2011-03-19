#ifndef MMWNDHOOK_IMPL_H_INCLUDED
#define MMWNDHOOK_IMPL_H_INCLUDED

gnn::tstring get_window_title(HWND);
bool adjust_pos(HWND, UINT);

class mmwndhook_impl : public mmwndhook {
public:
  static LRESULT CALLBACK hook_proc(int code, WPARAM param1, LPARAM param2) {
    CWPRETSTRUCT* p = reinterpret_cast<CWPRETSTRUCT*>(param2);
    switch (p->message) {
    case WM_WINDOWPOSCHANGED: {
      bool restrict_movement = true;
      profile() >> PROFILE_ENTRY_RESTRICTMOVEMENT >> restrict_movement;
      WINDOWPOS* wp = reinterpret_cast<WINDOWPOS*>(p->lParam);
      if (restrict_movement || (wp->flags & SWP_FRAMECHANGED)) {
        adjust_pos(p->hwnd, p->message);
      }
      break;
    }
    case WM_SHOWWINDOW:
      if (p->wParam) adjust_pos(p->hwnd, p->message);
      break;
    }
    return ::CallNextHookEx(NULL, code, param1, param2);
  }
  static mmwndhook_impl* get(void) {
    static mmwndhook_impl instance;
    return &instance;
  }
public:
  mmwndhook_impl(void):hook_(NULL),module_(NULL){}
  virtual bool install(void) {
    if (!module_) return false;
    hook_ = ::SetWindowsHookEx(WH_CALLWNDPROCRET, hook_proc, module_, 0);
    return hook_ != NULL;
  }
  virtual bool uninstall(void) {
    if (hook_ && ::UnhookWindowsHookEx(hook_)) hook_ = NULL;
    return hook_ == NULL;
  }
  void set_module_handle(HINSTANCE module) {
    module_ = module;
  }
private:
  HINSTANCE module_;
  HHOOK hook_;
};

#ifdef _DEBUG

gnn::tstring to_s(const RECT& rect) {
    std::basic_stringstream<gnn::tchar> ss;
    ss << _T("(")
       << rect.left << _T(", ") << rect.top
       << _T(") - (")
       << rect.right << _T(", ") << rect.bottom
       << _T(")");
    return ss.str();
}

#endif // _DEBUG

gnn::tstring get_window_title(HWND hwnd) {
  const size_t max_text = 1024;
  gnn::tchar title[max_text];
  title[0] = 0;
  ::GetWindowText(hwnd, title, max_text);
  return gnn::tstring(title);
}


bool adjust_pos(HWND hwnd, UINT msg) {
  WINDOWINFO wi;
  wi.cbSize = sizeof(wi);
  ::GetWindowInfo(hwnd, &wi);
  if (!(wi.dwStyle & WS_BORDER)) return false; // ignore no border

  RECT rect;
  ::GetWindowRect(hwnd, &rect);
  WINDOWPLACEMENT wndpl;
  wndpl.length = sizeof(wndpl);
  ::GetWindowPlacement(hwnd, &wndpl);

  if (::IsRectEmpty(&rect) ||
      wndpl.showCmd == 0 || (wndpl.showCmd & SW_MINIMIZE) ||
      wndpl.showCmd == SW_MAXIMIZE) return false; // not normal size window

  POINT pt1 = { rect.left, rect.top }, pt2 = { rect.right, rect.bottom };

  RECT vscr;
  vscr.left = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
  vscr.top = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
  vscr.right = vscr.left + ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
  vscr.bottom = vscr.top + ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
  if (!::PtInRect(&vscr, pt1) && !::PtInRect(&vscr, pt2)) {
    return false; // completely out of screen
  }

  HMONITOR mon1, mon2;
  if ((mon1 = ::MonitorFromPoint(pt1, MONITOR_DEFAULTTONULL)) &&
      (mon2 = ::MonitorFromPoint(pt2, MONITOR_DEFAULTTONULL))) {
    if (wi.dwExStyle & WS_EX_TOPMOST) {
      return false; // top most window shows over the taskbar
    }
    MONITORINFO mi1, mi2;
    mi1.cbSize = mi2.cbSize = sizeof(MONITORINFO);
    ::GetMonitorInfo(mon1, &mi1); ::GetMonitorInfo(mon2, &mi2);
    if (::PtInRect(&mi1.rcWork, pt1) && ::PtInRect(&mi2.rcWork, pt2)) {
      return false; // already on a monitor working area
    }
  }

  HMONITOR monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
  MONITORINFO mi;
  mi.cbSize = sizeof(mi);
  ::GetMonitorInfo(monitor, &mi);

  POINT pt = { rect.left, rect.top };
  int width = rect.right - rect.left, height = rect.bottom - rect.top;
  if (mi.rcWork.right < rect.right) pt.x = mi.rcWork.right - width;
  if (mi.rcWork.bottom < rect.bottom) pt.y = mi.rcWork.bottom - height;
  if (pt.x < mi.rcWork.left) pt.x = mi.rcWork.left;
  if (pt.y < mi.rcWork.top) pt.y = mi.rcWork.top;

  if (pt.x == rect.left && pt.y == rect.top) return false; // no change

  bool ignore_too_large = false;
  profile() >> PROFILE_ENTRY_IGNORETOOLARGE >> ignore_too_large;
  if (ignore_too_large) {
    POINT mtl = { mi.rcWork.left, mi.rcWork.top };
    POINT mbr = { mi.rcWork.right, mi.rcWork.bottom };

    // ignore too large window
    if (::PtInRect(&rect, mtl) && ::PtInRect(&rect, mbr)) return false;

    // ignore if the window cannot be fit in the monitor
    if (mi.rcWork.right < pt.x + width ||
        mi.rcWork.bottom < pt.y + height) return false;

    log() << _T("ignore_too_large did not ignore") << std::endl;
  }

  // ignore specific window class
  TCHAR class_name[_MAX_PATH];
  ::GetClassName(hwnd, class_name, _MAX_PATH);
  if (ignore_class()[class_name]) return false;

  // ignore specific window title
  gnn::tstring title = get_window_title(hwnd);
  if (ignore_title()[title]) return false;

  // ignore desktop window
  HWND desktop = ::GetDesktopWindow();
  if (hwnd == desktop) return false;

  // ignore descendants of taskbar
  HWND root = NULL;
  HWND tmp = hwnd;
  do { root = tmp; } while ((tmp = ::GetParent(tmp)) && tmp != desktop);
  TCHAR root_class[16];
  if (::GetClassName(root, root_class, 16) &&
      gnn::tstring(_T("Shell_TrayWnd")) == root_class) return false;

  HWND parent = ::GetParent(hwnd);
  if ((wi.dwStyle & WS_CHILD) && parent) ::ScreenToClient(parent, &pt);
  UINT flags = SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOOWNERZORDER;
  ::SetWindowPos(hwnd, NULL, pt.x, pt.y, width, height, flags);

#ifdef _DEBUG
  std::basic_stringstream<gnn::tchar> ss;
  ss << std::endl << _T("  ") << _T("[class] ") << class_name;
  ss << std::endl << _T("   ") << _T("[HWND] 0x") << std::setbase(16) << hwnd;
  ss << std::endl << _T("    ") << _T("[msg] 0x") << std::setbase(16) << msg;
  ss << std::endl << _T("    ") << _T("[pos] ") << to_s(rect);
  RECT dst = { pt.x, pt.y, pt.x+width, pt.y+height };
  ss << std::endl << _T("    ") << _T("[dst] ") << to_s(dst);
  ::GetWindowRect(hwnd, &rect);
  ss << std::endl << _T("    ") << _T("[aft] ") << to_s(rect);
  log() << _T("'") << title << _T("'") << std::endl;
  log() << _T("info") << ss.str() << std::endl;
#endif // _DEBUG

  return true;
}

#endif // !MMWNDHOOK_IMPL_H_INCLUDED
