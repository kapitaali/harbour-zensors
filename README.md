# harbour-myapp

A minimal, Harbour-compliant Sailfish OS application template.

## Build & Run

**In the Sailfish IDE:** `File > Open File or Project > harbour-myapp.pro`, pick a kit
(`SailfishOS-<ver>-i486` for emulator, `-armv7hl` / `-aarch64` for device), then run.

**With sfdk (CLI):**

```bash
./scripts/build.sh            # build RPM
./scripts/build.sh --sign     # signed RPM (needs signing config, see scripts/build.sh)
```

Deploy by copying `RPMS/*.rpm` to the device and `rpm -i`, or let the IDE's
"Deploy as RPM Package" kit option handle it.

## Rename the template

```bash
./scripts/rename.sh harbour-myapp harbour-yourapp
```

## Project layout

| Path | Purpose |
|---|---|
| `harbour-myapp.pro` | qmake project; `CONFIG += sailfishapp` pulls standard install rules |
| `src/harbour-myapp.cpp` | Entry point: `SailfishApp::main()` loads `qml/harbour-myapp.qml` |
| `qml/harbour-myapp.qml` | `ApplicationWindow`, page stack, cover |
| `qml/pages/` | Page components (pull-down menu, page stack demo) |
| `qml/cover/CoverPage.qml` | Home screen cover + cover action |
| `rpm/harbour-myapp.spec` | RPM packaging (`%build`, `%install`, `%files`) |
| `harbour-myapp.desktop` | Launcher entry incl. `[X-Sailjail]` sandbox permissions |
| `icons/` | App icons 86/108/128/172 (generate with `scripts/generate-icons.sh`) |
| `translations/` | `qsTr()` translation sources (`.ts`) |
| `tests/auto/` | Qt Quick Test skeleton + `tests.xml` for CI |

## Harbour checklist

- [x] Target/icon/desktop all named `harbour-myapp`
- [x] `[X-Sailjail]` section with minimal permissions (`Internet`)
- [x] Only allowlisted QML imports (`QtQuick 2.6`, `Sailfish.Silica 1.0`)
- [x] RPM spec limited to allowlisted `Requires`/`BuildRequires`
- [x] All user-visible strings wrapped in `qsTr()`
- [x] Icon PNGs generated (run `scripts/generate-icons.sh`)
- [ ] RPM validator passes on deploy (shown in the IDE compile window)
- [ ] Built with `-armv7hl` or `-aarch64` kit before Harbour submission

## References

- SDK workflow: https://docs.sailfishos.org/Develop/Apps/Your_First_App/
- Packaging: https://docs.sailfishos.org/Develop/Apps/Packaging/
- Allowed APIs: https://docs.sailfishos.org/Develop/Apps/Harbour/Allowed_APIs/
- Allowed permissions: https://docs.sailfishos.org/Develop/Apps/Harbour/Allowed_Permissions/
- Sandbox setup: https://docs.sailfishos.org/Develop/Apps/Application_Permissions/
- Submit to Harbour: https://harbour.jolla.com/
