zmp_sources = [
  'zmp-package-interface.c',
  'zmp-package-interface.h',
  'zmp-core.c',
  'zmp-core.h',
  'zmp-main.c',
  'zmp-main.h',
  'zmp-subwindow.c',
  'zmp-subwindow.h'
]

libzmp = static_library(
  'zmp',
  zmp_sources,
  dependencies: [libgio_dep, libgtk_dep, libgst_dep, libvte_dep], # TODO: get rid of gtk and vte here? Also gst is badly depended on via telnethandlers header
  c_args: gnome_mud_cflags,
  include_directories: [top_inc, src_inc]
)
