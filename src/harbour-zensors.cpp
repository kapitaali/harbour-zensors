/*
 * Application entry point.
 *
 * Uses the explicit SailfishApp pattern instead of SailfishApp::main() so
 * that the C++ backend (SystemProbe) can be handed to QML as the "probe"
 * context property before the root view is shown.
 *
 *   SailfishApp::application() -> QGuiApplication *
 *   SailfishApp::createView()  -> QQuickView *
 *   SailfishApp::pathTo(name)  -> QUrl into the installed files
 */

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <sailfishapp.h>
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include <QScopedPointer>
#include <QScreen>

#include "systemprobe.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    SystemProbe probe;
    view->rootContext()->setContextProperty("probe", &probe);
    probe.setWindow(view.data());   // SIGUSR1 -> window grab for SSH verification

    view->setSource(SailfishApp::pathTo("qml/harbour-zensors.qml"));
    view->showFullScreen();

    QScreen *screen = QGuiApplication::primaryScreen();
    qInfo("screen %dx%d dpr %.2f logical %dx%d",
          screen->size().width(), screen->size().height(),
          screen->devicePixelRatio(),
          int(screen->size().width() / screen->devicePixelRatio()),
          int(screen->size().height() / screen->devicePixelRatio()));

    return app->exec();
}
