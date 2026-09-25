.pragma library

/*
 * Static definitions for every category and row the app can show.
 *
 * Row keys match what src/systemprobe.cpp publishes in probe.values.
 * Rows marked with `from:` are read from the QML probes instead
 * (LocationProbe.qml / SensorsProbe.qml).
 *
 * A row that has no value renders as "—" and is dimmed: that is the
 * graceful-degradation path for probes the current device does not have.
 */

var dash = "—"

function fmtText(v) {
    if (v === undefined || v === null) return dash
    if (typeof v === "boolean") return v ? "yes" : "no"
    var s = String(v)
    return s.length > 0 ? s : dash
}

function fmtPct(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Math.round(Number(v)) + " %"
}

function fmtPct1(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(1) + " %"
}

function fmtNum(v, digits) {
    if (v === undefined || v === null || isNaN(v)) return dash
    var d = (digits === undefined) ? 2 : digits
    return Number(v).toFixed(d)
}

function fmtBytes(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    var b = Number(v)
    if (b < 0) return dash
    var units = ["B", "kB", "MB", "GB", "TB"]
    var i = 0
    while (b >= 1024 && i < units.length - 1) { b /= 1024; i++ }
    var digits = b >= 100 ? 0 : (b >= 10 ? 1 : 2)
    return b.toFixed(digits) + " " + units[i]
}

function fmtUptime(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    var s = Math.floor(Number(v))
    var d = Math.floor(s / 86400); s -= d * 86400
    var h = Math.floor(s / 3600); s -= h * 3600
    var m = Math.floor(s / 60); s -= m * 60
    function p(n) { return (n < 10 ? "0" : "") + n }
    return (d > 0 ? d + "d " : "") + p(h) + ":" + p(m) + ":" + p(s)
}

function fmtC(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(1) + " °C"
}

function fmtV(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(3) + " V"
}

function fmtMa(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(0) + " mA"
}

function fmtW(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(2) + " W"
}

function fmtMWh(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(0) + " mWh"
}

function fmtKbs(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(1) + " kB/s"
}

function fmtMhz(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(0) + " MHz"
}

function fmtMeters(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(1) + " m"
}

function fmtMs(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(1) + " m/s"
}

function fmtDeg(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(0) + "°"
}

function fmtLat(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    var n = Number(v)
    return Math.abs(n).toFixed(6) + "° " + (n >= 0 ? "N" : "S")
}

function fmtLon(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    var n = Number(v)
    return Math.abs(n).toFixed(6) + "° " + (n >= 0 ? "E" : "W")
}

function fmtTime(v) {
    if (v === undefined || v === null) return dash
    var d = (v instanceof Date) ? v : new Date(v)
    if (isNaN(d.getTime())) return dash
    return d.toLocaleTimeString()
}

function fmtFix(v) {
    if (v === null || v === undefined) return dash
    return v ? "position fixed" : "no fix"
}

function fmtAvail(v) {
    if (v === null || v === undefined) return dash
    return v ? "available" : "not available"
}

function fmtOnOff(v) {
    if (v === null || v === undefined) return dash
    return v ? "on" : "off"
}

function fmtNearFar(v) {
    if (v === null || v === undefined) return dash
    return v ? "near" : "far"
}

function fmtMotion(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(3) + " m/s²"
}

function fmtRad(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(3) + " rad/s"
}

function fmtTesla(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    var n = Number(v) * 1000000
    return n.toFixed(1) + " µT"
}

function fmtLux(v) {
    if (v === undefined || v === null || isNaN(v)) return dash
    return Number(v).toFixed(0) + " lx"
}

function fmtRatio(v, vals, key, digits) {
    if (v === undefined || v === null || isNaN(v)) return dash
    var max = vals[key]
    if (max === undefined || max === null || isNaN(max) || max <= 0) return dash
    var d = (digits === undefined) ? 0 : digits
    return Number(v).toFixed(d) + " of " + Number(max).toFixed(d)
}

/*
 * categories[i] = {
 *   id, name,
 *   headline: [ {key, fmt}, ... ]   first entry with a value wins
 *   sub:      [ {key, fmt}, ... ]
 *   hint:     text shown instead of the headline when nothing is readable
 *   rows:     [ ... ]
 *   dynamic:  "cores" | "thermal" | undefined
 * }
 */
