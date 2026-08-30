# ModernHAMLoggerQt

A Qt 6 Quick (QML) desktop app for logging amateur radio QSOs (contacts),
organized into "operations" (e.g. a POTA/SOTA activation or a contest
session).

## Features

- Log QSOs into named operations, with a sidebar to create, switch between,
  and delete operations
- QSO entry form with live callsign autofill via online lookup
  (hamdb.org), auto-uppercase callsign input, and a sortable QSO table that
  auto-fits its columns to content
- ADIF import/export, so logs can move to/from other loggers and
  contest/award tools
- Station profile (callsign, name, grid square) used to fill in ADIF export
  headers
- Light/dark theming

## Build

Requires Qt 6.10+ (Quick, Network components) and CMake 3.16+.

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=<path-to-Qt-6.10-kit>
cmake --build build
```

## Status

Actively developed, pre-1.0. Core logging, ADIF import/export, and operation
management are working; UI polish and additional features are ongoing.

## License

MIT — see [LICENSE](LICENSE).
