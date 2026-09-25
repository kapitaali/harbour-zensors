import QtQuick 2.6
import Sailfish.Silica 1.0
import "pages"

/*
 * Top-level QML file. The name must match TARGET in harbour-zensors.pro
 * because SailfishApp::pathTo() resolves it from there.
 */
ApplicationWindow {
    initialPage: Component {
        MainPage {}
    }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
