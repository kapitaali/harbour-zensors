import QtQuick 2.6
import Sailfish.Silica 1.0
import "../backend/metrics.js" as Metrics

/*
 * About: what the app is, where the numbers come from, and the device it is
 * running on.
 */
Page {
    id: page

    property var values: probe.values
    property string supportLink: "https://ko-fi.com/kapitaali"
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("About")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: page.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.primaryColor
                text: qsTr("Zensors gathers every measurement the operating system lets an app read into one list: performance, battery, storage, thermals, display, network, radio, Bluetooth, location, motion, microphone and the plain system facts.")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: page.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Values come from /proc and /sys, from the system daemons over D-Bus and from the Qt sensor, positioning and audio APIs. Probes the device does not have simply stay empty; nothing is invented.")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: page.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: {
                    var lines = []
                    var device = values["sys.device"]
                    var os = values["sys.prettyName"]
                    var kernel = values["sys.kernel"]
                    if (device !== undefined) lines.push(qsTr("Device: %1").arg(device))
                    if (os !== undefined) lines.push(qsTr("System: %1").arg(os))
                    if (kernel !== undefined) lines.push(qsTr("Kernel: %1").arg(kernel))
                    lines.push(qsTr("Updated every two seconds"))
                    lines.push(qsTr("We love Open Source software and the Jolla ecosystem. If you want to support me or my work, please leave some tip here: %1").arg(page.supportLink))
                    return lines.join("\n")
                }
                linkEnabled: true
                onLinkActivated: Qt.openUrlExternally(link)
            }

        VerticalScrollDecorator {}
    }
}
}
