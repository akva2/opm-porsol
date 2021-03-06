dnl -*- autoconf -*-
# Macros needed to find opm-porsol and dependent libraries.  They are called by
# the macros in ${top_src_dir}/dependencies.m4, which is generated by
# "dunecontrol autogen"

# Additional checks needed to build opm-porsol
# This macro should be invoked by every module which depends on opm-porsol, as
# well as by opm-porsol itself
AC_DEFUN([OPM_PORSOL_CHECKS],
[
        dnl Check for "constexpr" support.
        dnl Needed in some (optional) parts of the fluid system
        dnl implementation.
        AC_REQUIRE([AX_CXX0X_CONSTEXPR])

        # BLAS and LAPACK support.
        #
        # NOTE: ACX_LAPACK internally AC_REQUIRE's ACX_BLAS which,
        # subsequently, AC_REQUIRE's AC_F77_LIBRARY_LDFLAGS which sets
        # the $FLIBS macro.
        AC_REQUIRE([AC_F77_WRAPPERS])
        AC_REQUIRE([AX_LAPACK])
        AS_IF([test "$ax_lapack_ok" != "yes"],dnl
              [AC_MSG_ERROR([No suitable LAPACK library found!])],dnl
              [:])

        DUNE_ADD_MODULE_DEPS([OPM_PORSOL],dnl
                             [OPM_PORSOL],dnl
          [],[],dnl
          [[$LAPACK_LIBS] [$BLAS_LIBS] [$FLIBS]])

        # Additional summary entries.
        DUNE_ADD_SUMMARY_ENTRY([BLAS], [$ax_blas_ok])
        DUNE_ADD_SUMMARY_ENTRY([LAPACK], [$ax_lapack_ok])
])

# Additional checks needed to find opm-porsol
# This macro should be invoked by every module which depends on opm-porsol, but
# not by opm-porsol itself
AC_DEFUN([OPM_PORSOL_CHECK_MODULE],
[
  OPM_PORSOL_CHECK_MODULES([opm-porsol],
                           [opm/porsol/mimetic/IncompFlowSolverHybrid.hpp],
                           [int i;])
])
