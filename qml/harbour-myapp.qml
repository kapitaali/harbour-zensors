import QtQuick 2.6
import Sailfish.Silica 1.0
import "pages"

/*
 * Top-level QML file. The name must match TARGET in harbour-myapp.pro
 * because SailfishApp::main() resolves it automatically.
 */
ApplicationWindow {
    initialPage: Component {
        FirstPage {}
    }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations
}
