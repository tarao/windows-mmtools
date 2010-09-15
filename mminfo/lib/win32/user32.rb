require 'dl/import'
require 'dl/struct'

module Win32
  module User32
    extend DL::Importable
    dlload 'user32.dll'

    SM_XVIRTUALSCREEN = 76
    SM_YVIRTUALSCREEN = 77
    SM_CXVIRTUALSCREEN = 78
    SM_CYVIRTUALSCREEN = 79
    extern 'int GetSystemMetrics(int)'

    typealias('RECT', 'long[4]')
    typealias('HMONITOR', 'void*')
    typealias('HDC', 'void*')
    typealias('LPRECT', 'void*')
    typealias('LPARAM', 'long')
    typealias('MONITORENUMPROC', 'void*')

    def monitor_enum(hmon, hdc, lprect, obj_id)
      ObjectSpace._id2ref(obj_id).push(hmon)
      return true, [ hmon, hdc, lprect, obj_id ]
    end
    MONITOR_ENUM = callback 'BOOL monitor_enum(HMONITOR,HDC,LPRECT,LPARAM)'
    extern 'BOOL EnumDisplayMonitors(HDC,LPRECT,MONITORENUMPROC,LPARAM)'

    MONITORINFO_PRIMARY = 0x01
    MonitorInfo =
      struct [
              'DWORD cbSize',
              'long rcMon[4]',
              'long rcWork[4]',
              'DWORD dwFlags',
              'char szDevice[32]',
             ]
    extern 'BOOL GetMonitorInfoA(HMONITOR,void*)'
  end
end
