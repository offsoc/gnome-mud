subdir('icons')

gresource = files('org.gnome.MUD.gresource.xml')

resource_data = files(
  'main.ui',
  'muds.ui',
  'prefs.ui'
)

desktop_file = i18n.merge_file(
  input: 'org.gnome.MUD.desktop.in',
  output: 'org.gnome.MUD.desktop',
  type: 'desktop',
  po_dir: '../po',
  install: true,
  install_dir: desktopdir,
)

desktop_validator = find_program('desktop-file-validate', required: false)
if desktop_validator.found()
  test('Validate desktop file', desktop_validator, args: [desktop_file])
endif

install_data('org.gnome.MUD.gschema.xml', install_dir: schemadir)
