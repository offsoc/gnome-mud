dnl
dnl GNOME_LIBGTOP_TYPES
dnl
dnl some typechecks for libgtop.
dnl

AC_DEFUN([GNOME_LIBGTOP_TYPES],
[
	AC_CHECK_TYPE(u_int64_t, unsigned long long int)
	AC_CHECK_TYPE(int64_t, long long int)
])

dnl
dnl GNOME_LIBGTOP_HOOK (minversion, script-if-libgtop-enabled, failflag)
dnl
dnl if failflag is "fail" then GNOME_LIBGTOP_HOOK will abort if LibGTop
dnl is not found. 
dnl

AC_DEFUN([GNOME_LIBGTOP_HOOK],
[	
	AC_REQUIRE([GNOME_LIBGTOP_TYPES])

	AC_SUBST(LIBGTOP_LIBDIR)
	AC_SUBST(LIBGTOP_INCLUDEDIR)
	AC_SUBST(LIBGTOP_EXTRA_LIBS)
	AC_SUBST(LIBGTOP_LIBS)
	AC_SUBST(LIBGTOP_INCS)
	AC_SUBST(LIBGTOP_NAMES_LIBS)
	AC_SUBST(LIBGTOP_NAMES_INCS)
	AC_SUBST(LIBGTOP_GUILE_INCS)
	AC_SUBST(LIBGTOP_GUILE_LIBS)
	AC_SUBST(LIBGTOP_GUILE_NAMES_INCS)
	AC_SUBST(LIBGTOP_GUILE_NAMES_LIBS)
	AC_SUBST(LIBGTOP_MAJOR_VERSION)
	AC_SUBST(LIBGTOP_MINOR_VERSION)
	AC_SUBST(LIBGTOP_MICRO_VERSION)
	AC_SUBST(LIBGTOP_VERSION)
	AC_SUBST(LIBGTOP_VERSION_CODE)
	AC_SUBST(LIBGTOP_SERVER_VERSION)
	AC_SUBST(LIBGTOP_INTERFACE_AGE)
	AC_SUBST(LIBGTOP_BINARY_AGE)
	AC_SUBST(LIBGTOP_BINDIR)
	AC_SUBST(LIBGTOP_SERVER)

	dnl Get the cflags and libraries from the libgtop-config script
	dnl
	AC_ARG_WITH(libgtop,
	[  --with-libgtop=PFX      Prefix where LIBGTOP is installed (optional)],
	libgtop_config_prefix="$withval", libgtop_config_prefix="")
	AC_ARG_WITH(libgtop-exec,
	[  --with-libgtop-exec=PFX Exec prefix where LIBGTOP is installed (optional)],
	libgtop_config_exec_prefix="$withval", libgtop_config_exec_prefix="")

	if test x$libgtop_config_exec_prefix != x ; then
	  libgtop_config_args="$libgtop_config_args --exec-prefix=$libgtop_config_exec_prefix"
	  if test x${LIBGTOP_CONFIG+set} != xset ; then
	    LIBGTOP_CONFIG=$libgtop_config_exec_prefix/bin/libgtop-config
	  fi
	fi
	if test x$libgtop_config_prefix != x ; then
	  libgtop_config_args="$libgtop_config_args --prefix=$libgtop_config_prefix"
	  if test x${LIBGTOP_CONFIG+set} != xset ; then
	    LIBGTOP_CONFIG=$libgtop_config_prefix/bin/libgtop-config
	  fi
	fi

	AC_PATH_PROG(LIBGTOP_CONFIG, libgtop-config, no)
	dnl IMPORTANT NOTICE:
	dnl   If you increase this number here, this means that *ALL*
	dnl   modules will require the new version, even if they explicitly
	dnl   give a lower number in their `configure.in' !!!
	real_min_libgtop_version=0.26.1
	min_libgtop_version=ifelse([$1], ,$real_min_libgtop_version,$1)
	dnl I know, the following code looks really ugly, but if you want
	dnl to make changes, please test it with a brain-dead /bin/sh and
	dnl with a brain-dead /bin/test (not all shells/tests support the
	dnl `<' operator to compare strings, that's why I convert everything
	dnl into numbers and test them).
	min_libgtop_major=`echo $min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	min_libgtop_minor=`echo $min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	min_libgtop_micro=`echo $min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
	test x$min_libgtop_micro = x && min_libgtop_micro=0
	real_min_libgtop_major=`echo $real_min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	real_min_libgtop_minor=`echo $real_min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	real_min_libgtop_micro=`echo $real_min_libgtop_version | \
	  sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
	test x$real_min_libgtop_micro = x && real_min_libgtop_micro=0
	dnl You cannot require a version less then $real_min_libgtop_version,
	dnl so you don't need to update each `configure.in' when it's increased.
	if test $real_min_libgtop_major -gt $min_libgtop_major ; then
	  min_libgtop_major=$real_min_libgtop_major
	  min_libgtop_minor=$real_min_libgtop_minor
	  min_libgtop_micro=$real_min_libgtop_micro
	elif test $real_min_libgtop_major = $min_libgtop_major ; then
	  if test $real_min_libgtop_minor -gt $min_libgtop_minor ; then
	    min_libgtop_minor=$real_min_libgtop_minor
	    min_libgtop_micro=$real_min_libgtop_micro
	  elif test $real_min_libgtop_minor = $min_libgtop_minor ; then
	    if test $real_min_libgtop_micro -gt $min_libgtop_micro ; then
	      min_libgtop_micro=$real_min_libgtop_micro
	    fi
	  fi
	fi
	min_libgtop_version="$min_libgtop_major.$min_libgtop_minor.$min_libgtop_micro"
	AC_MSG_CHECKING(for libgtop - version >= $min_libgtop_version)
	no_libgtop=""
	if test "$LIBGTOP_CONFIG" = "no" ; then
	  no_libgtop=yes
	else
	  configfile=`$LIBGTOP_CONFIG --config`
	  libgtop_major_version=`$LIBGTOP_CONFIG --version | \
	    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
	  libgtop_minor_version=`$LIBGTOP_CONFIG --version | \
	    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
	  libgtop_micro_version=`$LIBGTOP_CONFIG --version | \
	    sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
	  test $libgtop_major_version != $min_libgtop_major && no_libgtop=yes
	  test $libgtop_minor_version -lt $min_libgtop_minor && no_libgtop=yes
	  if test $libgtop_minor_version = $min_libgtop_minor ; then
	    test $libgtop_micro_version -lt $min_libgtop_micro && no_libgtop=yes
	  fi
	  . $configfile
	fi
	if test x$no_libgtop = x ; then
	  AC_DEFINE(HAVE_LIBGTOP)
	  AC_DEFINE_UNQUOTED(LIBGTOP_VERSION, "$LIBGTOP_VERSION")
	  AC_DEFINE_UNQUOTED(LIBGTOP_VERSION_CODE, $LIBGTOP_VERSION_CODE)
	  AC_DEFINE_UNQUOTED(LIBGTOP_MAJOR_VERSION, $LIBGTOP_MAJOR_VERSION)
	  AC_DEFINE_UNQUOTED(LIBGTOP_MINOR_VERSION, $LIBGTOP_MINOR_VERSION)
	  AC_DEFINE_UNQUOTED(LIBGTOP_MICRO_VERSION, $LIBGTOP_MICRO_VERSION)
	  AC_DEFINE_UNQUOTED(LIBGTOP_SERVER_VERSION, $LIBGTOP_SERVER_VERSION)
	  AC_MSG_RESULT(yes)
	  dnl Note that an empty true branch is not valid sh syntax.
	  ifelse([$2], [], :, [$2])
	else
	  AC_MSG_RESULT(no)
	  if test "x$3" = "xfail"; then
	    AC_MSG_ERROR(LibGTop >= $min_libgtop_version not found)
	  else
	    AC_MSG_WARN(LibGTop >= $min_libgtop_version not found)
	  fi
	fi

	AM_CONDITIONAL(HAVE_LIBGTOP, test x$no_libgtop != xyes)
])

AC_DEFUN([GNOME_INIT_LIBGTOP],[
	GNOME_LIBGTOP_HOOK($1,[],$2)
])
