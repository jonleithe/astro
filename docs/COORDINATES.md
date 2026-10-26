# Coordinate systems and heights

[Back to README](../README.md) · [API reference](API.md)

## WGS-84

WGS-84 (World Geodetic System 1984) defines a global reference system and a
reference ellipsoid. The ellipsoid is a smooth, flattened model of Earth;
it does not describe mountains, buildings, or mean sea level everywhere.

`MGeoPoint(latitude, longitude, altitude)` uses its ellipsoid geometry:

| Quantity | Meaning | Units |
| --- | --- | --- |
| Latitude, φ | Angle of the ellipsoid normal above the equatorial plane; north positive | Degrees |
| Longitude, λ | Angle east of the reference meridian; west negative | Degrees |
| Height, h | Signed distance from the ellipsoid along its normal | Metres |

Geodetic latitude differs from geocentric latitude, which measures the direction
of the vector from Earth's centre. The API uses geodetic latitude.

The defining geometric constants used in the code are:

```text
a  = 6378137.0 m             equatorial semi-major axis
f  = 1 / 298.257223563       flattening
e² = 2f − f²                eccentricity squared
b  = a(1 − f)               polar semi-minor axis, approximately 6356752.314 m
```

These constants follow [NGA's WGS-84 specification](https://earth-info.nga.mil/index.php?action=wgs84&dir=wgs84).
The library implements ellipsoid geometry, not the complete WGS-84 gravity model
or transformations between reference-frame realizations and epochs.

## Ellipsoid, geoid, and terrain heights

The **geoid** is a gravity equipotential surface approximating mean sea level.
The **terrain** is the actual ground surface. Neither coincides everywhere with
the reference ellipsoid.

The usual height relationship is:

```text
h = H + N
```

Here `h` is ellipsoidal height, `H` is orthometric height above the geoid, and `N`
is geoid undulation (positive where the geoid is above the ellipsoid). A ground
elevation `H = 100 m` with `N = +40 m` gives `h = 140 m`. An observer 2 m above
that ground would have approximately `h = 142 m`.

Use the geoid model matching the elevation dataset's vertical reference. Some
datasets use other height systems and require a different conversion; do not
assume every elevation marked “above sea level” uses the same datum.

`MGeoPoint(lat, lon, 0)` means a point on the ellipsoid. It does not mean sea
level or ground level. Latitude and longitude alone require an external digital
elevation model (DEM) to obtain ground height. No DEM or geoid lookup is currently
implemented.

## ECEF

ECEF means **Earth-Centered, Earth-Fixed**. Its Cartesian axes rotate with Earth:

| Axis | Positive direction |
| --- | --- |
| X | Equator at longitude 0° |
| Y | Equator at longitude 90° east |
| Z | Conventional terrestrial north pole |

The origin is Earth's centre of mass. The axes form a right-handed system and
all three coordinates are in metres. ECEF is neither a map projection nor a
celestial frame. See the ECEF definition in [NOAA's positioning guidelines](https://geodesy.noaa.gov/PUBS_LIB/NGSRealTimeUserGuidelines.v2.0.4.pdf).

The forward conversion in `MGeoPoint::to_ecef()` is:

```text
ν = a / sqrt(1 − e² sin²φ)
X = (ν + h) cosφ cosλ
Y = (ν + h) cosφ sinλ
Z = (ν(1 − e²) + h) sinφ
```

Angles must be converted to radians for these trigonometric operations.
Here `ν` is the prime-vertical radius of curvature, not geoid undulation `N`.

For example:

```cpp
MGeoPoint point(0.0, 0.0, 0.0);
MEcefPoint xyz = point.to_ecef(); // {6378137.0, 0.0, 0.0}
MGeoPoint restored = MGeoPoint::from_ecef(xyz.x, xyz.y, xyz.z);
```

All three ECEF coordinates are needed to recover a general position and height.
The inverse currently uses iterative latitude refinement; see the
[API limitations](API.md#ecef-to-geodetic) for exact polar-axis and Earth-centre inputs.

`MEcefPoint` carries no frame identifier, epoch, velocity, or uncertainty.
Coordinates from ITRF or another realization must be reconciled to a consistent
frame and epoch before precision work; applying ellipsoid conversion alone does
not perform that transformation. Earth orientation parameters are used when
relating terrestrial and celestial frames, not to supply terrain elevation.

## Local horizontal coordinates and the visible horizon

`observer.direction_to(target)` subtracts ECEF positions and rotates the vector
into the observer's local east–north–up (ENU) frame. “Up” follows the ellipsoid
normal; the east–north plane is the local horizontal plane. It is an ellipsoidal
reference, not a model of the small local deviations of gravity's vertical.

For observer latitude `φ`, longitude `λ`, and ECEF difference `(dX, dY, dZ)`:

```text
east  = −sinλ dX + cosλ dY
north = −sinφ cosλ dX − sinφ sinλ dY + cosφ dZ
up    =  cosφ cosλ dX + cosφ sinλ dY + sinφ dZ

azimuth   = atan2(east, north), normalized to [0°, 360°)
elevation = atan2(up, hypot(east, north))
distance  = sqrt(dX² + dY² + dZ²)
```

The returned `MHorizontalDirection` contains:

- `azimuth_deg`: clockwise from true north, not magnetic north.
- `elevation_deg`: signed angle from the observer's local horizontal plane.
- `slant_distance_m`: straight-line distance to the target, not a surface arc.

A target above the horizontal plane has positive elevation. A distant ocean
horizon viewed from a mountain can have negative elevation because the observer
looks down towards it. A nearby ridge may instead have positive elevation.
When the target defines the skyline, its elevation is the horizon-mask value
at that azimuth; arbitrary targets do not necessarily belong to the skyline.

ECEF geometry includes Earth's curvature. It does not model atmospheric
refraction or determine whether the target is hidden by intervening terrain.
Coincident positions have undefined angles (returned as NaN). Vertically aligned
targets have undefined azimuth (returned as zero by convention). At an exact
pole, the observer's supplied longitude selects the local horizontal axes.

## Horizon profiles and solar events

A future 360° terrain profile would retain the highest terrain angle in each
direction for a specified observer position and height. It needs surrounding
DEM data, consistent vertical references, sufficient sampling and search range,
and explicit handling of missing data and sea surface. The profile must preserve
negative angles and wrap around north.

The existing `direction_to()` supplies geometric directions only. It does not
build a profile, and the current solar-event solver does not consume one.
Its fixed solar-centre threshold of −0.833° approximates standard upper-limb
sunrise/sunset. Terrain-aware events would instead compare solar visibility with
the horizon profile, handling solar radius and refraction without counting those
corrections twice. Terrain can cause multiple appearances and disappearances.

## Celestial position vectors

`MSunPosition::calculate_vector(observer, utc)` returns the observer-to-Sun vector
in local ENU metres. Its length is distance, and normalization gives direction.
Unlike the existing solar angle API, it subtracts the observer's full WGS-84
position, including height. Below-horizon vectors remain valid; no occlusion
is inferred. See the [vector API and model limitations](API.md#observer-relative-vectors).

`MObserverVector::from_ecef` uses the same subtraction and ENU rotation described
above for any target. Celestial positions in an inertial frame must first be
transformed into a compatible Earth-fixed frame at the observation epoch.
