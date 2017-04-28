AC_DEFUN([LX_CHECK_COIN],
[
  AC_ARG_WITH([coin],
AS_HELP_STRING([--with-coin@<:@=PREFIX@:>@], [search for CLP under PREFIX or under the default search paths if PREFIX is not given @<:@default@:>@])
AS_HELP_STRING([--without-coin], [disable checking for CLP]),
              [], [with_coin=yes])

  AC_ARG_WITH([coin-includedir],
AS_HELP_STRING([--with-coin-includedir=DIR], [search for CLP headers in DIR]),
              [], [with_coin_includedir=no])

  AC_ARG_WITH([coin-libdir],
AS_HELP_STRING([--with-coin-libdir=DIR], [search for CLP libraries in DIR]),
              [], [with_coin_libdir=no])

  lx_clp_found=no
  if test x"$with_coin" != x"no"; then
    AC_MSG_CHECKING([for CLP])

    if test x"$with_coin_includedir" != x"no"; then
      CLP_CXXFLAGS="-I$with_coin_includedir"
    elif test x"$with_coin" != x"yes"; then
      CLP_CXXFLAGS="-I$with_coin/include"
    fi

    if test x"$with_coin_libdir" != x"no"; then
      CLP_LDFLAGS="-L$with_coin_libdir"
    elif test x"$with_coin" != x"yes"; then
      CLP_LDFLAGS="-L$with_coin/lib"
    fi
    CLP_LIBS="-lClp -lCoinUtils -lm"

    lx_save_cxxflags="$CXXFLAGS"
    lx_save_ldflags="$LDFLAGS"
    lx_save_libs="$LIBS"
    CXXFLAGS="$CLP_CXXFLAGS"
    LDFLAGS="$CLP_LDFLAGS"
    LIBS="$CLP_LIBS"

    lx_clp_test_prog='
      #include <coin/ClpModel.hpp>

      int main(int argc, char** argv)
      {
        ClpModel clp;
        return 0;
      }'

    AC_LANG_PUSH(C++)
    AC_LINK_IFELSE([$lx_clp_test_prog], [lx_clp_found=yes], [lx_clp_found=no])
    AC_LANG_POP(C++)

    CXXFLAGS="$lx_save_cxxflags"
    LDFLAGS="$lx_save_ldflags"
    LIBS="$lx_save_libs"

    if test x"$lx_clp_found" = x"yes"; then
      AC_DEFINE([LEMON_HAVE_CLP], [1], [Define to 1 if you have CLP.])
      lx_lp_found=yes
      AC_DEFINE([LEMON_HAVE_LP], [1], [Define to 1 if you have any LP solver.])
      AC_MSG_RESULT([yes])
    else
      CLP_CXXFLAGS=""
      CLP_LDFLAGS=""
      CLP_LIBS=""
      AC_MSG_RESULT([no])
    fi
  fi
  CLP_LIBS="$CLP_LDFLAGS $CLP_LIBS"
  AC_SUBST(CLP_CXXFLAGS)
  AC_SUBST(CLP_LIBS)
  AM_CONDITIONAL([HAVE_CLP], [test x"$lx_clp_found" = x"yes"])


  lx_cbc_found=no
  if test x"$lx_clp_found" = x"yes"; then
    if test x"$with_coin" != x"no"; then
      AC_MSG_CHECKING([for CBC])

      if test x"$with_coin_includedir" != x"no"; then
        CBC_CXXFLAGS="-I$with_coin_includedir"
      elif test x"$with_coin" != x"yes"; then
        CBC_CXXFLAGS="-I$with_coin/include"
      fi

      if test x"$with_coin_libdir" != x"no"; then
        CBC_LDFLAGS="-L$with_coin_libdir"
      elif test x"$with_coin" != x"yes"; then
        CBC_LDFLAGS="-L$with_coin/lib"
      fi
      CBC_LIBS="-lOsi -lCbc -lCbcSolver -lClp -lOsiClp -lCoinUtils -lVol -lOsiVol -lCgl -lm -llapack -lblas"

      lx_save_cxxflags="$CXXFLAGS"
      lx_save_ldflags="$LDFLAGS"
      lx_save_libs="$LIBS"
      CXXFLAGS="$CBC_CXXFLAGS"
      LDFLAGS="$CBC_LDFLAGS"
      LIBS="$CBC_LIBS"

      lx_cbc_test_prog='
        #include <coin/CbcModel.hpp>

        int main(int argc, char** argv)
        {
          CbcModel cbc;
          return 0;
        }'

      AC_LANG_PUSH(C++)
      AC_LINK_IFELSE([$lx_cbc_test_prog], [lx_cbc_found=yes], [lx_cbc_found=no])
      AC_LANG_POP(C++)

      CXXFLAGS="$lx_save_cxxflags"
      LDFLAGS="$lx_save_ldflags"
      LIBS="$lx_save_libs"

      if test x"$lx_cbc_found" = x"yes"; then
        AC_DEFINE([LEMON_HAVE_CBC], [1], [Define to 1 if you have CBC.])
        lx_lp_found=yes
        AC_DEFINE([LEMON_HAVE_LP], [1], [Define to 1 if you have any LP solver.])
        lx_mip_found=yes
        AC_DEFINE([LEMON_HAVE_MIP], [1], [Define to 1 if you have any MIP solver.])
        AC_MSG_RESULT([yes])
      else
        CBC_CXXFLAGS=""
        CBC_LDFLAGS=""
        CBC_LIBS=""
        AC_MSG_RESULT([no])
      fi
    fi
  fi
  CBC_LIBS="$CBC_LDFLAGS $CBC_LIBS"
  AC_SUBST(CBC_CXXFLAGS)
  AC_SUBST(CBC_LIBS)
  AM_CONDITIONAL([HAVE_CBC], [test x"$lx_cbc_found" = x"yes"])
])
