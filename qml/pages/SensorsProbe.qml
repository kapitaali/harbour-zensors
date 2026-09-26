import QtQuick 2.6
// Version required, same as LocationProbe: a versionless library import is
// rejected on Qt 5 and this file would never load at all.
import QtSensors 5.6

/*
 * Reads the on-board motion and environment sensors.
 * Only instantiated (via a Loader) while the Sensors page is open.
 *
 * A sensor counts as "available" once it has delivered a reading, which
 * works whether or not the device has one - no backend introspection
 * needed, and the emulator simply stays at "not available".
 */
Item {
    id: probe

    visible: false
    property bool active: true
    property int revision: 0
    property real lastBump: 0

    property real ax: NaN
    property real ay: NaN
    property real az: NaN
    property real gx: NaN
    property real gy: NaN
    property real gz: NaN
    property real mx: NaN
    property real my: NaN
    property real mz: NaN
    property real light: NaN
    property var proxClose: null

    property bool accelAvailable: !isNaN(probe.ax)
    property bool gyroAvailable: !isNaN(probe.gx)
    property bool magAvailable: !isNaN(probe.mx)
    property bool lightAvailable: !isNaN(probe.light)
    property bool proxAvailable: probe.proxClose !== null
    property int count: (accelAvailable ? 1 : 0) + (gyroAvailable ? 1 : 0)
            + (magAvailable ? 1 : 0) + (lightAvailable ? 1 : 0)
            + (proxAvailable ? 1 : 0)

    // Same as LocationProbe: the motion sensors deliver readings many times a
    // second, and every bump makes all ten rows on the page re-read their
    // value. Publish at most once a second; the rows track their own
    // properties directly, so no reading is lost.
    function bump() {
        var now = Date.now()
        if (now - probe.lastBump < 1000) return
        probe.lastBump = now
        probe.revision++
    }

    Accelerometer {
        id: accel
        active: probe.active
        onReadingChanged: {
            if (accel.reading) {
                probe.ax = Number(accel.reading.x)
                probe.ay = Number(accel.reading.y)
                probe.az = Number(accel.reading.z)
                probe.bump()
            }
        }
    }

    Gyroscope {
        id: gyro
        active: probe.active
        onReadingChanged: {
            if (gyro.reading) {
                probe.gx = Number(gyro.reading.x)
                probe.gy = Number(gyro.reading.y)
                probe.gz = Number(gyro.reading.z)
                probe.bump()
            }
        }
    }

    Magnetometer {
        id: mag
        active: probe.active
        onReadingChanged: {
            if (mag.reading) {
                probe.mx = Number(mag.reading.x)
                probe.my = Number(mag.reading.y)
                probe.mz = Number(mag.reading.z)
                probe.bump()
            }
        }
    }

    AmbientLightSensor {
        id: light
        active: probe.active
        onReadingChanged: {
            if (light.reading) {
                probe.light = Number(light.reading.lightLevel)
                probe.bump()
            }
        }
    }

    ProximitySensor {
        id: prox
        active: probe.active
        onReadingChanged: {
            if (prox.reading) {
                probe.proxClose = Boolean(prox.reading.close)
                probe.bump()
            }
        }
    }

    Component.onCompleted: console.log("SensorsProbe started")
}
