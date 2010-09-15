#ifndef MMWND_H_INCLUDED
#define MMWND_H_INCLUDED

bool check_version(HMODULE module) {
  typedef const char* (WINAPI *proc_t)(void);
  proc_t proc = (proc_t)::GetProcAddress(module, "version");
  const char* ver = proc();
  return std::string(proc()) == MMWNDHOOK_VERSION_STRING;
}

class mmwnd {
public:
  static mmwnd& get(void) {
    static mmwnd instance;
    return instance;
  }
  bool load(void) {
    module_ = ::LoadLibrary(_T("mmwndhook.dll"));
    if (!module_) return false;
    if (!check_version(module_)) return false;
    log(_T("version check passed"));

    typedef mmwndhook* (WINAPI *proc_t)(void);
    proc_t proc = (proc_t)::GetProcAddress(module_, "the_mmwndhook");
    if (!proc || !(hook_ = proc())) {
      log(_T("cannot get hook instance"));
      return false;
    }
    return true;
  }
  bool install(void) {
    if (hook_->install()) {
      log(_T("hook installed"));
      return true;
    } else {
      log(_T("hook installation failed"));
      return false;
    }
  }
  bool reinstall(void) {
    return hook_->uninstall() && hook_->install();
  }
  ~mmwnd(void) {
    uninstall();
    if (module_) ::FreeLibrary(module_);
  }
private:
  bool uninstall(void) {
    if (hook_ && hook_->uninstall()) {
      log(_T("hook uninstalled"));
      return true;
    } else {
      log(_T("hook uninstallation failed"));
      return false;
    }
  }
  mmwnd(void):module_(NULL),hook_(NULL){}
  mmwnd(mmwnd&):module_(NULL),hook_(NULL){}
  mmwnd& operator=(mmwnd&){ return *this; }
private:
  HMODULE module_;
  mmwndhook* hook_;
};

#endif // !MMWND_H_INCLUDED