var categories = [
    {
        id: "performance",
        name: "Performance",
        headline: [ { key: "cpu.total", fmt: fmtPct1 } ],
        sub: [ { key: "mem.usedPct", fmt: function(v) { return "memory " + fmtPct(v) } } ],
        dynamic: "cores",
        rows: [
            { key: "cpu.total", label: "CPU total", fmt: fmtPct1, bar: true },
            { key: "load.1", label: "Load average, 1 min", fmt: fmtNum },
            { key: "load.5", label: "Load average, 5 min", fmt: fmtNum },
            { key: "load.15", label: "Load average, 15 min", fmt: fmtNum },
            { key: "uptime", label: "Uptime", fmt: fmtUptime },
            { key: "mem.usedPct", label: "Memory used", fmt: fmtPct1, bar: true },
            { key: "mem.used", label: "Memory in use", fmt: fmtBytes },
            { key: "mem.available", label: "Available for apps", fmt: fmtBytes },
            { key: "mem.total", label: "Physical memory", fmt: fmtBytes },
            { key: "mem.swapUsed", label: "Swap in use", fmt: fmtBytes },
            { key: "mem.swapTotal", label: "Swap total", fmt: fmtBytes }
        ]
    },
    {
        id: "power",
        name: "Battery",
        headline: [ { key: "bat.capacity", fmt: fmtPct } ],
        sub: [ { key: "bat.status", fmt: fmtText } ],
        rows: [
            { key: "bat.capacity", label: "Charge level", fmt: fmtPct, bar: true },
            { key: "bat.status", label: "Status", fmt: fmtText },
            { key: "bat.health", label: "Health against design", fmt: fmtPct, bar: true },
            { key: "bat.energyNow", label: "Energy stored", fmt: fmtMWh },
            { key: "bat.voltage", label: "Voltage", fmt: fmtV },
            { key: "bat.current", label: "Current", fmt: fmtMa },
            { key: "bat.power", label: "Power draw", fmt: fmtW },
            { key: "bat.temp", label: "Temperature", fmt: fmtC },
            { key: "bat.cycles", label: "Charge cycles", fmt: function(v) { return fmtNum(v, 0) } },
            { key: "bat.technology", label: "Technology", fmt: fmtText },
            { key: "bat.charger", label: "External power", fmt: fmtOnOff },
            { key: "bat.chargerType", label: "Power supply type", fmt: fmtText }
        ]
    },
    {
        id: "storage",
        name: "Storage",
        headline: [ { key: "disk.root.available", fmt: fmtBytes } ],
        sub: [ { key: "disk.root.total", fmt: function(v) { return "of " + fmtBytes(v) } } ],
        rows: [
            { key: "disk.root.available", label: "Free for apps", fmt: fmtBytes },
            { key: "disk.root.free", label: "Unallocated", fmt: fmtBytes },
            { key: "disk.root.total", label: "Root filesystem", fmt: fmtBytes },
            { key: "disk.home.available", label: "Home, free", fmt: fmtBytes },
            { key: "disk.home.total", label: "Home, size", fmt: fmtBytes },
            { key: "disk.io.readKbs", label: "Read rate", fmt: fmtKbs },
            { key: "disk.io.writeKbs", label: "Write rate", fmt: fmtKbs }
        ]
    },
    {
        id: "thermal",
        name: "Thermal",
        headline: [ { key: "thermal.z0.temp", fmt: fmtC } ],
        sub: [ { key: "thermal.zoneCount", fmt: function(v) { return fmtNum(v, 0) + " zones" } } ],
        dynamic: "thermal",
        rows: [
            { key: "thermal.zoneCount", label: "Thermal zones", fmt: function(v) { return fmtNum(v, 0) } }
        ]
    },
    {
        id: "display",
        name: "Display",
        headline: [ { key: "disp.state", fmt: fmtText } ],
        sub: [ { key: "disp.brightness", fmt: function(v, vals) {
            if (v === undefined || v === null) return dash
            var max = vals["disp.maxBrightness"]
            if (!max) return fmtNum(v, 0)
            return "brightness " + fmtPct(100 * Number(v) / Number(max))
        } } ],
        rows: [
            { key: "disp.state", label: "State", fmt: fmtText },
            { key: "disp.brightness", label: "Backlight", fmt: function(v, vals) {
                var max = vals["disp.maxBrightness"]
                if (max === undefined || !max) return fmtNum(v, 0)
                return fmtPct(100 * Number(v) / Number(max))
            } },
            { key: "disp.maxBrightness", label: "Backlight maximum", fmt: function(v) { return fmtNum(v, 0) } }
        ]
    },
    {
        id: "network",
        name: "Network",
        headline: [ { key: "wifi.strength", fmt: fmtPct },
                    { key: "net.eth.state", fmt: fmtText } ],
        sub: [ { key: "net.eth.state", fmt: function(v) { return "ethernet " + fmtText(v) } },
               { key: "net.techCount", fmt: function(v) { return fmtNum(v, 0) + " radio technologies" } } ],
        rows: [
            { key: "net.techCount", label: "Radio technologies", fmt: function(v) { return fmtNum(v, 0) } },
            { key: "net.eth.state", label: "Ethernet", fmt: fmtText },
            { key: "net.wifi.state", label: "WLAN", fmt: fmtText },
            { key: "net.cell.state", label: "Cellular data", fmt: fmtText },
            { key: "net.gps.power", label: "GPS powered", fmt: fmtOnOff },
            { key: "net.wifi.detail", label: "WLAN state", fmt: fmtText },
            { key: "wifi.strength", label: "Signal strength", fmt: fmtPct, bar: true },
            { key: "wifi.link", label: "Link quality", fmt: fmtNum },
            { key: "net.eth.detail", label: "Ethernet state", fmt: fmtText },
            { key: "net.ifCount", label: "Interfaces visible", fmt: function(v) { return fmtNum(v, 0) } },
            { key: "net.rxKbs", label: "Download rate", fmt: fmtKbs },
            { key: "net.txKbs", label: "Upload rate", fmt: fmtKbs }
        ]
    },
    {
        id: "cellular",
        name: "Cellular",
        headline: [ { key: "cell.strength", fmt: fmtPct },
                    { key: "cell.operator", fmt: fmtText } ],
        sub: [ { key: "cell.tech", fmt: fmtText },
               { key: "cell.present", fmt: function(v) { return v ? "modem detected" : "no modem" } } ],
        rows: [
            { key: "cell.present", label: "Modem", fmt: function(v) { return v ? "detected" : "none" } },
            { key: "cell.operator", label: "Operator", fmt: fmtText },
            { key: "cell.tech", label: "Technology", fmt: fmtText },
            { key: "cell.strength", label: "Signal strength", fmt: fmtPct, bar: true },
            { key: "cell.roaming", label: "Roaming", fmt: fmtOnOff },
            { key: "cell.mccmnc", label: "Network code", fmt: fmtText },
            { key: "cell.location", label: "Cell and area", fmt: fmtText },
            { key: "cell.sim", label: "SIM present", fmt: fmtOnOff },
            { key: "cell.simState", label: "SIM PIN state", fmt: fmtText },
            { key: "cell.modem", label: "Modem object", fmt: fmtText }
        ]
    },
    {
        id: "bluetooth",
        name: "Bluetooth",
        headline: [ { key: "bt.powered", fmt: fmtOnOff },
                    { key: "bt.present", fmt: function(v) { return v ? "running" : "absent" } } ],
        sub: [ { key: "bt.address", fmt: fmtText },
               { key: "bt.discovering", fmt: function(v) { return v ? "scanning" : "not scanning" } } ],
        rows: [
            { key: "bt.present", label: "Bluetooth daemon", fmt: function(v) { return v ? "running" : "not running" } },
            { key: "bt.powered", label: "Radio", fmt: fmtOnOff },
            { key: "bt.discovering", label: "Scanning for devices", fmt: fmtOnOff },
            { key: "bt.alias", label: "Adapter name", fmt: fmtText },
            { key: "bt.address", label: "Adapter address", fmt: fmtText }
        ]
    },
    {
        id: "location",
        name: "Location",
        headline: [],
        hint: "tap to locate",
        sub: [],
        rows: [
            { from: "location", prop: "valid", label: "Position", fmt: fmtFix },
            { from: "location", prop: "latitude", label: "Latitude", fmt: fmtLat },
            { from: "location", prop: "longitude", label: "Longitude", fmt: fmtLon },
            { from: "location", prop: "altitude", label: "Altitude", fmt: fmtMeters },
            { from: "location", prop: "haccuracy", label: "Horizontal accuracy", fmt: fmtMeters },
            { from: "location", prop: "vaccuracy", label: "Vertical accuracy", fmt: fmtMeters },
            { from: "location", prop: "speed", label: "Ground speed", fmt: fmtMs },
            { from: "location", prop: "course", label: "Course", fmt: fmtDeg },
            { from: "location", prop: "timestamp", label: "Last update", fmt: fmtTime },
            { from: "location", prop: "sourceName", label: "Position source", fmt: fmtText }
        ]
    },
    {
        id: "sensors",
        name: "Sensors",
        headline: [],
        hint: "tap to read",
        sub: [],
        rows: [
            { from: "sensors", prop: "accelAvailable", label: "Accelerometer", fmt: fmtAvail },
            { from: "sensors", prop: "ax", label: "Acceleration X", fmt: fmtMotion },
            { from: "sensors", prop: "ay", label: "Acceleration Y", fmt: fmtMotion },
            { from: "sensors", prop: "az", label: "Acceleration Z", fmt: fmtMotion },
            { from: "sensors", prop: "gyroAvailable", label: "Gyroscope", fmt: fmtAvail },
            { from: "sensors", prop: "gx", label: "Angular velocity X", fmt: fmtRad },
            { from: "sensors", prop: "gy", label: "Angular velocity Y", fmt: fmtRad },
            { from: "sensors", prop: "gz", label: "Angular velocity Z", fmt: fmtRad },
            { from: "sensors", prop: "magAvailable", label: "Magnetometer", fmt: fmtAvail },
            { from: "sensors", prop: "mx", label: "Magnetic field X", fmt: fmtTesla },
            { from: "sensors", prop: "my", label: "Magnetic field Y", fmt: fmtTesla },
            { from: "sensors", prop: "mz", label: "Magnetic field Z", fmt: fmtTesla },
            { from: "sensors", prop: "lightAvailable", label: "Ambient light sensor", fmt: fmtAvail },
            { from: "sensors", prop: "light", label: "Ambient light", fmt: fmtLux },
            { from: "sensors", prop: "proxAvailable", label: "Proximity sensor", fmt: fmtAvail },
            { from: "sensors", prop: "proxClose", label: "Near or far", fmt: fmtNearFar }
        ]
    },
    {
        id: "audio",
        name: "Audio",
        // The microphone only runs while the Audio page is open, so the
        // dashboard has nothing to show for it - say that instead of
        // claiming the device exposes nothing.
        hint: "tap to listen",
        headline: [ { key: "mic.level", fmt: function(v) { return fmtPct(100 * Number(v)) } } ],
        sub: [ { key: "mic.active", fmt: function(v) { return v ? "microphone live" : "microphone stopped" } } ],
        rows: [
            { key: "mic.supported", label: "Input device", fmt: fmtAvail },
            { key: "mic.active", label: "Microphone", fmt: function(v) { return v ? "listening" : "stopped" } },
            { key: "mic.level", label: "Level (rms)", fmt: function(v) { return fmtPct(100 * Number(v)) }, bar: true, barMax: 1 },
            { key: "mic.peak", label: "Peak", fmt: function(v) { return fmtPct(100 * Number(v)) }, bar: true, barMax: 1 }
        ]
    },
    {
        id: "system",
        name: "System",
        headline: [ { key: "sys.device", fmt: fmtText },
                    { key: "sys.os", fmt: fmtText } ],
        sub: [ { key: "sys.version", fmt: fmtText },
               { key: "sys.prettyName", fmt: fmtText } ],
        rows: [
            { key: "sys.device", label: "Device", fmt: fmtText },
            { key: "sys.os", label: "Operating system", fmt: fmtText },
            { key: "sys.version", label: "Version", fmt: fmtText },
            { key: "sys.prettyName", label: "Full name", fmt: fmtText },
            { key: "sys.kernel", label: "Kernel", fmt: fmtText },
            { key: "sys.hostname", label: "Host name", fmt: fmtText },
            { key: "sys.bootId", label: "Boot id", fmt: fmtText },
            { key: "uptime", label: "Uptime", fmt: fmtUptime }
        ]
    }
]

