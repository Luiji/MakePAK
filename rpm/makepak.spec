################################################################################
# makepak.spec.in: RPM package specification for MakePAK.rpm.
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

%define name makepak
%define version 0.0.0
%define release 1

Name: %{name}
Version: %{version}
Release: %{release}
Summary: Tool for creating Quake .PAK files
License: GPLv3+
URL: https://github.com/Luiji/MakePAK
Group: Productivity/Archiving/Backup
Source0: https://github.com/downloads/Luiji/MakePAK/makepak-0.0.0.tar.xz
BuildRequires: pkgconfig
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot

%description
MakePAK is a command-line utility for creating Quake I/II .PAK archives. It
follows the GNU Standards, providing a clean, portable, consistent, and complete
interface. It does not support extracting archives, as it is meant to be used
along with PhysicsFS (http://icculus.org/physfs/).

%prep
%setup -q

%pre
%info_del pak.info

%post
%info_add pak.info

%postun
%info_del pak.info

%build
%configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT INSTALL="install -p"

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README NEWS AUTHORS THANKS COPYING
%doc %{_mandir}/man1/pak.*
%doc %{_infodir}/pak.info*
%{_bindir}/pak

%changelog
* Fri Feb 11 2011 Luiji Maryo <luiji@users.sourceforge.net> - 0.0.0
- Initial package
