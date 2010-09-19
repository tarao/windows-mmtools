#ifndef MMWNDHOOK_IGNORE_LIST_H_INCLUDED
#define MMWNDHOOK_IGNORE_LIST_H_INCLUDED

#include <set>
#include <fstream>
#include <sstream>

class ignore_list {
public:
  static std::string default_path(HINSTANCE instance,
                                  const std::string& name) {
    char path[_MAX_PATH];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    ::GetModuleFileNameA(instance, path, _MAX_PATH);
    _splitpath(path, drive, dir, NULL, NULL);
    _makepath(path, drive, dir, name.c_str(), "txt");
    return path;
  }

public:
  void set_path(const std::string& path){ path_ = path; }
  void set_path(HINSTANCE instance, const std::string& name) {
    path_ = default_path(instance, name);
  }
  bool load(void) {
    if (list_.empty() && !path_.empty()) {
      std::basic_ifstream<gnn::tchar> in(path_.c_str(), std::ios::in);
      if (in.is_open()) {
        gnn::tstring buf;
        while (!in.eof()) {
          std::getline(in, buf);
          buf = gnn::strip(buf);
          if (!buf.empty()) {
            list_.insert(buf);
          }
        }
        in.close();

#ifdef _DEBUG
        log() << _T("loaded ignore list ") << list_.size()
              << (list_.size() == 1 ? _T(" line") : _T(" lines")) << std::endl;
#endif // !_DEBUG

        return true;
      }
    }
    return false;
  }
  bool test(const gnn::tstring& name) {
    load();
    return list_.count(name) != 0;
  }
  bool operator[](const gnn::tstring& name) {
    return test(name);
  }

protected:
  std::string path_;
  std::set<gnn::tstring> list_;
};

#endif // !MMWNDHOOK_IGNORE_LIST_H_INCLUDED
