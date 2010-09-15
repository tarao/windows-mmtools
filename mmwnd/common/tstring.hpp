#ifndef GNN_TSTRING_HPP_INCLUDED
#define GNN_TSTRING_HPP_INCLUDED

#include <string>           // string, wstring
#include <iostream>         // cout, cin, wcout, wcin, basic_istream, basic_ostream
#include <fstream>          // basic_istream, basic_ofstream

#ifdef _UNICODE
#ifndef __TCHAR_DEFINED
#define __T(x)      L ## x
#endif
#else
#ifndef __TCHAR_DEFINED
#define __T(x)      x
#endif
#endif // _UNICODE
#ifndef __TCHAR_DEFINED
#define _T(x)       __T(x)
#endif

namespace gnn {
#ifdef _UNICODE
    typedef std::wstring tstring;
#define _GNN_TCIN  std::wcin
#define _GNN_TCOUT std::wcout
#define _GNN_TCERR std::wcerr
#define _GNN_TCLOG std::wclog
#else
    typedef std::string tstring;
#define _GNN_TCIN  std::cin
#define _GNN_TCOUT std::cout
#define _GNN_TCERR std::cerr
#define _GNN_TCLOG std::clog
#endif // _UNICODE
}

namespace gnn {
    typedef tstring::value_type tchar;
    typedef tchar*              tstr;
    typedef const tchar*        ctstr;
    typedef std::basic_istream<tchar, std::char_traits<tchar> > tistream;
    typedef std::basic_ostream<tchar, std::char_traits<tchar> > tostream;
    typedef std::basic_ifstream<tchar, std::char_traits<tchar> > tifstream;
    typedef std::basic_ofstream<tchar, std::char_traits<tchar> > tofstream;
    static tistream& tcin  = _GNN_TCIN;
    static tostream& tcout = _GNN_TCOUT;
    static tostream& tcerr = _GNN_TCERR;
    static tostream& tclog = _GNN_TCLOG;
}

#undef _GNN_TCIN
#undef _GNN_TCOUT
#undef _GNN_TCERR
#undef _GNN_TCLOG

#endif // !GNN_TSTRING_HPP_INCLUDED
