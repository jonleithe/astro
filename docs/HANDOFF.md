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

## Next discussion: terminal visualization

The user intends to resume on austonna after travelling from isachsenfonna.
They liked the following TUI direction. No TUI implementation has started;
the dependency and detailed interaction choices remain proposals.

- Proposed opt-in entry point: `astro --tui`, keeping normal CLI output usable.
- Observer-centred circular sky chart: north at top, east at right; horizon at
  the outer circle (0 degrees altitude), zenith at centre (+90 degrees).
- Mark the Sun and show precise azimuth/altitude alongside the chart.
- Add a magnified side view of the horizon so small negative and positive
  altitudes remain readable. A Sun below the horizon must be clearly labelled,
  rather than appearing as an above-horizon position on the sky chart.
- Show timestamp, observer, and daily event information.
- Adapt to terminal dimensions. Consider an ASCII marker alongside Unicode.
- ncurses was suggested, but has not been selected or added to the build.
- Possible later additions: dotted daily solar path and time-stepping controls.
- Keep drawing separate from existing calculation code.

Useful reference case:

```bash
./bin/astro -r ny_aalesund.ini --date 2026-02-25 --time 08:56:07
```

At this time the Sun is southeast, azimuth 143.18 degrees, altitude -0.19
degrees. This is a useful case for checking visibility just below the horizon.

Before implementation, inspect the current CLI and build structure and settle
the first TUI scope with the user. This note records context, not an instruction
to start implementing the TUI automatically.