function categoryById(id) {
    for (var i = 0; i < categories.length; i++) {
        if (categories[i].id === id) return categories[i]
    }
    return null
}

function firstValue(list, values) {
    if (!list) return null
    for (var i = 0; i < list.length; i++) {
        var v = values[list[i].key]
        if (v !== undefined && v !== null && v !== "") {
            if (typeof v === "number" && isNaN(v)) continue
            return { key: list[i].key, value: v, fmt: list[i].fmt }
        }
    }
    return null
}

function headlineOf(cat, values) {
    var hit = firstValue(cat.headline, values)
    if (!hit) return { text: cat.hint ? cat.hint : dash, ok: false }
    return { text: hit.fmt ? hit.fmt(hit.value, values) : fmtText(hit.value), ok: true }
}

function subOf(cat, values) {
    var hit = firstValue(cat.sub, values)
    if (!hit) return ""
    return hit.fmt ? hit.fmt(hit.value, values) : fmtText(hit.value)
}

function buildRows(cat, values) {
    var rows = cat.rows.slice()

    if (cat.dynamic === "cores") {
        var n = Number(values["cpu.coreCount"])
        if (!isNaN(n)) {
            for (var i = 0; i < n; i++) {
                rows.splice(1 + i, 0, {
                    key: "cpu.c" + i,
                    label: "Core " + (i + 1),
                    fmt: fmtPct1,
                    bar: true
                })
            }
        }
    } else if (cat.dynamic === "thermal") {
        var zones = Number(values["thermal.zoneCount"])
        if (!isNaN(zones)) {
            for (var z = 0; z < zones && z < 8; z++) {
                rows.push({
                    key: "thermal.z" + z + ".temp",
                    label: "Zone " + (z + 1),
                    subKey: "thermal.z" + z + ".type",
                    fmt: fmtC,
                    bar: false
                })
            }
        }
        var cool = Number(values["thermal.coolCount"])
        if (!isNaN(cool)) {
            for (var c = 0; c < cool && c < 8; c++) {
                (function(index) {
                    rows.push({
                        key: "thermal.c" + index + ".cur",
                        label: "Policy " + (index + 1),
                        subKey: "thermal.c" + index + ".name",
                        fmt: function(v, vals) {
                            return fmtRatio(v, vals, "thermal.c" + index + ".max", 0)
                        },
                        bar: false
                    })
                })(c)
            }
        }
    }

    return rows
}

