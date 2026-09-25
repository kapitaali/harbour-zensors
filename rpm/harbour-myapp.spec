Name:       harbour-myapp
Summary:    My Sailfish OS Application
Version:    0.1
Release:    1
Group:      Qt/Qt
License:    LICENSE
URL:        http://example.org/
Source0:    %{name}-%{version}.tar.bz2

Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires: pkgconfig(sailfishapp) >= 1.0.2
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: desktop-file-utils

%description
Short description of my Sailfish OS Application.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 harbour-myapp.pro
%make_build

%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
%{_bindir}/harbour-myapp
%{_datadir}/harbour-myapp
%{_datadir}/applications/harbour-myapp.desktop
%{_datadir}/icons/hicolor/*/apps/harbour-myapp.png
