# AGENTS.md — drone_engage_sdr_module

DroneEngage SDR module (`de_sdr`). Software-defined radio module for the
DroneEngage bus. C++17. See parent `../AGENTS.md` for workspace
architecture, `de_common` vendoring, and config conventions.

## Build

    ./build.sh                 # DEBUG (only build script shipped)

Out-of-source in `build/`. Binaries: `bin/de_sdr`, `bin/de_sdr.so`.

### CMake options

- Version hard-coded (`1.0.1`), no `.version` auto-increment.
- No `DDEBUG` option in this module's CMakeLists.

### Dependencies

Threads. The CMakeLists has **commented-out** `find_package` calls for
SoapySDR, liquid, rtaudio, FFTW3 — these are conditionally/optionally
needed depending on the SDR backend in use. Re-enable the relevant
`find_package` lines if you build against those backends. `3rdparty/`
holds vendored deps.

## Config

- `de_sdr.config.module.json` — module config (WebClient UI).
- `de_sdr.local` — instance identity (in `bin/`).

## Source layout

`src/` — `main.cpp`, `sdr/`, `de_common/` (vendored), `defines.hpp`,
`global.hpp`, `version.h`. `resources/` — logos/assets. `todo.txt` —
open work items.