function filledCount(rows, values, probes) {
    var n = 0
    for (var i = 0; i < rows.length; i++) {
        if (valueFor(rows[i], values, probes) !== undefined) n++
    }
    return n
}

function valueFor(row, values, probes) {
    if (row.from === "location" && probes && probes.location) {
        var loc = probes.location[row.prop]
        return (loc === undefined) ? null : loc
    }
    if (row.from === "sensors" && probes && probes.sensors) {
        var s = probes.sensors[row.prop]
        return (s === undefined) ? null : s
    }
    if (row.from) return null
    return values[row.key]
}

function rowLabel(row, values) {
    var label = row.label
    if (row.subKey) {
        var t = values[row.subKey]
        if (t !== undefined && t !== null && t !== "")
            label = label + " \u00b7 " + String(t)
    }
    return label
}

// 0..1 for the cell bar, or null when the row has no meaningful bar.
function barFraction(row, values, probes) {
    if (!row.bar) return null
    var v = valueFor(row, values, probes)
    if (typeof v !== "number" || isNaN(v)) return null
    var max = (row.barMax === undefined) ? 100 : row.barMax
    if (!(max > 0)) return null
    var f = v / max
    if (f < 0) f = 0
    if (f > 1) f = 1
    return f
}

/*
 * The dashboard model: a flat list of section headers and cells carrying
 * only plain data (strings, numbers, null) - no function references, which
 * would not survive the trip through QML's model conversion.
 *
 * Every row with a reading becomes a cell, so the screen carries as much
 * live data as the device can provide rather than one headline per category.
 * A category with nothing readable contributes a single dimmed cell, so the
 * category is still discoverable.
 */
