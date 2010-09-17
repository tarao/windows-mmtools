#ifndef MMWND_NOTIFYICON_H_INCLUDED
#define MMWND_NOTIFYICON_H_INCLUDED

class notify_icon {
public:
  notify_icon(void):installed(false){}
  void init(HWND hwnd, UINT id, UINT msg, HICON icon, LPCTSTR tip) {
    nid.cbSize = sizeof(nid);
    nid.hWnd   = hwnd;
    nid.uID    = id;
    nid.uFlags = NIF_MESSAGE | /* NIF_ICON |*/ NIF_TIP;
    nid.uCallbackMessage = msg;
    nid.hIcon  = icon;
    lstrcpy(nid.szTip, tip);
  }
  bool install(void) {
    return (installed = (::Shell_NotifyIcon(NIM_ADD, &nid) != 0));
  }
  bool uninstall(void) {
    if (!installed) return true;
    return ::Shell_NotifyIcon(NIM_DELETE, &nid) != 0 && !(installed = false);
  }
private:
  NOTIFYICONDATA nid;
  bool installed;
};

#endif // !MMWND_NOTIFYICON_H_INCLUDED
