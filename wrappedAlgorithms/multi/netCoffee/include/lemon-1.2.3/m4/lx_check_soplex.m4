AC_DEFUN([LX_CHECK_SOPLEX],
[
  AC_ARG_WITH([soplex],
AS_HELP_STRING([--with-soplex@<:@=PREFIX@:>@], [search for SOPLEX under PREFIX or under the default search paths if PREFIX is not given @<:@default@:>@])
AS_HELP_STRING([--without-soplex], [disable checking for SOPLEX]),
              [], [with_soplex=yes])

  AC_ARG_WITH([soplex-includedir],
AS_HELP_STRING([--with-soplex-includedir=DIR], [search for SOPLEX headers in DIR]),
              [], [with_soplex_includedir=no])

  AC_ARG_WITH([soplex-libdir],
AS_HELP_STRING([--with-soplex-libdir=DIR], [search for SOPLEX libraries in DIR]),
              [], [with_soplex_libdir=no])

  lx_soplex_found=no
  if test x"$with_soplex" != x"no"; then
    AC_MSG_CHECKING([for SOPLEX])

    if test x"$with_soplex_includedir" != x"no"; then
      SOPLEX_CXXFLAGS="-I$with_soplex_includedir"
    elif test x"$with_soplex" != x"yes"; then
      SOPLEX_CXXFLAGS="-I$with_soplex/src"
    fi

    if test x"$with_soplex_libdir" != x"no"; then
      SOPLEX_LDFLAGS="-L$with_soplex_libdir"
    elif test x"$with_soplex" != x"yes"; then
      SOPLEX_LDFLAGS="-L$with_soplex/lib"
    fi
    SOPLEX_LIBS="-lsoplex -lz"

    lx_save_cxxflags="$CXXFLAGS"
    lx_save_ldflags="$LDFLAGS"
    lx_save_libs="$LIBS"
    CXXFLAGS="$SOPLEX_CXXFLAGS"
    LDFLAGS="$SOPLEX_LDFLAGS"
    LIBS="$SOPLEX_LIBS"

    lx_soplex_test_prog='
      #include <soplex.h>

      int main(int argc, char** argv)
      {
        soplex::SoPlex soplex;
        return 0;
      }'

    AC_LANG_PUSH(C++)
    AC_LINK_IFELSE([$lx_soplex_test_prog], [lx_soplex_found=yes], [lx_soplex_found=no])
    AC_LANG_POP(C++)

    CXXFLAGS="$lx_save_cxxflags"
    LDFLAGS="$lx_save_ldflags"
    LIBS="$lx_save_libs"

    if test x"$lx_soplex_found" = x"yes"; then
      AC_DEFINE([LEMON_HAVE_SOPLEX], [1], [Define to 1 if you have SOPLEX.])
      lx_lp_found=yes
      AC_DEFINE([LEMON_HAVE_LP], [1], [Define to 1 if you have any LP solver.])
      AC_MSG_RESULT([yes])
    else
      SOPLEX_CXXFLAGS=""
      SOPLEX_LDFLAGS=""
      SOPLEX_LIBS=""
      AC_MSG_RESULT([no])
    fi
  fi
  SOPLEX_LIBS="$SOPLEX_LDFLAGS $SOPLEX_LIBS"
  AC_SUBST(SOPLEX_CXXFLAGS)
  AC_SUBST(SOPLEX_LIBS)
  AM_CONDITIONAL([HAVE_SOPLEX], [test x"$lx_soplex_found" = x"yes"])
])
