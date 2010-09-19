#ifndef MMWND_PROFILE_H_INCLUDED
#define MMWND_PROFILE_H_INCLUDED

#include "../common/profile.hpp"
#define PROFILE_ENTRY_NOTIFYICON  \
  gnn::ini_profile::entry(_T("settings"), _T("notify_icon"))
#define PROFILE_ENTRY_RESTRICTMOVEMENT  \
  gnn::ini_profile::entry(_T("settings"), _T("restrict_movement"))

#endif // !MMWND_PROFILE_H_INCLUDED
