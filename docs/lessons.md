# Lessons learned

Everything that cost time or surprised us during the development and Harbour
preparation of `harbour-zensors`, so it does not have to be learned twice.

---

## 1. Never bind a QML view to a high-frequency live source

This was the single most expensive bug. `MainPage.cells` was a **binding** that
read every live sensor property via `Metrics.dashboard()`. On the phone the five
motion sensors deliver readings at 50-100+ Hz, so the entire ~100-delegate grid
was rebuilt hundreds of times per second. The main thread pegged at 100% CPU
(`state=R`, `wchan=0`, utime climbing) and never serviced input or the
compositor, producing the Sailfish "not responding" dialog and a blank window.

The emulator never reproduced it: with no sensors it stays idle, so the storm
never starts.

**Rule:** drive live views from a slow timer (2 s, matching `probe.values`),
and throttle the source's change signal (`bump()` to ~1 s). A binding is the
wrong tool for anything that changes faster than the eye can see.

---

## 2. An empty stderr redirect says nothing about whether QML ran

Qt/QML `console.log` and C++ `qDebug` both go to **journald** on Sailfish, not to
redirected stderr. We nearly concluded "the QML engine never started" from an
empty `/tmp/app.log` -- that was wrong. The non-Qt writers (`ld.so`'s MEMPROF,
firejail) *do* reach stderr, which made the redirect look alive while telling
us nothing about QML.

**Rule:** to read app logs on the phone, read the journal (`devel-su -c
'journalctl ...'`), or force console output headless with `QT_LOGGING_TO_CONSOLE=1`.

---

## 3. A busy-spinning thread is not a blocked thread

`/proc/PID/wchan` + `/proc/PID/stat` utime deltas distinguish them:

| Symptom | Meaning |
|---|---|
| `state=S`, utime flat | blocked in a syscall (idle) |
| `state=R`, `wchan=0`, utime climbing | busy-spinning on a core |
| utime ~0, load ~0 | healthy idle event loop |

The ANR was the second row: the main thread was computing, not waiting. That is
what separates "UI thread is stuck" from "UI thread never gets the CPU".

---

## 4. Phone vs emulator: the backend plugins are the whole difference

`/usr/lib64/qt5/plugins/position/libqtposition_geoclue.so` and
`/usr/lib64/qt5/plugins/sensors/libqtsensors_sensorfw.so` exist only on the
phone. `sensorfwd` and `geoclue-master` run only there. So:

- QML/JS logic is testable on the emulator.
- Anything that touches a real sensor, position, modem, Bluetooth, or audio
  backend is **unverified** until it runs on the phone.

Anything that "works on the emulator" but touches these backends is a guess
until proven otherwise.

---

## 5. Sailfish app output goes to journald even when launched directly

Even `setsid env ... /usr/bin/harbour-zensors >/tmp/app.log 2>&1` produces an
empty `app.log` on the emulator. To capture Qt output headless:

```sh
env -u JOURNAL_STREAM QT_LOGGING_TO_CONSOLE=1 ... /usr/bin/harbour-zensors
```

Without `QT_LOGGING_TO_CONSOLE=1` everything Qt-related vanishes into journald.

---

## 6. Silica `Label` != `Text` for links

`linkEnabled` and `onLinkActivated` are **Text** properties, not Label. Using
them on a `Label` gives `Cannot assign to non-existent property "linkEnabled"`,
which prevents the whole page from instantiating ("Could not load page").

```qml
Text {
    textFormat: Text.RichText
    text: "... <a href=\"%1\">%1</a>".arg(url)
    onLinkActivated: Qt.openUrlExternally(link)
}
```

---

## 7. A dropped brace fails silently until the page is opened

Removing one `}` when editing a QML file leaves the component unbalanced. It
may load fine as the initial page in some cases but blow up when pushed from a
menu -- the stack shows "Could not load page". The brace count is the first
thing to check:

```sh
python3 -c "s=open('qml/pages/X.qml').read(); print(s.count('{'), s.count('}'))"
```

---

## 8. Harbour submission checklist

* **Binaries for both architectures** -- aarch64 *and* armv7hl. Emulator (i486)
  builds do not count.
* **Screenshot vs cover are separate fields:**
  * Cover: 1080x540 px (2:1 banner, custom graphic).
  * Screenshots: minimum 1080x1920 px (portrait app captures).
  * Do not upload a portrait screenshot into the cover slot or vice-versa.
* **`LICENSE` file is required** -- Harbour rejects packages without one. The
  spec's `License:` must be a valid SPDX identifier (`GPL-3.0-only`, not the
  literal string `LICENSE`).
* **Privacy policy link** -- put `PRIVACY.md` in the repo root and link to it
  from the submission form.
* **`[X-Sailjail]` permissions** -- only allowlisted ones pass review
  (`Audio, Bluetooth, Camera, Internet, Location, MediaIndexing, Microphone,
  NFC, RemovableMedia, UserDirs, WebView, Documents, Downloads, Music, Pictures,
  PublicDir, Videos, Compatibility, Secrets, Contacts, Accounts`).
  `Sensors` and `Connman` are *not* allowlisted -- fine for local installs, but
  a Store build would have to drop them.
* **README checklist** -- tick the box once you have built with the device kits.

---

## 9. Building both architectures

The tree holds one target's objects at a time. Switching without a clean
packages the previous target's binary (the stale-object trap).

```sh
sfdk build-shell make -o Makefile distclean
sfdk -c target=SailfishOS-5.1.0.11-aarch64 build
sfdk build-shell make -o Makefile distclean
sfdk -c target=SailfishOS-5.1.0.11-armv7hl build
```

---

## 10. Git / GitHub

* **Fine-grained PATs** only work for **organisations**, not personal
  accounts -- that is why there is no "Contents" permission to select.
* A **classic PAT** with the `repo` scope is what you need to push to a private
  personal repo.
* `git push` can leave the PAT inside `.git/config` (in the tracking URL).
  Clean it after:

  ```sh
  git remote set-url origin https://github.com/<user>/<repo>.git
  git config branch.main.remote origin
  git config branch.main.merge refs/heads/main
  ```

* A token pasted into a chat or shell history should be revoked afterwards.

---

## 11. The emulator grab has a transparent background

Everything the app does not paint comes back with **alpha 0**. Viewers show
that as black. Flatten before judging a pixel:

```sh
magick shot.png -background '#303030' -alpha remove flat.png
```

And capture **after a few ticks** -- a screenshot taken 12 s after launch shows
an empty grid because `probe.values` has not populated yet. Wait ~45 s.

---

## 12. `pgrep -f` matches the SSH wrapper

`pgrep -f "/usr/bin/harbour-zensors"` also matches the `bash -c` command string
carrying that text, which once killed the SSH session. Split the pattern so the
wrapper's cmdline does not contain the contiguous string:

```sh
P=$(pgrep -f "/usr/bin/harbour-z""ensors" | head -1)
```