function dashboard(values, probes) {
    var out = []
    for (var i = 0; i < categories.length; i++) {
        var cat = categories[i]
        var all = buildRows(cat, values)
        var live = []
        for (var r = 0; r < all.length; r++) {
            var v = valueFor(all[r], values, probes)
            if (v === undefined || v === null || v === "") continue
            if (typeof v === "number" && isNaN(v)) continue
            // A reading can exist and still have nothing to say (a cooling
            // policy with no max state formats as an em dash); a cell that
            // only repeats its own label is noise, so drop it here and let
            // the section count match what is on screen.
            var text = all[r].fmt ? all[r].fmt(v, values) : fmtText(v)
            if (text === dash) continue
            live.push({ row: all[r], text: text })
        }

        out.push({ kind: "section", index: i, name: cat.name, count: live.length })

        if (live.length === 0) {
            out.push({
                kind: "cell", index: i, label: cat.name,
                value: cat.hint ? cat.hint : "not exposed here",
                frac: null, dim: true
            })
            continue
        }

        for (var k = 0; k < live.length; k++) {
            var row = live[k].row
            var frac = barFraction(row, values, probes)
            out.push({
                kind: "cell", index: i,
                label: rowLabel(row, values),
                value: live[k].text,
                frac: frac,
                // cells that draw something get the wider two-column slot
                graph: frac !== null && frac !== undefined,
                dim: false
            })
        }
    }
    return out
}
