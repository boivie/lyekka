#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(lyekka, 0.1, victor-lyekka@boivie.com)
AM_INIT_AUTOMAKE(lyekka, 0.1)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_LANG_PUSH([C++])

lyekka_BOOST_CPPFLAGS=""
lyekka_BOOST_LDFLAGS=""

AC_ARG_WITH([boost-include],
 AS_HELP_STRING([--with-boost-include=DIR],
 [specify exact directory for Boost headers]),
 [lyekka_boost_include="$withval"])

AC_ARG_WITH([boost-libdir],
 AS_HELP_STRING([--with-boost-libdir=DIR],
 [specify exact directory for Boost libraries]),
 [lyekka_boost_libdir="$withval"])

if test "${lyekka_boost_include}"; then
 CPPFLAGS="$CPPFLAGS -I${lyekka_boost_include}"
fi

if test "${lyekka_boost_libdir}"; then
 LDFLAGS="$LDFLAGS -L${lyekka_boost_libdir}"
fi

AC_CHECK_HEADER(boost/filesystem.hpp, [], [AC_MSG_ERROR([header file <boost/filesystem.hpp> is required for boost filesystem])])
LIBS="$LIBS -lboost_filesystem-mt -lboost_system-mt -lboost_program_options-mt -lsqlite3 -lbotan -lprotobuf -lz -lssl -lexpat"

AC_CHECK_HEADER(google/protobuf/stubs/common.h,,
		  [AC_MSG_ERROR([
  ERROR: protobuf headers are required.

  You must either install protobuf from google,
  or if you have it installed in a custom location
  you must add '-Iincludedir' to CXXFLAGS
  and '-Llibdir' to LDFLAGS.
  ])])

AC_CHECK_HEADERS([sqlite3.h], , [AC_MSG_ERROR([sqlite3 header (sqlite3.h) not found])])
AC_CHECK_HEADERS([zlib.h], , [AC_MSG_ERROR([zlib header (zlib.h) not found])])

AC_CHECK_HEADERS(sys/endian.h)
AC_CHECK_HEADERS(machine/endian.h)
AC_CHECK_HEADERS(endian.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
