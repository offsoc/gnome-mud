SUBDIRS = icons

gsettings_SCHEMAS = \
	org.gnome.MUD.gschema.xml \
	$(NULL)

@GSETTINGS_RULES@

EXTRA_DIST = \
	org.gnome.MUD.gresource.xml \
	main.ui \
	muds.ui \
	prefs.ui \
	$(NULL)

CLEANFILES = $(BUILT_SOURCES)

-include $(top_srcdir)/git.mk
