# define our project
project(
  'vterm',
  ['c', 'cpp'],
  version: '0.1',
  default_options: [
    'buildtype=release',
    'c_std=gnu11',
    'cpp_std=gnu++17',
    'warning_level=0',
    'b_ndebug=false',
  ],
  meson_version: '>= 0.50.0',
)

# dependencies
gtkdep = dependency('gtk+-3.0')

# our custom vte
libvte_proj = subproject('vte-vterm')
libvte_dep = libvte_proj.get_variable('libvte_gtk3_static_dep')

# sources in our project
src = ['src/v_term.cc', 'src/v_tab.cc', 'src/v_mode.cc']

args = []
if get_option('buildtype').contains('debug')
  args = ['-DVTERM_DEBUG=1']
endif

# finally our exec
executable('vterm', 
    sources:src, 
    dependencies : [gtkdep, libvte_dep], 
    cpp_args: args,
    install: true)

