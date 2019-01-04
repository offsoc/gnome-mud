project(
  'gnome-mud', 'c',
  license: 'GPL2+',
  version: '0.11.3',
  default_options: [
    'warning_level=2', # TODO: Try 3 after 2 is clean
    'buildtype=debugoptimized',
    'c_std=gnu11',
  ],
  meson_version: '>= 0.47.0'
)

gnome = import('gnome')
i18n = import('i18n')

prefix = get_option('prefix')

bindir = join_paths(prefix, get_option('bindir'))
libdir = join_paths(prefix, get_option('libdir'))
datadir = join_paths(prefix, get_option('datadir'))

desktopdir =  join_paths(datadir, 'applications')
icondir = join_paths(datadir, 'icons')
localedir = join_paths(datadir, 'locale')
schemadir = join_paths(datadir, 'glib-2.0', 'schemas')

libgio_dep = dependency('gio-2.0', version: '>= 2.48')
libgtk_dep = dependency('gtk+-3.0', version: '>= 3.4')
libvte_dep = dependency('vte-2.91', version: '>= 0.37')
libpcre_dep = dependency('libpcre', version: '>= 6.0.0')
libgst_dep = dependency('gstreamer-1.0', version: '>= 1.0', required: get_option('gstreamer'))
libz_dep = dependency('zlib', required: get_option('mccp'))

status = [
  'Version ....... : @0@'.format(meson.project_version()),
  'Build Type .... : @0@'.format(get_option('buildtype')),
  'Prefix ........ : @0@'.format(get_option('prefix')),
  '',
  'GStreamer ..... : @0@'.format(libgst_dep.found()),
  'MCCP .......... : @0@'.format(libz_dep.found()),
  'Debug Logger .. : @0@'.format(get_option('debug-logger')),
  ''
]

config_h = configuration_data()

set_defines = [
  ['PACKAGE_URL', 'https://wiki.gnome.org/Apps/MUD'],
  ['VERSION', '@VCS_TAG@'],
  ['GETTEXT_PACKAGE', meson.project_name()],
  ['LOCALEDIR', localedir]
]

foreach define: set_defines
  config_h.set_quoted(define[0], define[1])
endforeach

config_h.set('ENABLE_NLS', true) # FIXME: remove conditionals from source code
config_h.set('ENABLE_GST', libgst_dep.found())
config_h.set('ENABLE_MCCP', libz_dep.found())
config_h.set('ENABLE_DEBUG_LOGGER', get_option('debug-logger'))

vcs_identifier_h = vcs_tag(
  input: configure_file(
    output: 'config.h.in',
    configuration: config_h
  ),
  output: 'config.h',
)

configure_file(
  output: 'config.h',
  configuration: config_h
)
top_inc = include_directories('.')

cc = meson.get_compiler('c')
global_c_args = []
test_c_args = [
  '-Wno-unused-parameter',
]

foreach arg: test_c_args
  if cc.has_multi_arguments(arg)
    global_c_args += arg
  endif
endforeach

add_project_arguments(global_c_args, language: 'c')

subdir('data')
subdir('src')
subdir('po')
subdir('doc')

meson.add_install_script('meson_post_install.sh')

message('\n  '.join(status))