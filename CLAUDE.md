# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

ModernHAMLoggerQt is a Qt 6 Quick (QML) desktop app for logging amateur radio
QSOs (contacts), organized into "operations" (e.g. a POTA/SOTA activation or a
contest session). It supports ADIF import/export and online callsign lookups.

## Session continuity

Before starting work, check `.claude/state/` for `plan.md`, `context.md`, and
`tasks.md` (gitignored, personal working notes — not always present). If they
exist, read them first: they capture the current plan, key decisions/reasoning
from prior sessions, and a live task checklist, and prevent re-litigating
settled questions or re-discovering already-fixed bugs. Update them with the
`update-dev-docs` skill before ending a session or compacting.

## Commit signing (non-negotiable)

Every commit to this repository must be signed and verifiable. This repo is
configured for SSH-based signing (`gpg.format=ssh`) with
`user.signingkey=Z:\Repositories\_claude\keys\kc5cd_ed25519.pub` and
`gpg.ssh.allowedsignersfile=Z:\Repositories\_claude\keys\allowed_signers`, and
`commit.gpgsign=true` is set globally. Never create a commit with
`--no-gpg-sign` or otherwise bypass signing. Before ending a session that
created commits, confirm each one is signed and valid with:

```
git log --show-signature -1
```

If signing fails (missing key, misconfigured `gpg.format`/`user.signingkey`),
stop and fix the signing configuration rather than committing unsigned.

## Git workflow

This is a **public** repository for a solo project.

- Never include a Claude session link (`https://claude.ai/code/session_...`)
  or any other internal/session-identifying URL in a commit message. A
  `Co-Authored-By:` trailer is fine; a session link is not.
- Commit locally as normal, but do **not** `git push` unless the user
  explicitly asks for it in that conversation turn. The user works locally
  and pushes in large batches themselves — pushing on their behalf without
  being asked (even "helpfully," even from a background/worktree session)
  is not wanted.

## Build

