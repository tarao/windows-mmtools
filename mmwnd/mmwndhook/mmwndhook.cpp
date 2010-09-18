#include "stdafx.h"

#include "../common/logger.hpp"
using gnn::log;

#include "mmwndhook.h"
#include "mmwndhook_impl.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain(HMODULE module, DWORD  reason, LPVOID reserved) {
  if (reason == DLL_PROCESS_ATTACH) {
    log().file(module);
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
