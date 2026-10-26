# Development handoff

Updated: 2026-10-26.

## Current state

- The CLI calculates solar azimuth/altitude and UTC daily events, plus an
  observer-to-Sun ENU vector, distance, and unit direction.
- Large output numbers have space-separated groups of three digits, e.g.
  `148 081 453.635 km`. Formatting lives in `src/print_output.cpp`.
- CLI help is a raw string in `inc/print_output.h`; `print_usage()` prints via
  `std::cout`, replacing `{program}` with the executable name. The refactor
  preserved help output byte for byte.
- `ny_aalesund.ini` has its example date commented out. Pass an explicit date
  and time for reproducible comparisons.
- `64d3557` added solar vectors and grouped output; `a405ea5` contains the help
  refactor and configuration adjustment. Both were confirmed pushed to main.
- Validation before this handoff: debug build succeeded, both existing tests
  passed after the vector/formatting changes, and the help refactor built and
  passed a before/after output comparison.

## Terminal visualization implemented

- `--tui` opens an optional ncurses interface, implemented in `src/tui.cpp`.
- ASCII sky chart: north up, east right, geometric horizon at rim, zenith at
  centre. Below-horizon Sun positions are omitted from the sky disc and labelled.
- A separate ±5° altitude scale shows the Sun near the horizon.
- Left/right step one minute; down/up step one hour; `r` resets; `q` quits.
  The timestamp is fixed between key presses. UTC stepping handles date rollover.
- Observer, precise solar angles and daily events are displayed. Event times
  retain day offsets; no complete rise/set pair is labelled explicitly.
- Resize handling shows a prompt below 76 columns by 36 rows.
- CMake detects ncurses optionally; `ASTRO_ENABLE_TUI=OFF` disables support.
  Normal CLI operation remains available without it.
- `--tui` rejects redirected input/output and name-edit mode.

Reference case:

```bash
./bin/astro --tui -r ny_aalesund.ini --date 2026-02-25 --time 08:56:07
```

The Sun is southeast at azimuth 143.18°, altitude −0.19°.

Proposed later work: dotted daily path, adjustable step sizes, and visual polish.

### Moon added to the TUI

- Yellow `*` Sun and white `o` Moon share the sky chart and time controls.
  `@` marks a shared chart cell. Both have numeric angles and horizon labels.
- Horizon zoom has separate marker columns; minimum terminal is now 76 × 36.
- `moon_position` implements Schlyter's approximate lunar orbit with major
  perturbations and WGS-84 observer parallax/height. Limitations and source
  are documented in API.md. Solar calculations and normal CLI output are unchanged.
- The lunar suite checks a published topocentric fixture, polar/equatorial
  geometry, distance, time evolution, observer height and invalid UTC.
- Lunar rise/set calculation is not implemented.

### Object selection

- `--objects sun,moon` filters text output and TUI markers/panels. Without it,
  text remains Sun-only and TUI shows both. Selection is lowercase and CLI-only.
- Moon text output includes timestamp, azimuth, altitude and grouped distance.
  Daily events appear only for the Sun; output order is always Sun then Moon.
- Empty/unknown entries, repeated flags and name-edit combinations are rejected;
  duplicate object names are harmless. Help and Bash completion include selection.
- CTest CLI coverage checks selection combinations and invalid lists.

### Lunar phase visualization

- Moon results now include geocentric illumination fraction, cycle longitude
  and waxing/waning. Text and TUI show phase name and percentage.
- TUI adds a separate ASCII schematic disc: `#` lit, `.` dark, waxing right-lit,
  waning left-lit. Sky marker remains `o`; minimum dimensions stay 76 × 36.
- Sky chart is smaller vertically to leave space for the phase disc beneath it.
- Primary phase names use ±5° longitude bands; these are display categories.
- Tests cover published primary phases, all names/boundaries, observer independence
  and phase evolution. No eclipse darkening or observer orientation is modelled.
