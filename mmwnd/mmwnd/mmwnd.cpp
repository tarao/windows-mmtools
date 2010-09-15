#include "stdafx.h"

#include "../common/logger.hpp"
using gnn::log;

#include "../mmwndhook/mmwndhook.h"
#include "mmwnd.h"

int main(void) {
  mmwnd& mm = mmwnd::get();
  if (!mm.load()) return 1;
  if (!mm.install()) return 2;

  ::MessageBox(NULL, _T("hook started"), _T("mmwnd"), MB_OK); // FIXME

  return 0;
}

int APIENTRY _tWinMain(HINSTANCE instance, HINSTANCE prev,
                       LPTSTR cmd, int show) {
  log().file(instance);
  int status = main();
  log() << _T("exit ") << status << std::endl;
  return status;
}
