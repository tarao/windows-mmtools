#ifndef GNN_PROFILE_HPP_INCLUDED
#define GNN_PROFILE_HPP_INCLUDED

#include <sstream>
#include <map>
#include "tstring.hpp"

namespace gnn {

class iprofile {
public:
  virtual ~iprofile(void){}
  virtual bool delete_section(const tstring& section)= 0;
  virtual bool delete_entry(const tstring& section,
                            const tstring& entry) = 0;
  virtual bool write(const tstring& section, const tstring& entry,
                     const tstring& value) = 0;
  virtual tstring read(const tstring& section, const tstring& entry,
                       const tstring& default_) = 0;
public:
  struct entry {
    entry(const tstring& sec_, const tstring& ent_):sec(sec_),ent(ent_){}
    tstring sec, ent;
  };
};

#ifdef _WINDOWS

class ini_profile : public iprofile {
public:
  static tstring default_path(HINSTANCE instance=NULL,
                              const tstring& name = tstring()) {
    tchar path[_MAX_PATH];
    tchar drive[_MAX_DRIVE];
    tchar dir[_MAX_DIR];
    tchar fname[_MAX_FNAME];
    if (!instance) instance = ::GetModuleHandle(NULL);
    ::GetModuleFileName(instance, path, _MAX_PATH);
    _tsplitpath(path, drive, dir, fname, NULL);
    _tmakepath(path, drive, dir, name.empty()?fname:name.c_str(), _T("ini"));
    return path;
  }

public:
  ini_profile(void):path_(default_path()){}
  explicit ini_profile(const tstring& path):path_(path){}
  ini_profile(const ini_profile& other):path_(other.path_){}

  void set_path(const tstring& path){ path_ = path; }
  void set_path(HINSTANCE instance, const tstring& fname) {
    path_ = default_path(instance, fname);
  }
  tstring get_path(void) const { return path_; }

  virtual bool delete_section(const tstring& section) {
    return delete_entry(section, tstring());
  }
  virtual bool delete_entry(const tstring& section,
                            const tstring& entry) {
    return write(section, entry, tstring());
  }
  virtual bool write(const tstring& section, const tstring& entry,
                     const tstring& value) {
    return ::WritePrivateProfileString(section.c_str(), entry.c_str(),
                                       value.c_str(), path_.c_str()) != 0;
  }
  virtual tstring read(const tstring& section, const tstring& entry,
                       const tstring& default_) {
    enum { buf_len = 4096 };
    tchar buf[buf_len];
    ::GetPrivateProfileString(section.c_str(), entry.c_str(),
                              default_.c_str(), buf, buf_len, path_.c_str());
    return tstring(buf);
  }

protected:
  tstring path_;
};

#endif // _WINDOWS

class cached_profile : public iprofile {
public:
  cached_profile(iprofile& p):p_(p){}
  cached_profile(cached_profile& other):p_(other.p_), cache_(other.cache_){}
  virtual bool delete_section(const tstring& section) {
    cache_.clear();
    return p_.delete_section(section);
  }
  virtual bool delete_entry(const tstring& section,
                            const tstring& entry) {
    cache_.erase(make_entry(section, entry));
    return p_.delete_entry(section, entry);
  }
  virtual bool write(const tstring& section, const tstring& entry,
                     const tstring& value) {
    cache_[make_entry(section, entry)] = value;
    return p_.write(section, entry, value);
  }
  virtual tstring read(const tstring& section, const tstring& entry,
                       const tstring& default_) {
    tstring val = p_.read(section, entry, default_);
    tstring ent = make_entry(section, entry);
    if (!cache_.count(ent))  const_cast<cache_type&>(cache_)[ent] = val;
    return val;
  }

protected:
  static tstring make_entry(const tstring& section, const tstring& entry) {
    return section + _T("::") + entry;
  }
  typedef std::map<tstring, tstring> cache_type;
  iprofile& p_;
  cache_type cache_;
};

struct profile_entry {
  profile_entry(iprofile& p_, const iprofile::entry& e_):p(p_),e(e_){}
  profile_entry(const profile_entry& other):p(other.p),e(other.e){}
  iprofile& p;
  const iprofile::entry& e;
};
profile_entry operator<<(iprofile& p, const iprofile::entry& e) {
  return profile_entry(p, e);
}
profile_entry operator>>(iprofile& p, const iprofile::entry& e) {
  return profile_entry(p, e);
}
template<typename T>
profile_entry& operator<<(profile_entry& p, const T& val) {
  std::basic_stringstream<tchar> ss;
  ss << val;
  p.p.write(p.e.sec, p.e.ent, ss.str());
  return p;
}
template<typename T>
profile_entry& operator>>(profile_entry& p, T& val) {
  std::basic_stringstream<tchar> ss1, ss2;
  ss1 << val;
  ss2.str(p.p.read(p.e.sec, p.e.ent, ss1.str()));
  ss2 >> val;
  return p;
}

} // namespace gnn

#endif // !GNN_PROFILE_HPP_INCLUDED
