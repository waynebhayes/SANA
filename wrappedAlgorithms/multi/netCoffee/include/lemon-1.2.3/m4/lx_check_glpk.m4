AC_DEFUN([LX_CHECK_GLPK],
[
  AC_ARG_WITH([glpk],
AS_HELP_STRING([--with-glpk@<:@=PREFIX@:>@], [search for GLPK under PREFIX or under the default search paths if PREFIX is not given @<:@default@:>@])
AS_HELP_STRING([--without-glpk], [disable checking for GLPK]),
              [], [with_glpk=yes])

  AC_ARG_WITH([glpk-includedir],
AS_HELP_STRING([--with-glpk-includedir=DIR], [search for GLPK headers in DIR]),
              [], [with_glpk_includedir=no])

  AC_ARG_WITH([glpk-libdir],
AS_HELP_STRING([--with-glpk-libdir=DIR], [search for GLPK libraries in DIR]),
              [], [with_glpk_libdir=no])

  lx_glpk_found=no
  if test x"$with_glpk" != x"no"; then
    AC_MSG_CHECKING([for GLPK])

    if test x"$with_glpk_includedir" != x"no"; then
      GLPK_CFLAGS="-I$with_glpk_includedir"
    elif test x"$with_glpk" != x"yes"; then
      GLPK_CFLAGS="-I$with_glpk/include"
    fi

    if test x"$with_glpk_libdir" != x"no"; then
      GLPK_LDFLAGS="-L$with_glpk_libdir"
    elif test x"$with_glpk" != x"yes"; then
      GLPK_LDFLAGS="-L$with_glpk/lib"
    fi
    GLPK_LIBS="-lglpk"

    lx_save_cxxflags="$CXXFLAGS"
    lx_save_ldflags="$LDFLAGS"
    lx_save_libs="$LIBS"
    CXXFLAGS="$GLPK_CFLAGS"
    LDFLAGS="$GLPK_LDFLAGS"
    LIBS="$GLPK_LIBS"

    lx_glpk_test_prog='
      extern "C" {
      #include <glpk.h>
      }

      #if (GLP_MAJOR_VERSION < 4) \
         || (GLP_MAJOR_VERSION == 4 && GLP_MINOR_VERSION < 33)
      #error Supported GLPK versions: 4.33 or above
      #endif

      int main(int argc, char** argv)
      {
        LPX *lp;
        lp = lpx_create_prob();
        lpx_delete_prob(lp);
        return 0;
      }'

    AC_LANG_PUSH(C++)
    AC_LINK_IFELSE([$lx_glpk_test_prog], [lx_glpk_found=yes], [lx_glpk_found=no])
    AC_LANG_POP(C++)

    CXXFLAGS="$lx_save_cxxflags"
    LDFLAGS="$lx_save_ldflags"
    LIBS="$lx_save_libs"

    if test x"$lx_glpk_found" = x"yes"; then
      AC_DEFINE([LEMON_HAVE_GLPK], [1], [Define to 1 if you have GLPK.])
      lx_lp_found=yes
      AC_DEFINE([LEMON_HAVE_LP], [1], [Define to 1 if you have any LP solver.])
      lx_mip_found=yes
      AC_DEFINE([LEMON_HAVE_MIP], [1], [Define to 1 if you have any MIP solver.])
      AC_MSG_RESULT([yes])
    else
      GLPK_CFLAGS=""
      GLPK_LDFLAGS=""
      GLPK_LIBS=""
      AC_MSG_RESULT([no])
    fi
  fi
  GLPK_LIBS="$GLPK_LDFLAGS $GLPK_LIBS"
  AC_SUBST(GLPK_CFLAGS)
  AC_SUBST(GLPK_LIBS)
  AM_CONDITIONAL([HAVE_GLPK], [test x"$lx_glpk_found" = x"yes"])
])
