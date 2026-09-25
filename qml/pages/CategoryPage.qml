import QtQuick 2.6
import Sailfish.Silica 1.0
import "../backend/metrics.js" as Metrics

/*
 * Generic detail page: renders whatever rows metrics.js defines for the
 * chosen category. Location and sensor rows are served by the two probe
 * items, which are only instantiated while this page is open.
 */
Page {
    id: page

    property int categoryIndex: 0
    property var category: Metrics.categories[categoryIndex]
    property var values: probe.values
    property var rows: Metrics.buildRows(category, values)

    // Re-read whenever either probe reloads, so rows bound through
    // valueOf() still repaint on sensor data.
    property int probeRevision: (locationLoader.item ? locationLoader.item.revision : 0)
            + (sensorLoader.item ? sensorLoader.item.revision : 0)
    property var probes: ({ location: locationLoader.item, sensors: sensorLoader.item })
    property int filled: Metrics.filledCount(rows, values, probes)

    allowedOrientations: Orientation.All

    function valueOf(row) {
        var _ = probeRevision
        return Metrics.valueFor(row, values, probes)
    }

    Loader {
        id: locationLoader
        active: category.id === "location"
        source: Qt.resolvedUrl("LocationProbe.qml")
    }

    Loader {
        id: sensorLoader
        active: category.id === "sensors"
        source: Qt.resolvedUrl("SensorsProbe.qml")
    }

    Component.onCompleted: {
        if (category.id === "audio")
            probe.setMicActive(true)
    }

    Component.onDestruction: {
        if (category.id === "audio")
            probe.setMicActive(false)
    }

    SilicaListView {
        id: list
        anchors.fill: parent
        model: page.rows.length

        header: Column {
            width: page.width

            PageHeader {
                title: page.category.name
                titleColor: Metrics.categoryColor
            }

            Label {
                x: Theme.horizontalPageMargin
                width: page.width - 2 * Theme.horizontalPageMargin
                visible: page.filled === 0
                wrapMode: Text.Wrap
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("None of these readings is available on this device.")
            }
        }

        PullDownMenu {
            MenuItem {
                visible: page.category.id === "audio"
                text: probe.micActive ? qsTr("Stop microphone")
                                      : qsTr("Start microphone")
                onClicked: probe.setMicActive(!probe.micActive)
            }
            MenuItem {
                text: qsTr("Refresh now")
                onClicked: probe.refresh()
            }
        }

        delegate: BackgroundItem {
            id: delegate

            property var row: page.rows[index]
            property var raw: page.valueOf(row)
            property bool hasValue: raw !== undefined && raw !== null
                    && !(typeof raw === "number" && isNaN(raw))
            property real barFraction: {
                if (!row.bar || !hasValue) return 0
                var n = Number(raw)
                if (isNaN(n)) return 0
                var max = row.barMax ? row.barMax : 100
                return Math.max(0, Math.min(1, n / max))
            }
            property string subText: row.subKey ? Metrics.fmtText(page.values[row.subKey]) : ""
            property string valueText: hasValue
                    ? (row.fmt ? row.fmt(raw, page.values) : Metrics.fmtText(raw))
                    : Metrics.dash

            height: content.height + Theme.paddingSmall

            Column {
                id: content
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    margins: Theme.horizontalPageMargin
                }
                spacing: 2

                Row {
                    width: parent.width
                    spacing: Theme.paddingLarge

                    Label {
                        width: parent.width - value.width - parent.spacing
                        text: delegate.row.label
                        color: delegate.hasValue ? Theme.primaryColor : Theme.secondaryColor
                        truncationMode: TruncationMode.Fade
                    }

                    Label {
                        id: value
                        text: delegate.valueText
                        color: delegate.hasValue ? Theme.highlightColor
                                                 : Theme.rgba(Theme.secondaryColor, Theme.opacityLow)
                        horizontalAlignment: Text.AlignRight
                    }
                }

                Label {
                    width: parent.width
                    visible: text.length > 0 && text !== Metrics.dash
                    text: delegate.subText
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                    truncationMode: TruncationMode.Fade
                }

                Item {
                    width: parent.width
                    height: delegate.barFraction > 0 ? Theme.paddingSmall : 0

                    Rectangle {
                        anchors {
                            left: parent.left
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                        }
                        height: 3
                        radius: 1.5
                        color: Theme.primaryColor
                        opacity: 0.3
                    }

                    Rectangle {
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        width: parent.width * delegate.barFraction
                        height: 3
                        radius: 1.5
                        color: Theme.highlightColor
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
