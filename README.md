# harbour-zensors

Zensors gathers every measurement the operating system lets an app read into
one information-dense dashboard: performance, battery, storage, thermals,
display, network, cellular, Bluetooth, location, motion, audio and the plain
system facts. Each reading is one tap away from its full category page, which
also lists what the device does *not* expose.

Values come from `/proc` and `/sys`, from the system daemons over D-Bus, and
from the Qt sensor, positioning and audio APIs. A dash means the subsystem
answered with nothing -- never a missing reading, and nothing is invented.

The app runs entirely on your device. It has no accounts, makes no network
connections, sends no analytics, and stores nothing outside its own private
data.

## Installing

### From Harbour

Search for **Zensors** in the Jolla Store on your Sailfish device and install
it. (Harbour submission in progress.)

### From the RPM

Download the RPM for your architecture from the
[releases page](https://github.com/kapitaali/harbour-zensors/releases), copy it
to the device, and install it:

```bash
scp harbour-zensors-0.1-1.<arch>.rpm defaultuser@<phone-ip>:/tmp/
ssh defaultuser@<phone-ip> "devel-su -c 'rpm -Uvh /tmp/harbour-zensors-0.1-1.<arch>.rpm'"
```

`<arch>` is `aarch64` (most phones) or `armv7hl` (older 32-bit devices).

## Building

### With sfdk (CLI)

```sh
# Phone (aarch64)
sfdk build-shell make -o Makefile distclean
sfdk -c target=SailfishOS-<ver>-aarch64 build

# Phone (armv7hl)
sfdk build-shell make -o Makefile distclean
sfdk -c target=SailfishOS-<ver>-armv7hl build

# Emulator
sfdk -c target=SailfishOS-<ver>-i486 build
```

Replace `<ver>` with your installed SDK version (e.g. `5.1.0.11`). RPMs land in
`RPMS/SailfishOS-<ver>-<arch>/harbour-zensors-<ver>.<arch>.rpm`.

**Important:** switching targets requires `make distclean` first, or you will
package the previous target's binary.

### In the Sailfish IDE

`File > Open File or Project > harbour-zensors.pro`, pick a kit
(`SailfishOS-<ver>-i486` for emulator, `-armv7hl` / `-aarch64` for device),
then run.

## Project layout

| Path | Purpose |
|---|---|
| `harbour-zensors.pro` | qmake project; `CONFIG += sailfishapp` pulls standard install rules |
| `src/` | C++ backend: `SystemProbe` reads every meter and exposes `probe.values` |
| `qml/harbour-zensors.qml` | `ApplicationWindow`, page stack, cover |
| `qml/pages/` | Dashboard, category pages, About page |
| `qml/cover/` | Home screen cover + refresh action |
| `rpm/harbour-zensors.spec` | RPM packaging |
| `harbour-zensors.desktop` | Launcher entry incl. `[X-Sailjail]` sandbox permissions |
| `icons/` | App icons (86/108/128/172 px) |
| `translations/` | `qsTr()` translation sources (`.ts`) |
| `PRIVACY.md` | Privacy policy (no data collection, no network) |
| `LICENSE` | GPL-3.0 |

## Permissions

The app requests Sailjail permissions for the data it reads: `Location`,
`Sensors`, `Connman`, `Internet`, `Bluetooth`, `Audio`, `Microphone`. See
`harbour-zensors.desktop` → `[X-Sailjail]`.

## License

GPL-3.0. See [LICENSE](LICENSE).
