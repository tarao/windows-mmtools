#ifndef MMWNDHOOK_IMPL_H_INCLUDED
#define MMWNDHOOK_IMPL_H_INCLUDED

bool adjust_pos(HWND hwnd);

class mmwndhook_impl : public mmwndhook {
public:
  static LRESULT CALLBACK hook_proc(int code, WPARAM param1, LPARAM param2) {
    CWPRETSTRUCT* p = reinterpret_cast<CWPRETSTRUCT*>(param2);
    switch (p->message) {
    case WM_WINDOWPOSCHANGED:
    case WM_CREATE:
      adjust_pos(p->hwnd);
      break;
    }
    return ::CallNextHookEx(NULL, code, param1, param2);
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
  virtual void set_module_handle(HINSTANCE module) {
    module_ = module;
  }
private:
  HINSTANCE module_;
  HHOOK hook_;
};

#ifdef _DEBUG

gnn::tstring get_window_title(HWND hwnd) {
  const size_t max_text = 1024;
  gnn::tchar title[max_text];
  title[0] = 0;
  ::GetWindowText(hwnd, title, max_text);
  return gnn::tstring(title);
}

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

bool adjust_pos(HWND hwnd) {
  WINDOWINFO wi;
  wi.cbSize = sizeof(wi);
  ::GetWindowInfo(hwnd, &wi);
  if (!(wi.dwStyle & WS_VISIBLE)) return false; // ignore invisible window

  RECT rect;
  ::GetWindowRect(hwnd, &rect);
  WINDOWPLACEMENT wndpl;
  wndpl.length = sizeof(wndpl);
  ::GetWindowPlacement(hwnd, &wndpl);

  if (rect.right-rect.left <= 0 || rect.bottom-rect.top <= 0 ||
      wndpl.showCmd == 0 || (wndpl.showCmd & SW_MINIMIZE) ||
      wndpl.showCmd == SW_MAXIMIZE) return false; // not normal size window

  POINT pt1 = { rect.left, rect.top }, pt2 = { rect.right, rect.bottom };
  HMONITOR mon1, mon2;
  if ((mon1 = ::MonitorFromPoint(pt1, MONITOR_DEFAULTTONULL)) &&
      (mon2 = ::MonitorFromPoint(pt2, MONITOR_DEFAULTTONULL))) {
    if (wi.dwExStyle & WS_EX_TOPMOST) {
      return false; // top most window will show over the taskbar
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
  ::MoveWindow(hwnd, pt.x, pt.y, width, height, TRUE);

#ifdef _DEBUG
  std::basic_stringstream<gnn::tchar> ss;
  ss << std::endl << _T("    ") << _T("[pos] ") << to_s(rect);
  ss << std::endl << _T("    ") << _T("[mon] ") << to_s(mi.rcWork);
  RECT dst = { pt.x, pt.y, pt.x+width, pt.y+height };
  ss << std::endl << _T("    ") << _T("[dst] ") << to_s(dst);
  ::GetWindowRect(hwnd, &rect);
  ss << std::endl << _T("    ") << _T("[aft] ") << to_s(rect);
  log() << _T("'") << get_window_title(hwnd) << _T("'")
        << ss.str() << std::endl;
#endif // _DEBUG

  return true;
}

#endif // !MMWNDHOOK_IMPL_H_INCLUDED
