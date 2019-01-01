SUBDIRS = data doc po src ui

@INTLTOOL_SCHEMAS_RULE@
@INTLTOOL_DESKTOP_RULE@

Applicationsdir = $(datadir)/applications
Applications_in_files = gnome-mud.desktop.in
Applications_DATA = $(Applications_in_files:.desktop.in=.desktop)

# Removed gnome-doc-utils.make. Will need to be replaced with
# updated docs.

EXTRA_DIST = \
	ABOUT-NLS \
	AUTHORS \
	BUGS \
	COPYING \
	ChangeLog \
	INSTALL \
	NEWS \
	README \
	ROADMAP \
	config.rpath \
	gnome-mud.desktop.in \
	gnome-mud.spec \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	$(Applications_DATA) \
	$(schema_DATA)

DISTCLEANFILES =		\
	intltool-extract	\
	intltool-merge		\
	intltool-update

MAINTAINERCLEANFILES = \
	INSTALL \
	aclocal.m4 \
	compile \
	config.guess \
	config.h.in \
	config.sub \
	configure.scan \
	depcomp \
	install-sh \
	ltmain.sh \
	missing \
	mkinstalldirs \
	gnome-mud.desktop \
	gnome-mud.schemas


DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install --disable-scrollkeeper

-include $(top_srcdir)/git.mk
