# API Reference

[Back to README](../README.md)

Related docs:

- [Build and run](BUILD.md)
- [Project structure](PROJECT_STRUCTURE.md)
- [Git workflow](GIT_WORKFLOW.md)

Primary public headers:

- inc/geo_point.h
- inc/sun_position.h

## Coordinate Types

### MEcefPoint

Simple Cartesian point in ECEF coordinates (metres):

- x
- y
- z

## Class MGeoPoint

Represents a geodetic point on WGS-84.

### Constructor

```cpp
MGeoPoint(double lat_deg, double lon_deg, double alt_m = 0.0);
```

Contract:

- Latitude range: [-90, +90] degrees
- Longitude range: [-180, +180] degrees
- Throws std::domain_error on invalid latitude/longitude

### Accessors

```cpp
double get_lat(void) const noexcept;
double get_lon(void) const noexcept;
double get_alt(void) const noexcept;
```

Units:

- get_lat/get_lon: decimal degrees
- get_alt: metres

### Geodetic to ECEF

```cpp
MEcefPoint to_ecef(void) const noexcept;
```

Returns ECEF x/y/z in metres.

### ECEF to Geodetic

```cpp
static MGeoPoint from_ecef(double x_m, double y_m, double z_m);
```

Notes:

- Uses Bowring-style iterative latitude refinement
- Input/output units are metres and degrees

### Great-circle distance

```cpp
static double haversine_distance(const MGeoPoint& a, const MGeoPoint& b) noexcept;
```

Returns distance in metres using the Haversine formula with a WGS-84 mean Earth radius.

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
- altitude_deg: degrees above the geometric horizon

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

- Calculates the Sun's apparent horizontal position for an observer on Earth
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
- sunrise/sunset are based on 90.833 deg apparent zenith convention
- In polar day/night, has_sunrise_sunset is false
