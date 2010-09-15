#ifndef GNN_LOGGER_HPP_INCLUDED
#define GNN_LOGGER_HPP_INCLUDED

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include "tstring.hpp"

namespace gnn {

#ifdef _WINDOWS

std::string log_file(HINSTANCE module) {
  char path[_MAX_PATH];
  char drive[_MAX_DRIVE];
  char dir[_MAX_DIR];
  char fname[_MAX_FNAME];
  ::GetModuleFileNameA(module, path, _MAX_PATH);
  _splitpath(path, drive, dir, fname, NULL);
  _makepath(path, drive, dir, fname, "log");
  return path;
}

#endif // !_WINDOWS

template<typename CHAR> class logger {
public:
  static logger<CHAR>& get(void) {
    static logger<CHAR> instance;
    return instance;
  }
  void file(const std::string& fname) {
    fname_ = fname;
  }
#ifdef _WINDOWS
  void file(HINSTANCE module) {
    file(log_file(module));
  }
#endif // _WINDOWS
public:
  template<typename T>
  logger<CHAR>& put(const T& msg) {
    ss_ << msg;
    return *this;
  }
  logger<CHAR>& flush(void) {
    using namespace std;
    basic_ofstream<CHAR> fout(fname_.c_str(), ios::out|ios::app);
    if (fout.is_open()) {
      put_time(fout);
      fout << ss_.str() << endl;
      fout.close();
    }
    ss_.str(std::basic_string<CHAR>()); // clear
    return *this;
  }
private:
  std::tm* gettime(void) {
    time_t rawtime = std::time(NULL);
    return std::localtime(&rawtime);
  }
  template<typename CHAR2>
  void put_time(std::basic_ofstream<CHAR2>&){}
  template<> void put_time<char>(std::basic_ofstream<char>& fout) {
    using namespace std;
    tm* tm = gettime();
    fout << setfill('0') << setw(4) << tm->tm_year+1900 << "-"
         << setfill('0') << setw(2) << tm->tm_mon << "-"
         << setfill('0') << setw(2) << tm->tm_mday << "T"
         << setfill('0') << setw(2) << tm->tm_hour << ":"
         << setfill('0') << setw(2) << tm->tm_min << ":"
         << setfill('0') << setw(2) << tm->tm_sec << " ";
  }
  template<> void put_time<wchar_t>(std::basic_ofstream<wchar_t>& fout) {
    using namespace std;
    tm* tm = gettime();
    fout << setfill(L'0') << setw(4) << tm->tm_year+1900 << L"-"
         << setfill(L'0') << setw(2) << tm->tm_mon << L"-"
         << setfill(L'0') << setw(2) << tm->tm_mday << L"T"
         << setfill(L'0') << setw(2) << tm->tm_hour << L":"
         << setfill(L'0') << setw(2) << tm->tm_min << L":"
         << setfill(L'0') << setw(2) << tm->tm_sec << L" ";
  }
private:
  std::string fname_;
  std::basic_stringstream<CHAR> ss_;
};

template<typename CHAR, typename T>
logger<CHAR>& operator<<(logger<CHAR>& l, const T& msg) {
  return l.put(msg);
}

template<typename CHAR>
logger<CHAR>& operator<<(
  logger<CHAR>& l,
  std::basic_ostream<CHAR>& (*func)(std::basic_ostream<CHAR>&)) {
  return func == std::endl ? l.flush() : l.put(func);
}

class null_logger {
public:
  void file(const std::string&){}
#ifdef _WINDOWS
  void file(HINSTANCE){}
#endif // _WINDOWS
};

template<typename T>
null_logger& operator<<(null_logger& l, const T&) {
  return l;
}

null_logger& operator<<(
  null_logger& l,
  std::basic_ostream<CHAR>& (*)(std::basic_ostream<CHAR>&)) {
  return l;
}

#ifdef _DEBUG

logger<tchar>& log(void) {
  return logger<tchar>::get();
}
template<typename T>
logger<tchar>& log(const T& msg) {
  return log().put(msg).flush();
}

#else

null_logger& log(void) {
  static null_logger instance;
  return instance;
}
template<typename T>
null_logger& log(const T&) {
  return log();
}

#endif // !_DEBUG

} // namespace gnn

#endif // !GNN_LOGGER_HPP_INCLUDED
