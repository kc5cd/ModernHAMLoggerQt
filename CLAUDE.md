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

## Build

Requires Qt 6.10+ (Quick, Network components) and CMake 3.16+.

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=<path-to-Qt-6.10-kit>
cmake --build build
```

There is no test suite, lint config, or CMake presets in this repo — don't
assume `ctest` or a formatter target exists.

An `importedcontent/` subdirectory is picked up automatically by the root
`CMakeLists.txt` if it contains its own `CMakeLists.txt` — this is a drop point
for design assets exported from Figma-to-Qt tooling, not hand-written source.

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
