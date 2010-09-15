require 'win32/user32'

module Win32
  class Monitor
    def self.virtual_screen_rect
      return [ User32::SM_XVIRTUALSCREEN,
               User32::SM_YVIRTUALSCREEN,
               User32::SM_CXVIRTUALSCREEN,
               User32::SM_CYVIRTUALSCREEN,
             ].map{|i| Win32::User32.getSystemMetrics(i)}
    end

    def self.enum
      a = []
      if User32.enumDisplayMonitors(nil, nil, User32::MONITOR_ENUM, a.__id__)
        return a.map{|handle| self.new(handle)}
      else
        puts('EnumDisplayMonitors failed')
      end
    end

    def initialize(handle); @handle = handle end

    def info
      unless @info
        mi = User32::MonitorInfo.malloc
        mi.cbSize = mi.size
        if User32.getMonitorInfoA(@handle, mi)
          @info = {
            :rc_mon  => mi.rcMon,
            :rc_work => mi.rcWork,
            :flags   => mi.dwFlags,
            :device  => mi.szDevice,
          }
        end
      end
      return @info
    end

    def screen_rect; return info && info[:rc_mon] end
    def work_rect; return info && info[:rc_work] end
    def primary?
      return info && (info[:flags]&User32::MONITORINFO_PRIMARY) > 0
    end
    def name
      if info
        name = info[:device]
        return name[0 ... (name.index(0)||-1)].map{|x|x.chr}.join
      end
    end
  end
end
