import QtQuick 2.6
import Sailfish.Silica 1.0
import "../backend/metrics.js" as Metrics

/*
 * Dashboard: every reading this device can actually give, grouped by
 * category and laid out as a dense grid of cells (two columns on the phone,
 * more in landscape). Each cell drills into the category's full page, where
 * the unavailable readings are listed as dashes.
 *
 * The model is built by metrics.js as plain strings and numbers: handing QML
 * a JS array of objects would convert the entries and drop anything that is
 * not a plain value.
 *
 * Delegate children address their geometry through ids rather than `parent`:
 * during construction the parent is still null and such bindings would fail
 * to evaluate. The model property is called `entry`, never `data` - that
 * name belongs to Item's own list property.
 */
Page {
    id: page

    property var values: probe.values
    property var cells: Metrics.dashboard(values, {})
    allowedOrientations: Orientation.All

    // label line + value line + padding + a sliver for the graph bar
    readonly property real cellHeight: Theme.fontSizeExtraSmall + Theme.fontSizeSmall
                                       + Theme.paddingMedium + 8

    // Extra room left under every cell. Flow gives every cell the same box,
    // the content sits top-aligned inside it, and Flow adds `spacing` between
    // lines, so the whitespace from one row's reading to the next is
    // (cellHeight - content) + spacing. Adding here adds the same amount to
    // every row: at 0 the rows sit ~9 px apart, too tight to read as
    // separate readings; at 9 they open out to ~18 px.
    readonly property real rowGap: 9

    // A cell that draws a graph runs 6 px of content lower than a plain one,
    // so the reading under it lands closer to the next row than it does
    // under a plain cell - about 18 px against 32 px. All the cells of a
    // line share that line's height, so raising just the graph cells lifts
    // the whole line and gives the row below 9 px more, ~18 px to ~27 px.
    readonly property real barRowGap: 9

    Component.onCompleted: {
        console.log("LAYOUT page", page.width, "x", page.height,
                    "flow", flow.width, "cols", flow.columns,
                    "margin", Theme.horizontalPageMargin,
                    "fontSizes", Theme.fontSizeExtraSmall, Theme.fontSizeSmall, Theme.fontSizeLarge)
        console.log("PROBEKEYS", Object.keys(probe.values).sort().join(","))
    }

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Refresh now")
                onClicked: probe.refresh()
            }
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("FirstPage.qml"))
            }
        }

        Column {
            id: column
            width: page.width

            PageHeader {
                title: qsTr("Zensors")
            }

            Flow {
                id: flow
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                spacing: Theme.paddingSmall

                // Text cells: at most three across, and never narrower than
                // ~180 px or the labels start truncating.
                readonly property int columns: Math.min(3, Math.max(1, Math.floor((width + spacing) / (180 + spacing))))
                // Cells that draw a graph need the extra room, so they only
                // ever share a row with one other cell.
                readonly property int graphColumns: Math.min(2, columns)
                readonly property real cellWidth: (width - spacing * (columns - 1)) / columns
                readonly property real graphCellWidth: (width - spacing * (graphColumns - 1)) / graphColumns

                Repeater {
                    model: page.cells.length

                    delegate: Item {
                        id: cell

                        property var entry: page.cells[index]

                        width: entry.kind === "section" ? flow.width
                               : (entry.graph ? flow.graphCellWidth : flow.cellWidth)
                        height: entry.kind === "section" ? header.implicitHeight + Theme.paddingSmall
                                                          : page.cellHeight + page.rowGap
                                                            + (entry.graph ? page.barRowGap : 0)

                        opacity: tap.pressed ? 0.6 : 1.0

                        MouseArea {
                            id: tap
                            x: 0
                            y: 0
                            width: cell.width
                            height: cell.height
                            onClicked: pageStack.push(Qt.resolvedUrl("CategoryPage.qml"),
                                                      { categoryIndex: cell.entry.index })
                        }

                        Label {
                            id: header
                            visible: cell.entry.kind === "section"
                            x: Theme.paddingSmall
                            y: 0
                            width: cell.width - 2 * Theme.paddingSmall
                            text: cell.entry.kind === "section"
                                  ? cell.entry.name
                                    + (cell.entry.count
                                       ? "  \u00b7  " + cell.entry.count
                                         + (cell.entry.count === 1 ? " reading" : " readings")
                                       : "")
                                  : ""
                            color: Metrics.categoryColor
                            font.pixelSize: Theme.fontSizeSmall
                            truncationMode: TruncationMode.Fade
                        }

                        Column {
                            id: info
                            visible: cell.entry.kind === "cell"
                            x: Theme.paddingSmall
                            y: Theme.paddingSmall
                            width: cell.width - 2 * Theme.paddingSmall
                            spacing: 2

                            Label {
                                width: info.width
                                text: cell.entry.kind === "cell" ? cell.entry.label : ""
                                color: Theme.secondaryColor
                                font.pixelSize: Theme.fontSizeExtraSmall
                                truncationMode: TruncationMode.Fade
                            }

                            Label {
                                width: info.width
                                text: cell.entry.kind === "cell" ? cell.entry.value : ""
                                color: cell.entry.dim ? Theme.rgba(Theme.secondaryColor, Theme.opacityLow)
                                                      : Theme.highlightColor
                                font.pixelSize: Theme.fontSizeSmall
                                truncationMode: TruncationMode.Fade
                            }

                            Item {
                                width: info.width
                                height: 4
                                visible: cell.entry.frac !== null && cell.entry.frac !== undefined

                                Rectangle {
                                    x: 0
                                    y: 0
                                    width: info.width
                                    height: 4
                                    color: Theme.rgba(Theme.primaryColor, 0.25)
                                }
                                Rectangle {
                                    x: 0
                                    y: 0
                                    width: info.width * (cell.entry.frac || 0)
                                    height: 4
                                    color: Theme.highlightColor
                                }
                            }
                        }
                    }
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                width: page.width - 2 * Theme.horizontalPageMargin
                height: contentHeight + Theme.paddingMedium
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("Only readings this device actually exposes are listed. Tap anything for the full page, including what is missing.")
            }
        }

        VerticalScrollDecorator {}
    }
}
