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
