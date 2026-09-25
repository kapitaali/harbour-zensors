import QtQuick 2.6
import Sailfish.Silica 1.0
import "../backend/metrics.js" as Metrics

/*
 * Cover: the three headline numbers, plus a refresh action.
 *
 * The Repeater model is a count, and each entry is read back from the JS
 * array: handing QML a JS array of objects would convert the entries to
 * QVariantMap and silently drop the formatter functions.
 */
CoverBackground {
    id: cover

    property var values: probe.values

    property var entries: [
        { label: qsTr("CPU"), key: "cpu.total", fmt: Metrics.fmtPct1 },
        { label: qsTr("Memory"), key: "mem.usedPct", fmt: Metrics.fmtPct },
        { label: qsTr("Battery"), key: "bat.capacity", fmt: Metrics.fmtPct }
    ]

    Column {
        id: column
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Theme.paddingLarge
        }
        spacing: Theme.paddingMedium

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Sensors")
            color: Theme.primaryColor
            font.pixelSize: Theme.fontSizeMedium
        }

        Repeater {
            model: cover.entries.length

            delegate: Row {
                property var entry: cover.entries[index]

                width: column.width
                spacing: Theme.paddingLarge

                Label {
                    width: parent.width * 0.45
                    text: entry.label
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                }

                Label {
                    width: parent.width - parent.children[0].width - parent.spacing
                    text: {
                        var v = cover.values[entry.key]
                        if (v === undefined || v === null || isNaN(v))
                            return Metrics.dash
                        return entry.fmt(v, cover.values)
                    }
                    color: Theme.highlightColor
                    horizontalAlignment: Text.AlignRight
                    font.pixelSize: Theme.fontSizeMedium
                }
            }
        }

        Label {
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            text: Metrics.fmtUptime(cover.values["uptime"])
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeSmall
            horizontalAlignment: Text.AlignHCenter
        }
    }

    CoverActionList {
        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
            onTriggered: probe.refresh()
        }
    }
}
