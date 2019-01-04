#!/bin/sh

if [ -z "$DESTDIR" ]; then
  echo Compiling GSettings schemas...
  glib-compile-schemas ${MESON_INSTALL_PREFIX}/share/glib-2.0/schemas

  echo Updating desktop database...
  update-desktop-database -q ${MESON_INSTALL_PREFIX}/share/applications
fi