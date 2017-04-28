AC_DEFUN([LX_CHECK_CPLEX],
[
  AC_ARG_WITH([cplex],
AS_HELP_STRING([--with-cplex@<:@=PREFIX@:>@], [search for CPLEX under PREFIX or under the default search paths if PREFIX is not given @<:@default@:>@])
AS_HELP_STRING([--without-cplex], [disable checking for CPLEX]),
              [], [with_cplex=yes])

  AC_ARG_WITH([cplex-includedir],
AS_HELP_STRING([--with-cplex-includedir=DIR], [search for CPLEX headers in DIR]),
              [], [with_cplex_includedir=no])

  AC_ARG_WITH([cplex-libdir],
AS_HELP_STRING([--with-cplex-libdir=DIR], [search for CPLEX libraries in DIR]),
              [], [with_cplex_libdir=no])

  lx_cplex_found=no
  if test x"$with_cplex" != x"no"; then
    AC_MSG_CHECKING([for CPLEX])

    if test x"$with_cplex_includedir" != x"no"; then
      CPLEX_CFLAGS="-I$with_cplex_includedir"
    elif test x"$with_cplex" != x"yes"; then
      CPLEX_CFLAGS="-I$with_cplex/include"
    elif test x"$CPLEX_INCLUDEDIR" != x; then
      CPLEX_CFLAGS="-I$CPLEX_INCLUDEDIR"
    fi

    if test x"$with_cplex_libdir" != x"no"; then
      CPLEX_LDFLAGS="-L$with_cplex_libdir"
    elif test x"$with_cplex" != x"yes"; then
      CPLEX_LDFLAGS="-L$with_cplex/lib"
    elif test x"$CPLEX_LIBDIR" != x; then
      CPLEX_LDFLAGS="-L$CPLEX_LIBDIR"
    fi
    CPLEX_LIBS="-lcplex -lm -lpthread"

    lx_save_cxxflags="$CXXFLAGS"
    lx_save_ldflags="$LDFLAGS"
    lx_save_libs="$LIBS"
    CXXFLAGS="$CPLEX_CFLAGS"
    LDFLAGS="$CPLEX_LDFLAGS"
    LIBS="$CPLEX_LIBS"

    lx_cplex_test_prog='
      extern "C" {
      #include <ilcplex/cplex.h>
      }

      int main(int argc, char** argv)
      {
        CPXENVptr env = NULL;
        return 0;
      }'

    AC_LANG_PUSH(C++)
    AC_LINK_IFELSE([$lx_cplex_test_prog], [lx_cplex_found=yes], [lx_cplex_found=no])
    AC_LANG_POP(C++)

    CXXFLAGS="$lx_save_cxxflags"
    LDFLAGS="$lx_save_ldflags"
    LIBS="$lx_save_libs"

    if test x"$lx_cplex_found" = x"yes"; then
      AC_DEFINE([LEMON_HAVE_CPLEX], [1], [Define to 1 if you have CPLEX.])
      lx_lp_found=yes
      AC_DEFINE([LEMON_HAVE_LP], [1], [Define to 1 if you have any LP solver.])
      lx_mip_found=yes
      AC_DEFINE([LEMON_HAVE_MIP], [1], [Define to 1 if you have any MIP solver.])
      AC_MSG_RESULT([yes])
    else
      CPLEX_CFLAGS=""
      CPLEX_LDFLAGS=""
      CPLEX_LIBS=""
      AC_MSG_RESULT([no])
    fi
  fi
  CPLEX_LIBS="$CPLEX_LDFLAGS $CPLEX_LIBS"
  AC_SUBST(CPLEX_CFLAGS)
  AC_SUBST(CPLEX_LIBS)
  AM_CONDITIONAL([HAVE_CPLEX], [test x"$lx_cplex_found" = x"yes"])
])
