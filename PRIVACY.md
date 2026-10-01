# Privacy Policy — Zensors

*Last updated: 1 October 2026*

Zensors is an offline device-dashboard app for Sailfish OS, distributed
through the Jolla Store and from this repository. This policy covers
the app itself — the store, your device and the websites you visit have
their own policies.

## In short

The app has **no network access, no servers, no accounts, no analytics
and no tracking** — and it **stores nothing at all**. It reads your
device's own sensors to show you readings on your screen, and that is
where they stay.

## What the app reads

Zensors is a dashboard: it gathers measurements the operating system
exposes to apps — performance, battery, storage, thermals, display,
network, cellular, Bluetooth, location, motion, audio and plain system
facts — and displays them. Values come from `/proc` and `/sys`, from
system services over the local D-Bus, and from the Qt sensor,
positioning and audio APIs.

Everything it reads it reads **to show you, on your device**. Readings
exist only as pixels on the screen while the app is open.

## What the app stores

**Nothing.** There is no database, no settings file, no log file and
no cache: the app makes no writes to your device's storage. Close the
app and no trace of your measurements remains — there is no history to
find, because none is kept.

## What the app does not do

- **No network connections.** The sandbox grants the Sailjail
  `Internet` permission, which the app uses to *read* network
  interface statistics (such as signal strength from
  `/proc/net/wireless`) — it opens no sockets and sends no traffic.
- **No analytics, telemetry, crash reporting, advertising or
  tracking**, in any form.
- **No third-party SDKs, libraries or services.**
- **No accounts, no login, no cloud.** The developer receives no data
  from you — there is no channel by which the developer could receive
  it.
- **No location tracking.** The position is read only while the
  Location page is open; the probe is torn down when you leave the
  page, so no satellite is tracked in the background. The fix is
  displayed and never stored.
- **No microphone capture beyond the level meter.** When you switch the
  mic meter on, incoming audio is reduced to two numbers — an RMS level
  and a peak — and the samples themselves are discarded immediately.
  Nothing is recorded, stored or transmitted.
- The only outbound action in the app is opening a web link (the
  donation page on the About page) in your browser when you tap it.
  That is a standard browser hand-off; the app itself transmits
  nothing.

## Permissions and why they exist

| Permission | Used for |
|---|---|
| `Location` | The Location page: latitude, longitude, altitude, accuracy, speed — displayed only, while that page is open. |
| `Internet` | Reading network interface statistics (`/proc/net/wireless` signal strength and friends). No connections are opened. |
| `Bluetooth` | The Bluetooth category: adapter and device information from the system Bluetooth service. |
| `Audio` | The Audio category: which audio devices and formats exist. |
| `Microphone` | The optional live level meter (RMS/peak), only while you enable it. |
| `NFC` | Granted by the application profile; the current code does not use it. Listed here for completeness. |

## Your rights

Since the app holds no data — it stores nothing and transmits nothing —
there is no stored information about you to access, correct, port or
delete, on the device or anywhere else. What it shows you is your
device's own current state, under your control: close the page, close
the app, and the readings are gone.

## Children

The app is not directed at children and collects no data from anyone,
children included — it collects no data at all.

## Changes to this policy

This policy lives in the project repository
(<https://github.com/kapitaali/harbour-zensors>) and changes are
committed alongside the code; the date at the top reflects the current
version.

## Contact

Questions? Open an issue at
<https://github.com/kapitaali/harbour-zensors/issues>.
