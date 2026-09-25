/*
 * Application entry point.
 *
 * SailfishApp::main() creates the QGuiApplication and QQuickView and loads
 * the QML file named after TARGET, i.e. qml/harbour-myapp.qml.
 * The file cannot be renamed without updating TARGET in harbour-myapp.pro.
 *
 * For more control over initialisation use instead:
 *   SailfishApp::application(int, char *[])  -> QGuiApplication *
 *   SailfishApp::createView()                -> QQuickView *
 *   SailfishApp::pathTo(QString)             -> QUrl to a resource file
 *   then call view->show() (fullscreen on device).
 */

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <sailfishapp.h>

int main(int argc, char *argv[])
{
    // If you expose C++ types to QML for a Harbour submission, register them
    // under a harbour.-prefixed namespace, e.g.:
    //   qmlRegisterType<DemoModel>("harbour.myapp", 1, 0, "DemoModel");
    // See: https://harbour.jolla.com/faq#1.5.0
    return SailfishApp::main(argc, argv);
}
