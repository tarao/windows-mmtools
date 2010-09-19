#include "stdafx.h"

#include "../common/logger.hpp"
using gnn::log;

#include "mmwndhook.h"
#include "../mmwnd/profile.h"

gnn::ini_profile& raw_profile(void) {
  static gnn::ini_profile instance;
  return instance;
}

gnn::cached_profile& profile(void) {
  static gnn::cached_profile instance(raw_profile());
  return instance;
}

#include "mmwndhook_impl.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain(HMODULE module, DWORD  reason, LPVOID reserved) {
  if (reason == DLL_PROCESS_ATTACH) {
    log().file(module);
    raw_profile().set_path(module, _T("mmwnd"));
    mmwndhook_impl::get()->set_module_handle(module);
  }
  return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

MMWNDHOOK_API const char* version(void) {
  return MMWNDHOOK_VERSION_STRING;
}

MMWNDHOOK_API mmwndhook* the_mmwndhook(void) {
  return mmwndhook_impl::get();
}
