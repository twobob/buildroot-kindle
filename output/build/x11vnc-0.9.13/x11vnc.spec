# Note that this is NOT a relocatable package
Name: x11vnc
Version: 0.9.13
Release: 2
Summary: a VNC server for the current X11 session
Copyright: GPL
Group: Libraries/Network
Packager: Karl Runge <runge@karlrunge.com>
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot

%description
x11vnc is to X Window System what WinVNC is to Windows, i.e. a server
which serves the current Xwindows desktop via RFB (VNC) protocol
to the user.

Based on the ideas of x0rfbserver and on LibVNCServer, it has evolved
into a versatile and performant while still easy to use program.

x11vnc was put together and is (actively ;-) maintained by
Karl Runge <runge@karlrunge.com>



%package x11vnc
Requires:     %{name} = %{version}
Summary:      VNC server for the current X11 session
Group:        User Interface/X
Requires:     %{name} = %{version}

%description x11vnc
x11vnc is to X Window System what WinVNC is to Windows, i.e. a server
which serves the current X Window System desktop via RFB (VNC)
protocol to the user.

Based on the ideas of x0rfbserver and on LibVNCServer, it has evolved
into a versatile and performant while still easy to use program.

%prep
%setup -n %{name}-%{version}

%build
# CFLAGS="$RPM_OPT_FLAGS" ./configure --prefix=%{_prefix}
%configure
make

%install
[ -n "%{buildroot}" -a "%{buildroot}" != / ] && rm -rf %{buildroot}
# make install prefix=%{buildroot}%{_prefix}
%makeinstall includedir="%{buildroot}%{_includedir}/rfb"

%{__install} -d -m0755 %{buildroot}%{_datadir}/x11vnc/classes
%{__install} classes/VncViewer.jar classes/index.vnc \
  %{buildroot}%{_datadir}/x11vnc/classes

%clean
[ -n "%{buildroot}" -a "%{buildroot}" != / ] && rm -rf %{buildroot}

%pre
%post
%preun
%postun



%files
%doc README x11vnc/ChangeLog
%defattr(-,root,root)
%{_bindir}/x11vnc
%{_mandir}/man1/x11vnc.1*
%{_datadir}/x11vnc/classes

%changelog
* Fri Aug 19 2005 Alberto Lusiani <alusiani@gmail.com> release 2
- create separate package for x11vnc to prevent conflicts with x11vnc rpm
- create devel package, needed to compile but not needed for running
* Sun Feb 9 2003 Karl Runge
- created libvncserver.spec.in

