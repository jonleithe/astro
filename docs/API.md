# API Reference

[Back to README](../README.md)

Related docs:

- [Build and run](BUILD.md)
- [Project structure](PROJECT_STRUCTURE.md)
- [Git workflow](GIT_WORKFLOW.md)

Primary public header: inc/geo_point.h

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
