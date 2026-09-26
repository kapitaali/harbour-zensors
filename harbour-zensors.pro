TARGET = harbour-zensors
CONFIG += sailfishapp sailfishapp_i18n c++11
QT += dbus multimedia

# GLib is used for exactly one thing: reading D-Bus property maps. Sailjail
# runs the app with --private-bin=<app-name>, so /usr/bin inside the sandbox
# holds only harbour-zensors and an external "gdbus" cannot be spawned. GDBus
# (libgio) answers a{sv} in-process, where Qt's demarshaller crashes.
#
# Do NOT add "CONFIG += link_pkgconfig" here: sailfishapp.prf already adds it,
# and repeating it makes qmake expand PKGCONFIG before that feature has
# contributed its own entry, which silently drops -lsailfishapp from the link.
PKGCONFIG += gio-2.0

SOURCES += src/harbour-zensors.cpp \
           src/systemprobe.cpp

HEADERS += src/systemprobe.h

DISTFILES += qml/harbour-zensors.qml \
             qml/pages/MainPage.qml \
             qml/pages/CategoryPage.qml \
             qml/pages/LocationProbe.qml \
             qml/pages/SensorsProbe.qml \
             qml/pages/FirstPage.qml \
             qml/backend/metrics.js \
             qml/cover/CoverPage.qml \
             rpm/harbour-zensors.spec \
             rpm/harbour-zensors.changes \
             rpm/harbour-zensors.changes.run \
             harbour-zensors.desktop \
             translations/*.ts \
             tests/auto/tests.xml \
             tests/auto/tst_firstpage.qml

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

TRANSLATIONS += translations/harbour-zensors-de.ts
