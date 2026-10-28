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
  centre. Below-horizon Sun positions are omitted from the sky disc and labelled,
  except that a Sun centre from −0.833° to 0° is marked on the rim as an
  apparent-sunrise indicator.
- Signed altitude readouts and explicit labels describe positions near/below
  the horizon; the former zoom panel has been removed.
- Left/right step one minute; down/up step one hour; `n` switches to current
  UTC time; `q` quits.
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
- Minimum terminal is 76 × 36; the former horizon zoom is removed.
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
- The phase disc sits in the Moon panel to the right of the sky chart.
- Primary phase names use ±5° longitude bands; these are display categories.
- Tests cover published primary phases, all names/boundaries, observer independence
  and phase evolution. No eclipse darkening or observer orientation is modelled.

### Horizon zoom removed

- Removed the coarse ±5° horizon zoom and enlarged the sky chart vertically.
- Moved the schematic phase disc into the Moon panel. Signed angles and explicit
  above/below-horizon labels remain; no numerical model changes.
- Planet work was paused before numerical validation; the prototype is now
  saved in the continuation checkpoint described below.

### Horizon styling

- The geometric 0° rim is cyan and bold (bold only on monochrome terminals).
- A chart legend identifies the rim as 0° and the centre as +90°.
- This is the local horizontal reference, not a terrain or refracted skyline.

### First terrain profile implementation

- `--terrain-dem FILE --terrain-ellipsoid` reads a local WGS-84 geographic ESRI
  ASCII grid of ellipsoidal metre heights. No GDAL dependency or automatic datum
  conversion; GeoTIFF preparation is documented in TERRAIN.md.
- 720 rays at 0.5°; default radius 20 km and radial step 100 m, configurable via
  `--terrain-radius`/`--terrain-step`. Ray placement is spherical, elevation is
  WGS-84 ECEF. Cache is in-memory for one launch, not persisted across runs.
- Green `:` skyline, `?` for incomplete coverage, centre-clearance readouts;
  cyan remains 0°. Terrain mode adds a −10° chart margin. Lower angles are not
  clamped to zero. Partial data yields UNKNOWN clearance.
- Sun readout in terrain mode uses the new vector-derived topocentric helper.
  Normal output and the existing solar-event solver retain their prior models.
- Synthetic demo: examples/terrain_demo.ini and terrain_demo.asc at 0°N, 0°E.
  This is artificial terrain, not Ny-Ålesund data. No real DEM has been tested.
- Added geometry and CLI tests for terrain; no sea filling, refraction, limb
  visibility or terrain-aware solar events. Planet work is still paused.

### Continuation checkpoint

- Terrain/TUI implementation is committed in `27c7be3`; the terrain demo date
  has subsequently been changed locally to 2026-04-20 and is preserved here.
- Saved the paused Mercury/Venus/Mars prototype so work can continue on another
  checkout: orbital model, `--objects` names, text blocks and TUI planet table.
  This is unfinished work, not a validated planetary feature. No further planet
  development was performed for this checkpoint.
- All six existing CTest suites pass, but they do not validate planet positions.
  Before resuming planet work, add independent ephemeris fixtures and proper CLI
  selection coverage. The old invalid-object test uses `mars` without observer
  coordinates and can fail for missing coordinates instead of an unknown name.
- The previously generated Doxygen HTML snapshot is included for transfer.
  It predates recent source changes; regenerate it when ready to update API HTML.
- Terrain still has synthetic validation only; no real DEM has been validated.