Requires Qt 6.10+ (Quick, Network, Test, QuickTest components) and CMake 3.22+.

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=<path-to-Qt-6.10-kit>
cmake --build build
```

This is a **dev build**: `appModernHAMLoggerQt.exe` alone, which only runs if
Qt's `bin` directory (DLLs) is already on `PATH`. For a **production build**
that bundles every Qt runtime dependency next to the exe so it runs
standalone on a machine with no Qt installed, configure with
`-DMHL_PRODUCTION_BUILD=ON` and run `cmake --install`:

```
cmake -S . -B build-release -DCMAKE_PREFIX_PATH=<path-to-Qt-6.10-kit> -DCMAKE_BUILD_TYPE=Release -DMHL_PRODUCTION_BUILD=ON
cmake --build build-release
cmake --install build-release --prefix build-release/install
```

This uses Qt's own CMake deployment API
(`qt_generate_deploy_qml_app_script()`, the Qt Quick variant — not
`qt_generate_deploy_app_script()`, which is for Widgets apps), which invokes
`windeployqt` under the hood.

There is no lint config or CMake presets in this repo — don't assume a
formatter target exists.

An `importedcontent/` subdirectory is picked up automatically by the root
`CMakeLists.txt` if it contains its own `CMakeLists.txt` — this is a drop point
for design assets exported from Figma-to-Qt tooling, not hand-written source.

## Tests

Backend logic (`backend/`) and QML have a `ctest`-driven test suite under
`tests/`. Backend code is compiled into a static library (`hamloggercore`,
also the QML module's backing target) that both `appModernHAMLoggerQt` and
every test executable link, rather than being compiled straight into the app.

```
ctest --test-dir build --output-on-failure
```

- `tests/cpp/` — one `QTEST_GUILESS_MAIN` executable per backend area
  (`tst_adifio`, `tst_qsologmodel`, `tst_operationlistmodel`,
  `tst_logbookmanager`), added via the `mhl_add_cpp_test()` helper in
  `tests/CMakeLists.txt`. These link `hamloggercore` directly and construct
  backend classes in plain C++ — no `QQmlEngine` involved, so
  `QML_UNCREATABLE`/`QML_SINGLETON` don't block anything here.
- `tests/qml/` — a Qt Quick Test harness (`tst_qmlharness`, via
  `QUICK_TEST_MAIN_WITH_SETUP`) that scans this directory at runtime for
  `tst_*.qml` files. Currently just a smoke test; add new `tst_*.qml` files
  here with no CMake changes needed. Runs with `-platform offscreen` so it
  never pops a window.
- Any test that touches `LogbookManager` or `StationProfile` **must** call
  `QStandardPaths::setTestModeEnabled(true)` and
  `QSettings::setDefaultFormat(QSettings::IniFormat)` before constructing
  either — both read/write real user data in their constructors (a JSON file
  under `AppDataLocation`, and `QSettings`, which is the Windows registry by
  default and is *not* covered by `setTestModeEnabled` alone).
- `CallsignLookup` owns its own `QNetworkAccessManager` with no injection
  seam and isn't unit-tested; don't write a test that calls `lookup()`
  without adding one first.

## Architecture

**C++ backend (`backend/`) exposed to QML via `QML_ELEMENT`/`QML_SINGLETON`:**

- `LogbookManager` — the app's single entry point into app state, registered
  as a QML singleton (`LogbookManager` global in QML). Owns the list of
  `Operation`s and the `StationProfile`, tracks which operation is currently
  selected, and is the only place that persists state (see below). All QML
  mutations (adding a QSO, importing ADIF, deleting rows) go through its
  `Q_INVOKABLE` methods rather than touching models directly.
- `Operation` — one logging session (name, POTA/SOTA reference, creation
  time) that owns a `QsoLogModel`. `QML_UNCREATABLE`: only `LogbookManager`
  constructs these.
- `OperationListModel` — `QAbstractListModel` of `Operation*`, backs the
  operation sidebar/switcher.
- `QsoLogModel` — `QAbstractTableModel` of `Qso` structs, backs the QSO table.
  Row removal is intentionally *not* `Q_INVOKABLE` on the model itself —
  removal must go through `LogbookManager::deleteQso(s)` so the change gets
  persisted (see Persistence below). Cell edits from the table's edit
  delegates go through `setCell()` rather than property binding, since
  required properties on delegates are one-way and won't reach `setData()`.
- `StationProfile` — the operator's own callsign/name/grid, used to fill in
  ADIF export headers.
- `AdifIo` — static parse/write functions for the ADIF format; no QML
  exposure, used only from `LogbookManager`.
- `CallsignLookup` — QML singleton wrapping an async `QNetworkAccessManager`
  call to the hamdb.org API; results come back via `lookupSucceeded`/
  `lookupFailed` signals rather than a return value, so QML call sites must
  connect to those signals rather than expecting a synchronous result.

**Persistence:** `LogbookManager` loads/saves a single JSON file at
`QStandardPaths::AppDataLocation`/`logbook.json` (`Operation::toJson`/
`fromJson`, `QsoLogModel::toJson`/`fromJson`). Saving is triggered two ways:
`registerOperation()` wires each operation's `QsoLogModel::dataChanged` to
`saveState()` (covers in-place cell edits), while structural changes
(add/select operation, import, row deletion) call `saveState()` explicitly.
When adding a new mutation path, check whether it needs an explicit
`saveState()` call — `dataChanged` alone won't cover row insertion/removal or
metadata changes.

**QML UI (`qml/`, entry point `Main.qml`):**

- `Theme.qml` — singleton (`QT_QML_SINGLETON_TYPE` set in `CMakeLists.txt`)
  holding the light/dark palette. `Main.qml` applies `Theme` colors to the
  `ApplicationWindow.palette.*` properties so that `QtQuick.Controls.Basic`
  controls (which draw from the system palette) pick up the theme — new
  controls should rely on the palette rather than hardcoding `Theme` colors
  directly where possible.
- `OperationSidebar.qml` — operation list/switcher, backed by
  `LogbookManager.operations`.
- `EntryForm.qml` — QSO entry form; submits via
  `LogbookManager.logQso(fields)`, and drives `CallsignLookup` for
  autofill.
- `LogTable.qml` — the QSO table for the current operation, backed by
  `LogbookManager.currentOperation.log`.
- `NewOperationDialog.qml`, `StationProfileDialog.qml` — modal dialogs for
  creating an operation and editing the station profile, respectively.
- `CheckIndicator.qml` — small reusable checkbox-style indicator control.

QML imports the C++ types under the `ModernHAMLoggerQt` module URI (set in
`qt_add_qml_module` in `CMakeLists.txt`); any new backend type needs both the
`QML_ELEMENT`/`QML_SINGLETON` macro and a `SOURCES` entry there, and any new
QML file needs a `QML_FILES` entry there.
