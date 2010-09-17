#ifndef GNN_PROFILE_HPP_INCLUDED
#define GNN_PROFILE_HPP_INCLUDED

#include <sstream>
#include "tstring.hpp"

namespace gnn {

#ifdef _WINDOWS

class ini_profile {
public:
  static tstring default_path(void) {
    tchar path[_MAX_PATH];
    tchar drive[_MAX_DRIVE];
    tchar dir[_MAX_DIR];
    tchar fname[_MAX_FNAME];
    ::GetModuleFileName(::GetModuleHandle(NULL), path, _MAX_PATH);
    _tsplitpath(path, drive, dir, fname, NULL);
    _tmakepath(path, drive, dir, fname, _T("ini"));
    return path;
  }

public:
  ini_profile(void):path_(default_path()){}
  explicit ini_profile(const tstring& path):path_(path){}
  ini_profile(const ini_profile& other):path_(other.path_){}

  bool delete_section(const tstring& section) const {
    return delete_entry(section, tstring());
  }
  bool delete_entry(const tstring& section, const tstring& entry) const {
    return write_string(section, entry, tstring());
  }
  bool write_string(const tstring& section, const tstring& entry,
                    const tstring& value) const {
    return ::WritePrivateProfileString(section.c_str(), entry.c_str(),
                                       value.c_str(), path_.c_str()) != 0;
  }
  tstring read_string(const tstring& section, const tstring& entry,
                      const tstring& default_=tstring()) const {
    enum { buf_len = 4096 };
    tchar buf[buf_len];
    ::GetPrivateProfileString(section.c_str(), entry.c_str(),
                              default_.c_str(), buf, buf_len, path_.c_str());
    return tstring(buf);
  }

public:
  struct entry {
    entry(const tstring& sec_, const tstring& ent_):sec(sec_),ent(ent_){}
    tstring sec, ent;
  };

protected:
  tstring path_;
};

struct ini_profile_entry {
  ini_profile_entry(ini_profile& p_, const ini_profile::entry& e_)
    :p(p_),e(e_){}
  ini_profile& p;
  const ini_profile::entry& e;
};
ini_profile_entry operator<<(ini_profile& p, const ini_profile::entry& e) {
  return ini_profile_entry(p, e);
}
ini_profile_entry operator>>(ini_profile& p, const ini_profile::entry& e) {
  return ini_profile_entry(p, e);
}
template<typename T>
ini_profile_entry& operator<<(ini_profile_entry& p, const T& val) {
  std::basic_stringstream<tchar> ss;
  ss << val;
  p.p.write_string(p.e.sec, p.e.ent, ss.str());
  return p;
}
template<typename T>
ini_profile_entry& operator>>(ini_profile_entry& p, T& val) {
  std::basic_stringstream<tchar> ss1, ss2;
  ss1 << val;
  ss2.str(p.p.read_string(p.e.sec, p.e.ent, ss1.str()));
  ss2 >> val;
  return p;
}

#endif // _WINDOWS

} // namespace gnn

#endif // !GNN_PROFILE_HPP_INCLUDED
