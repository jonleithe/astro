# API Reference

[Back to README](../README.md)

Related docs:

- [Coordinate systems: WGS-84 and ECEF](COORDINATES.md)
- [Build and run](BUILD.md)
- [Project structure](PROJECT_STRUCTURE.md)
- [Git workflow](GIT_WORKFLOW.md)

Primary public headers:

- inc/geo_point.h
- inc/sun_position.h
- inc/observer_vector.h

## Coordinate Types

### MEcefPoint

Cartesian point in an Earth-Centered, Earth-Fixed frame (metres):

- `x`: toward latitude 0°, longitude 0°
- `y`: toward latitude 0°, longitude 90° east
- `z`: toward the conventional north pole

The origin is Earth’s centre of mass. See [ECEF](COORDINATES.md#ecef) for
axis conventions, formulas, and frame limitations.

## Class MGeoPoint

Represents a position relative to the WGS-84 ellipsoid; it need not lie on its surface.

### Constructor

```cpp
MGeoPoint(double lat_deg, double lon_deg, double alt_m = 0.0);
```

Contract:

- Latitude range: [-90, +90] degrees
- Longitude range: [-180, +180] degrees
- Throws `std::domain_error` for out-of-range latitude/longitude
- Supply finite coordinates; the constructor currently does not reject NaN
  or validate altitude finiteness
- `alt_m` is ellipsoidal height, defaulting to zero; no terrain lookup occurs

### Accessors

```cpp
double get_lat(void) const noexcept;
double get_lon(void) const noexcept;
double get_alt(void) const noexcept;
```

Units:

- get_lat/get_lon: decimal degrees
- get_alt: metres above the WGS-84 ellipsoid, not above the geoid or ground

### Geodetic to ECEF

```cpp
MEcefPoint to_ecef(void) const noexcept;
```

Returns ECEF x/y/z in metres.

### Observer-to-target direction

```cpp
struct MHorizontalDirection
{
    double azimuth_deg;
    double elevation_deg;
    double slant_distance_m;
};

MHorizontalDirection direction_to(const MGeoPoint& target) const noexcept;
```

The calling point is the observer. Azimuth is clockwise from true north in
`[0, 360)`. Elevation is signed relative to the observer's local horizontal
plane, perpendicular to the WGS-84 ellipsoid normal. A skyline target therefore
gives the angle between that plane and the visible horizon in that direction;
the angle can be negative for a depressed horizon. Distance is the straight-line
ECEF distance to the target in metres, not distance along the ground.

The calculation includes Earth curvature through ECEF geometry, without
atmospheric refraction or terrain occlusion tests. It does not find the skyline
or load elevation data. Coincident points return NaN angles and zero distance;
vertical targets use zero azimuth by convention because their bearing is undefined.

```cpp
MGeoPoint observer(78.0, 15.0, 500.0);
MGeoPoint target(78.1, 15.2, 200.0);
auto direction = observer.direction_to(target);
```

### ECEF to Geodetic

```cpp
static MGeoPoint from_ecef(double x_m, double y_m, double z_m);
```

Notes:

- Uses Bowring-style iterative latitude refinement
- Input ECEF coordinates are metres; returned latitude/longitude are degrees
  and returned ellipsoidal height is metres
- Intended for ordinary terrestrial positions; the current iterative inverse
  has no dedicated handling for the Earth centre or exact polar axis. Do not
  rely on its height result there

### Great-circle distance

```cpp
static double haversine_distance(const MGeoPoint& a, const MGeoPoint& b) noexcept;
```

Returns spherical great-circle distance in metres with radius 6371008.8 m.
This is an approximation to an ellipsoidal geodesic; it ignores both altitudes
and differs from `direction_to()`’s straight-line slant distance.

## API Surface Guidance

For compatibility and maintainability:

- Treat only the public declarations above as stable API
- Internal constants and conversion helpers are implementation details

## Solar Position API

### MDateTimeUtc

UTC calendar input for solar calculations:

```cpp
struct MDateTimeUtc
{
	int year;
	short month;
	short day;
	short hour;
	short minute;
	short second;
};
```

Notes:

- Interpreted as UTC
- Uses the proleptic Gregorian calendar

### MSunHorizontalPosition

Returned horizontal sky coordinates and selected intermediate values:

```cpp
struct MSunHorizontalPosition
{
	double azimuth_deg;
	double altitude_deg;
	double right_ascension_deg;
	double declination_deg;
	double hour_angle_deg;
	double julian_day;
	double geocentric_distance_m;
};
```

Conventions:

- azimuth_deg: clockwise from true north in [0, 360)
- altitude_deg: signed degrees above the observer’s local horizontal plane,
  not above the visible terrain skyline

### MSunPosition

```cpp
class MSunPosition
{
	public:
		static MSunHorizontalPosition calculate(const MGeoPoint& observer,
												const MDateTimeUtc& utc);
};
```

Behavior:

- Uses apparent solar equatorial coordinates to calculate geometric horizontal
  angles; atmospheric refraction is not applied to `altitude_deg`
- Observer height, terrain, and topocentric solar parallax are not applied
- Throws std::invalid_argument if the UTC calendar fields are invalid
- Uses Meeus/NOAA-style solar equations with local sidereal time

### Daily solar events

```cpp
static MSunDailyEventsUtc calculate_daily_events(const MGeoPoint& observer,
												 const MDateTimeUtc& utc_date);
```

Where:

```cpp
struct MSunDailyEventsUtc
{
	bool has_sunrise_sunset;
	double solar_noon_utc_hours;
	double sunrise_utc_hours;
	double sunset_utc_hours;
	double equation_of_time_minutes;
	double declination_deg;
};
```

Notes:

- Uses UTC calendar date (time fields ignored)
- Uses numerical crossings of solar-centre altitude -0.833°, a conventional
  approximation for upper-limb visibility including standard refraction
- Observer height, horizon dip, terrain, and actual weather are not applied
- The pair surrounds the date’s solar transit; decimal UTC hours may be outside
  `[0, 24)` and must retain their date offset
- If a complete pair is unavailable, `has_sunrise_sunset` is false and both
  sunrise/sunset fields are -1; this includes polar day/night
- This interface does not represent multiple terrain visibility transitions

## Observer-relative vectors

`inc/observer_vector.h` defines `MObserverVector { east, north, up }`.
Position components and `magnitude()` are in metres; `unit_direction()` returns
another vector with dimensionless components. A zero vector has zero magnitude
and returns three NaNs when normalized. Supply finite inputs.

```cpp
MGeoPoint observer(78.24490, 15.49369, 8.0);
MDateTimeUtc utc{2026, 6, 21, 12, 0, 0};
auto sun = MSunPosition::calculate_vector(observer, utc);
double distance_m = sun.magnitude();
auto direction = sun.unit_direction();
```

Call `calculate_vector` for each UTC timestamp to track the Sun over time. Each
result uses the observer's local east–north–up axes at that timestamp, not fixed
inertial axes. Invalid UTC fields throw `std::invalid_argument`.

The solar vector points toward the Sun's centre. The orbital radius is estimated
from eccentricity and true anomaly, with AU = 149597870700 metres. The existing
apparent solar RA/declination and sidereal rotation supply the direction; the
WGS-84 observer ECEF position is subtracted before rotating into ENU. Observer
height and solar parallax are therefore included. The added
`MSunHorizontalPosition::geocentric_distance_m` is Earth-centre distance;
`calculate_vector(...).magnitude()` is observer distance.

This combines the existing approximate apparent direction with an orbital radius;
it is not a precision simultaneous geometric ephemeris. It does not add explicit
light-time iteration, Earth orientation data, atmospheric refraction or terrain
visibility. Existing `calculate()` angles and daily events retain their previous
height/parallax conventions and can differ slightly from vector-derived angles.
No new precision guarantee is implied by the printed decimal places.

For another object whose Earth-fixed position is available:

```cpp
MEcefPoint target = {10000000.0, 20000000.0, 30000000.0}; // metres
auto relative = MObserverVector::from_ecef(observer, target);
```

Supply target and observer in the same Earth-fixed frame and at the same epoch.
The vector stores no epoch or object identity. Planet and star ephemerides
are not yet implemented; future providers can feed this shared conversion.
The CLI prints the solar ENU vector, unit direction and observer distance in km.

## Moon position

`inc/moon_position.h` provides
`MMoonPosition::calculate(observer, utc) -> MMoonHorizontalPosition`.
The result holds `azimuth_deg` clockwise from true north in [0, 360), signed
`altitude_deg` above the geometric horizon, and observer-to-Moon-centre
`distance_m`. Invalid UTC calendar fields throw `std::invalid_argument`.

The implementation uses [Paul Schlyter's lunar orbital elements and major
perturbations](https://stjarnhimlen.se/comp/ppcomp.html), rotates equatorial
coordinates of date into ECEF using mean sidereal time, then subtracts the
WGS-84 observer position using the shared ENU conversion. Observer height and
lunar parallax are included. UTC approximates UT; TT, Earth orientation data,
nutation, aberration, light-time, atmospheric refraction and terrain are omitted.
This is an approximate visualization model intended near 1900–2100, with no
precision ephemeris guarantee. Printed decimals do not imply equivalent accuracy.
A published 1990 topocentric example is checked within 0.06° in the test suite;
this single fixture is not an accuracy bound over the full date range.

The TUI displays the Moon alongside the existing Sun model. Their observer
parallax/height conventions differ as documented above. Daily events remain
solar events; lunar rise/set times are not calculated.

### Lunar phase

`MMoonHorizontalPosition` also returns `illuminated_fraction` in [0, 1],
`phase_cycle_deg` (Moon-minus-Sun geocentric ecliptic longitude in [0, 360))
and `waxing` (cycle angle below 180°). The phase uses the same approximate
orbital model, with a finite-distance Earth–Moon–Sun triangle for illumination.
These geocentric values are independent of the observer and do not model eclipses.
The percentage describes illuminated disc area, not brightness.

`phase_name()` returns eight display categories. Primary names use half-open
10° bands centred on 0°, 90°, 180°, 270°; other angles use crescent/gibbous
names. These bands are an application convention, not phase-event predictions.
The TUI disc is schematic: waxing right-lit, waning left-lit, no observer rotation
or libration. Text Moon output includes name and percentage as well.

Reference definitions: [USNO phases and illumination](https://aa.usno.navy.mil/faq/moon_phases).
Tests compare four published 2026 phase dates from
[Espenak's ephemeris](https://astropixels.com/ephemeris/moon/phases2001.html)
within one percentage point, alongside naming bands and waxing/waning checks.
