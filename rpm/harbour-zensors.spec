Name:       harbour-zensors
Summary:    Every sensor and meter in one list
Version:    0.1
Release:    1
Group:      Qt/Qt
License:    LICENSE
URL:        http://example.org/
Source0:    %{name}-%{version}.tar.bz2

Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   qt5-qtdeclarative-import-sensors
Requires:   qt5-qtdeclarative-import-positioning
BuildRequires: pkgconfig(sailfishapp) >= 1.0.2
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(Qt5Multimedia)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: desktop-file-utils

%description
Reads every measurement the OS exposes to an application - performance,
battery, storage, thermals, display, network, radio, Bluetooth, location,
motion, microphone and system facts - and lists them per category, with
probes that degrade to "not available" on hardware that does not have them.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 harbour-zensors.pro
%make_build

%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
%{_bindir}/harbour-zensors
%{_datadir}/harbour-zensors
%{_datadir}/applications/harbour-zensors.desktop
%{_datadir}/icons/hicolor/*/apps/harbour-zensors.png
