TARGET = harbour-myapp
CONFIG += sailfishapp sailfishapp_i18n

SOURCES += src/harbour-myapp.cpp

DISTFILES += qml/harbour-myapp.qml \
             qml/pages/FirstPage.qml \
             qml/pages/SecondPage.qml \
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
