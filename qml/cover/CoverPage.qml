import QtQuick 2.6
import Sailfish.Silica 1.0

/*
 * Shown on the Home screen while the app is running in the background.
 * Cover actions are executable directly from Home without opening the app.
 */
CoverBackground {
    id: cover

    Label {
        id: coverLabel
        anchors {
            top: cover.top
            topMargin: Theme.paddingLarge
            horizontalCenter: parent.horizontalCenter
        }
        width: cover.width - 2 * Theme.paddingLarge
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        color: Theme.primaryColor
        text: qsTr("My App")
    }

    CoverActionList {
        id: coverActions

        CoverAction {
            iconSource: "image://theme/icon-cover-new"
            onTriggered: {
                // e.g. reset state or perform a quick action from Home
            }
        }
    }
}
