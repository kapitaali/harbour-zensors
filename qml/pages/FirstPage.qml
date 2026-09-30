import QtQuick 2.6
import Sailfish.Silica 1.0

/*
 * About: what the app is, where the numbers come from, and the device it is
 * running on. Section divisions and the support button follow sailotp's
 * About page.
 */
Page {
    id: page

    property var values: probe.values
    property string supportLink: "https://ko-fi.com/kapitaali"
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: page.width - 2 * Theme.horizontalPageMargin
            x: Theme.horizontalPageMargin
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("About")
                leftMargin: 0
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.primaryColor
                text: qsTr("Zensors gathers every measurement the operating system lets an app read into one list: performance, battery, storage, thermals, display, network, radio, Bluetooth, location, motion, microphone and the plain system facts.")
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Values come from /proc and /sys, from the system daemons over D-Bus and from the Qt sensor, positioning and audio APIs.")
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Updated every two seconds")
            }

            SectionHeader {
                text: qsTr("Support")
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("We love Open Source software and the Jolla ecosystem. If you want to support me or my work, please leave some tip here:")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Leave a tip")
                onClicked: Qt.openUrlExternally(page.supportLink)
            }

            SectionHeader {
                text: qsTr("System information")
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                // Plain text, not RichText: the HTML parser treats a lone \n
                // as whitespace, which collapsed every line into one paragraph.
                textFormat: Text.PlainText
                text: {
                    var lines = []
                    var device = values["sys.device"]
                    var os = values["sys.prettyName"]
                    var kernel = values["sys.kernel"]
                    if (device !== undefined) lines.push(qsTr("Device: %1").arg(device))
                    if (os !== undefined) lines.push(qsTr("System: %1").arg(os))
                    if (kernel !== undefined) lines.push(qsTr("Kernel: %1").arg(kernel))
                    return lines.join("\n")
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
