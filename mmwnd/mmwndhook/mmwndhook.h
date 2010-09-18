#ifndef MMWNDHOOK_H_INCLUDED
#define MMWNDHOOK_H_INCLUDED

#ifdef MMWNDHOOK_EXPORTS
#define MMWNDHOOK_API extern "C" __declspec(dllexport)
#else
#define MMWNDHOOK_API __declspec(dllimport)
#endif

class mmwndhook {
public:
  virtual bool install(void)=0;
  virtual bool uninstall(void)=0;
};

const char* MMWNDHOOK_VERSION_STRING = __TIMESTAMP__;
MMWNDHOOK_API const char* __cdecl version(void);
MMWNDHOOK_API mmwndhook* __cdecl the_mmwndhook(void);

#endif // !MMWNDHOOK_H_INCLUDED
