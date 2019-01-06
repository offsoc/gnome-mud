src_inc = include_directories('.')
gnome_mud_cflags = [
  '-include', 'config.h'
]

subdir('handlers')
subdir('zmp')

gnome_mud_sources = [
  'debug-logger.c',
  'debug-logger.h',
  'ecma48.h',
  'gnome-mud.c',
  'gnome-mud.h',
  'gnome-mud-icons.h',
  'mud-character.c',
  'mud-character.h',
  'mud-connection-view.c',
  'mud-connections.c',
  'mud-connections.h',
  'mud-connection-view.h',
  'mud-connection.c',
  'mud-connection.h',
  'mud-input-view.c',
  'mud-input-view.h',
  'mud-line-buffer.c',
  'mud-line-buffer.h',
  'mud-log.c',
  'mud-log.h',
  'mud-mud.c',
  'mud-mud.h',
  'mud-parse-alias.c',
  'mud-parse-alias.h',
  'mud-parse-base.c',
  'mud-parse-base.h',
  'mud-parse-trigger.c',
  'mud-parse-trigger.h',
  'mud-profile.c',
  'mud-profile.h',
  'mud-profile-manager.c',
  'mud-profile-manager.h',
  'mud-regex.c',
  'mud-regex.h',
  'mud-subwindow.c',
  'mud-subwindow.h',
  'mud-telnet.c',
  'mud-telnet.h',
  'mud-trigger.c',
  'mud-trigger.h',
  'mud-window.c',
  'mud-window.h',
  'mud-window-prefs.c',
  'mud-window-prefs.h',
  'mud-window-profile.c',
  'mud-window-profile.h',
  'utils.c',
  'utils.h'
]

gnome_mud_sources += gnome.compile_resources(
  'resources', gresource,
  source_dir: join_paths(meson.source_root(), 'data'),
  c_name: 'gnome_mud',
  dependencies: resource_data,
  export: true, # TODO: Once we don't need to use manual access, we can get rid of this?
)

enum_headers = files('mud-trigger.h')
enum_types = 'gnome-mud-builtins'
gnome_mud_sources += gnome.mkenums(
  enum_types,
  sources: enum_headers,
  c_template: enum_types + '.c.template',
  h_template: enum_types + '.h.template'
)

executable('gnome-mud', gnome_mud_sources,
  gui_app: true,
  link_with: [libtelnethandlers, libzmp],
  dependencies: [libgio_dep, libgtk_dep, libvte_dep, libpcre_dep, libgst_dep],
  c_args: gnome_mud_cflags,
  install: true
)
