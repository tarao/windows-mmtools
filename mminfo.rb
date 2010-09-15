$:.unshift(File.join(File.dirname($0), 'lib'))
require 'win32/monitor'

class Argv
  def initialize(argv, opt)
    @a = argv
    @r = {}
    opt.each do |k, v|
      pat, default, conv = v
      @r[k] = ( conv ? (long(pat[1]) || short(pat[0])) : flag(pat) ) || default
      @r[k] = @r[k].send(conv) if @r[k] && conv
    end
  end
  def flag(pat); pat.any?{|x| @a.delete(x)} end
  def long(pat); arg=nil; @a.reject!{|x|x=~/^#{pat}=(.*)$/&&arg=$1} && arg end
  def short(pat); (i=@a.index(pat)) && @a.delete_at(i) && @a.delete_at(i) end
  def [](name); @r[name.to_sym] end
  def []=(name, v); @r[name.to_sym]=v end
end

opt = {
  :help    => [ %w'-h --help' ],
  :all     => [ %w'-a --all', true ],
  :virtual => [ %w'-v --virtual-screen' ],
  :index   => [ %w'-i --index', nil, :to_i ],
  :primary => [ %w'-p --primary' ],
  :monitor => [ %w'-m --monitor' ],
  :work    => [ %w'-w --work' ],
  :name    => [ %w'-n --name' ],
  :quiet   => [ %w'-q --quiet' ],
}
$argv = Argv.new($*, opt)

if $argv[:help]
  puts <<"EOM"
Usage: #{$0} OPTIONS
  Show information of multiple monitors.
Options:
  -h,     --help              Show this message.
  -v,     --virtual-screen    Virtual screen information.
  -i <n>, --index=<n>         Specify the monitor index.
  -p,     --primary           Select the primary monitor.
  -m,     --monitor           Print the screen rectangle.
  -w,     --work              Print the work area rectangle.
  -n,     --name              Print the name of the monitor.
  -q,     --quiet             Suppress messages for human readability.
EOM
  exit
end

select = [ :index, :primary, ]
$argv[:all] = false if select.any?{|k| $argv[k]}
fields = [ :monitor, :work, :name, ]
fields << :virtual unless select.any?{|k| $argv[k]}
fields.each{|k| $argv[k]=true} unless fields.any?{|k| $argv[k]}

class Caption
  class Verbose
    def puts(what, f); Kernel.puts(f % what) end
    def print(what, f); Kernel.print(f % what) end
  end
  class Quiet
    def puts(what, f); end
    def print(what, f); end
  end
  def initialize(quiet, f); @c = quiet ? Quiet.new : Verbose.new; @f = f end
  def puts(what); @c.puts(what, @f) end
  def print(what); @c.print(what, @f) end
end
$cap = Caption.new($argv[:quiet], '[%s]')
$fld = Caption.new($argv[:quiet], "%s:\n  ")

class Array; def to_rect_str; return '(%d, %d) - (%d, %d)' % self end end

if $argv[:virtual]
  $cap.puts('Virtual Screen')
  puts(Win32::Monitor.virtual_screen_rect.to_rect_str)
  puts unless $argv[:quiet]
end

module Win32
  class Monitor
    def puts
      $argv[:quiet] ? Kernel.puts(name) : $cap.puts(name) if $argv[:name]
      [ [ :monitor, 'screen rectangle', :screen_rect ],
        [ :work, 'work area rectangle', :work_rect ],
      ].each do |k, s, meth|
        if $argv[k]
          $fld.print(s)
          val = self.send(meth)
          val.is_a?(Array) ? Kernel.puts(val.to_rect_str) : Kernel.puts(val)
        end
      end
    end
  end
end

Win32::Monitor.enum.each_with_index do |mon,i|
  if $argv[:all] ||
      ($argv[:index] && i==$argv[:index]) ||
      ($argv[:primary] && mon.primary?)
    mon.puts
    puts unless $argv[:quiet]
  end
end
