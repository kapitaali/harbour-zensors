import QtQuick 2.6
import QtPositioning

/*
 * Reads the position through the system location service.
 * Only instantiated (via a Loader) while the Location page is open, so no
 * satellite is tracked behind the user's back.
 *
 * Everything is NaN/null until a fix exists: the row formatter renders that
 * as "—".
 */
Item {
    id: probe

    visible: false
    property bool active: true
    property int revision: 0

    property bool valid: false
    property real latitude: NaN
    property real longitude: NaN
    property real altitude: NaN
    property real haccuracy: NaN
    property real vaccuracy: NaN
    property real speed: NaN
    property real course: NaN
    property var timestamp: null
    property string sourceName: ""

    PositionSource {
        id: positionSource
        active: probe.active
        onValidChanged: probe.sync()
        onPositionChanged: probe.sync()
    }

    function bump() {
        probe.revision++
    }

    function sync() {
        var p = positionSource.position
        var fixed = positionSource.valid && p !== undefined && p !== null
                && p.coordinate !== undefined
        probe.valid = fixed
        if (!fixed) {
            probe.latitude = NaN
            probe.longitude = NaN
            probe.altitude = NaN
            probe.haccuracy = NaN
            probe.vaccuracy = NaN
            probe.speed = NaN
            probe.course = NaN
            probe.timestamp = null
            probe.sourceName = ""
            probe.bump()
            return
        }

        probe.latitude = Number(p.coordinate.latitude)
        probe.longitude = Number(p.coordinate.longitude)
        probe.altitude = Number(p.coordinate.altitude)
        probe.haccuracy = numberOrNaN(p.horizontalAccuracy)
        probe.vaccuracy = numberOrNaN(p.verticalAccuracy)
        probe.speed = numberOrNaN(p.speed)
        probe.course = numberOrNaN(p.course)
        probe.timestamp = (p.timestamp !== undefined && p.timestamp !== null)
                ? new Date(p.timestamp) : null
        probe.sourceName = positionSource.sourceName !== undefined
                ? String(positionSource.sourceName) : ""
        probe.bump()
    }

    function numberOrNaN(v) {
        if (v === undefined || v === null) return NaN
        var n = Number(v)
        return isNaN(n) ? NaN : n
    }

    Component.onCompleted: sync()
}
