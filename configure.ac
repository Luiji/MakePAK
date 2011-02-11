################################################################################
# configure.ac: Execute autogen.sh to produce a configure script from this file.
# Copyright (C) 2011 Entertaining Software, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

AC_PREREQ([2.65])
AC_INIT([MakePAK], [0.0.0], [luiji@users.sourceforge.net],,
	[http://github.com/Luiji/MakePAK/])
AC_COPYRIGHT([Copyright (C) 2011 Entertaining Software, Inc.])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/pak.c])
AC_CONFIG_HEADERS([config.h])
# GNU standards are a primary target of the project, but refusing to produce
# build scripts because of not following standards is unreasonable.
AM_INIT_AUTOMAKE([foreign dist-xz])

# Checks for C compiler.
AC_PROG_CC

# Initialize Gnulib as soon as possible after AC_PROG_CC.
gl_EARLY
gl_INIT

# Checks for help2man (not required).
if test $cross_compiling = no; then
  AM_MISSING_PROG(HELP2MAN, help2man)
else
  HELP2MAN=:
fi

# Checks for endianess.
AC_C_BIGENDIAN(
  AC_DEFINE([ENCODE_BIG_ENDIAN], 1, [Is the current CPU is big-endian?]),
  AC_DEFINE([ENCODE_BIG_ENDIAN], 0, [Is the current CPU is big-endian?]))

# Translation support has been suspended because I could not get it to work. Any
# help on the subject would be greatly appreciated.
dnl # Translations support from Gettext.
dnl AM_GNU_GETTEXT_VERSION([0.17])
dnl AM_GNU_GETTEXT([external])

# Export configurations.
AC_CONFIG_FILES([Makefile man/Makefile doc/Makefile lib/Makefile src/Makefile])
AC_OUTPUT

dnl vim:set ts=8 sts=2 sw=2 noet:
