#!/bin/sh
################################################################################
# autogen.sh: Imports Gnulib modules and produces build system for MakePAK.
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

set -e # quit on first error
set -x # print commands

name=`basename "$0"`

case "x$1" in
  x)
    echo "Generating build system..."
    shift || true
    echo "Importing Gnulib..."
    gnulib-tool --import isdir dirent stdint stdio stdarg locale stdlib string \
		xalloc realloc-gnu getopt-gnu vasprintf byteswap error
    echo "Configuring build system..."
    autoreconf --force --install --verbose
    echo "Generated build system.";;

  xhelp)
    echo "Usage: $name [ACTION]"
    echo
    echo "Generates Autotools build system for Git checkout."
    echo
    echo "Actions:"
    echo "  help   print this help message"
    echo "  clean  remove generated files"
    echo
    echo "If no actions are specified it will generate the build system."
    echo
    echo "Report bugs to: luiji@users.sourceforge.net"
    echo "$name home page: <http://github.com/Luiji/makepak>";;

  xclean)
    if [ -f Makefile ]; then
      echo "Trying 'make maintainer-clean...'"
      if ! make maintainer-clean; then
	echo "WARNING: 'make maintainer-clean' failed, continuing anyway..."
      fi
    fi
    echo "Removing various generated files..."
    rm -rf aclocal.m4 config.h.in config.h.in~ configure autom4te.cache/ \
       build-aux lib m4
    echo "Removing 'Makefile.in's recursively..."
    for file in `find -name Makefile.in`; do
      rm -f $file
    done
    echo "Generated files removed.";;

  *)
    echo "$name: Unrecognized option $1." >&2
    echo "Try '$name help'.";;
esac

# vim:set ts=8 sts=2 sw=2 noet:
