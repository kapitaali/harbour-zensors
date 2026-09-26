import QtQuick 2.6
// A library import must carry a version on Qt 5: without one the module is
// rejected with "Library import requires a version", the Loader above never
// gets an item and every location row reads "not started" for ever.
import QtPositioning 5.4

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
    property real lastBump: 0

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

    // A revision bump makes every row on the page re-read its value, and the
    // position can update many times a second. Publish at most once a second;
    // the rows also track their own properties directly, so no reading is lost.
    function bump() {
        var now = Date.now()
        if (now - probe.lastBump < 1000) return
        probe.lastBump = now
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
            // PositionSource.sourceName only exists on newer QtPositioning
            // than this device ships, where reading it gives undefined and
            // the row would sit at a dash forever. Say what is being asked
            // instead: the system service, which is what it always is here.
            probe.sourceName = positionSource.active ? "system location service" : ""
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
        var src = (positionSource.sourceName !== undefined
                   && positionSource.sourceName !== null)
                ? String(positionSource.sourceName) : ""
        probe.sourceName = src.length > 0 ? src : "system location service"
        probe.bump()
    }

    function numberOrNaN(v) {
        if (v === undefined || v === null) return NaN
        var n = Number(v)
        return isNaN(n) ? NaN : n
    }

    Component.onCompleted: {
        console.log("LocationProbe started")
        sync()
    }
}
