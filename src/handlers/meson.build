telnet_handler_sources = [
  'mud-telnet-charset.c',
  'mud-telnet-charset.h',
  'mud-telnet-echo.c',
  'mud-telnet-echo.h',
  'mud-telnet-eor.c',
  'mud-telnet-eor.h',
  'mud-telnet-handlers.h',
  'mud-telnet-handler-interface.c',
  'mud-telnet-handler-interface.h',
  'mud-telnet-mssp.c',
  'mud-telnet-mssp.h',
  'mud-telnet-naws.c',
  'mud-telnet-naws.h',
  'mud-telnet-new-environ.c',
  'mud-telnet-new-environ.h',
  'mud-telnet-ttype.c',
  'mud-telnet-ttype.h',
  'mud-telnet-zmp.c',
  'mud-telnet-zmp.h'
]

if libgst_dep.found()
  telnet_handler_sources += [
    'mud-telnet-msp.c',
    'mud-telnet-msp.h',
  ]
endif

if libz_dep.found()
  telnet_handler_sources += [
    'mud-telnet-mccp.c',
    'mud-telnet-mccp.h',
  ]
endif

libtelnethandlers = static_library(
  'telnethandlers',
  telnet_handler_sources,
  dependencies: [libgio_dep, libgst_dep, libz_dep, libgtk_dep, libvte_dep], # TODO: get rid of gtk and vte here?
  c_args: gnome_mud_cflags,
  include_directories: [top_inc, src_inc]
)
