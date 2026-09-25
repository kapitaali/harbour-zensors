TARGET = harbour-myapp
CONFIG += sailfishapp sailfishapp_i18n c++11
QT += dbus multimedia

SOURCES += src/harbour-myapp.cpp \
           src/systemprobe.cpp

HEADERS += src/systemprobe.h

DISTFILES += qml/harbour-myapp.qml \
             qml/pages/MainPage.qml \
             qml/pages/CategoryPage.qml \
             qml/pages/LocationProbe.qml \
             qml/pages/SensorsProbe.qml \
             qml/pages/FirstPage.qml \
             qml/backend/metrics.js \
             qml/cover/CoverPage.qml \
             rpm/harbour-myapp.spec \
             rpm/harbour-myapp.changes \
             rpm/harbour-myapp.changes.run \
             harbour-myapp.desktop \
             translations/*.ts \
             tests/auto/tests.xml \
             tests/auto/tst_firstpage.qml

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

TRANSLATIONS += translations/harbour-myapp-de.ts
