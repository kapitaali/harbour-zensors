import QtQuick 2.6
import Sailfish.Silica 1.0

Page {
    id: page2
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        PushUpMenu {
            MenuItem {
                text: qsTr("Back to Page 1")
                onClicked: pageStack.pop()
            }
        }

        Column {
            id: column
            width: page2.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("Second Page")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: page2.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                text: qsTr("A second page demonstrating the page stack.\n\n" +
                           "Flick up from the bottom edge for the push-up menu.")
            }
        }
    }
}
