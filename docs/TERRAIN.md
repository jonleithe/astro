# Terrain skyline

The optional terrain profile is a sampled geometric obstruction model, separate
from the cyan local horizontal reference. It does not calculate weather-dependent
visibility. See the [demo command](../README.md#terrain-horizon-first-version).

## Input data

The dependency-free reader accepts only local ESRI ASCII grids with:

- `ncols`, `nrows`, `xllcorner`/`xllcenter`, `yllcorner`/`yllcenter`, `cellsize`;
  optional `NODATA_value` (default −9999), followed by north-to-south rows.
- WGS-84 longitude/latitude in degrees, one square angular cell size, and
  WGS-84 ellipsoidal elevations in metres. The ASCII format cannot establish the
  CRS or vertical datum: the user confirms both by preparing the raster and
  supplying `--terrain-ellipsoid`. Sidecar `.prj` files are not inspected.
- Finite cell values and at most 10 million cells (about 80 MB elevation storage).
  Nodata values and points outside the grid are missing data, never sea level.

Original [ArcticDEM products](https://www.pgc.umn.edu/data/arcticdem/) have WGS-84
ellipsoidal heights. Some web services redistribute heights in a geoidal datum;
check the exact product rather than relying on the dataset name.

## Preparing a GeoTIFF

GDAL is an external preparation tool, not a build/runtime dependency of astro.
For an already ellipsoidal ArcticDEM raster, this example crops a small region
around Ny-Ålesund and reprojects it to a square angular grid:

```bash
gdalwarp -t_srs EPSG:4326 -te 11.0 78.7 12.8 79.1 -tr 0.001 0.001 \
  -r near -dstnodata -9999 input_ellipsoidal.tif local_dem.tif
gdal_translate -of AAIGrid -ot Float32 local_dem.tif local_dem.asc
```

Adjust the region and resolution to your data and search radius. These commands
are an example, not an automatic download or vertical-datum conversion. Confirm
that source and output values are ellipsoidal metres; a horizontal reprojection
alone does not convert orthometric heights. GDAL is not installed in the current
development environment, so this conversion example has not been run here.

Then use your actual observer ellipsoidal eye height:

```bash
./bin/astro -r observer.ini --tui --terrain-dem local_dem.asc --terrain-ellipsoid
```

## Sampling and coverage

The profile contains 720 rays spaced 0.5° apart. Each ray samples the DEM cell
containing points at the requested radial step, ending at the search radius.
Ray placement uses a spherical forward path (radius 6371008.8 m). Each sampled
height is then converted through WGS-84 ECEF geometry, which includes Earth
curvature and observer height in the elevation angle. This hybrid is approximate,
not an ellipsoidal geodesic ray tracer. It has not been validated against a real
surveyed skyline. The observer must lie in valid DEM coverage at or above ground.

Defaults: radius 20000 m, radial step 100 m. Allowed radius: 100–100000 m;
step: 10 m through the radius. Features between rays/steps, within the first step,
beyond the radius, or unresolved in the DEM may be missed. Choose finer sampling
for nearby terrain; a coarse TUI does not justify coarse visibility calculations.
At high latitude, one angular pixel's east-west size is much smaller than its
north-south size. Square angular cells do not mean square ground cells.

The maximum sampled elevation along each ray is retained, including negative
angles. An azimuth lookup takes the larger neighbouring ray maximum, wrapping
through north, to avoid smoothing a peak downward. This is conservative relative
to those samples only, not a guarantee that all intervening terrain is represented.
Any missing radial sample marks its ray incomplete; either adjacent incomplete
ray makes object clearance UNKNOWN. Green `?` marks incomplete skyline samples.
The source path, range, spacing and complete-ray count are reported in text;
the TUI shows sampling settings and complete-ray count. The profile is cached
in memory for one launch only. Restart after changing the observer or DEM.

## Display and scientific limits

Green `:` is the terrain outline; cyan remains the 0° reference. Hidden objects
remain plotted with a signed centre clearance (`object altitude − terrain angle`).
Positive means clear of the sampled profile, zero/negative means hidden; UNKNOWN
means insufficient raster coverage. Results concern object centres, not full discs.
An outer chart margin allows down to −10°. Lower terrain/objects are omitted
rather than clamped to zero; their numerical values are retained.

No refraction, vegetation/building detail beyond what the DEM captures, sea-surface
model, tides, geoid conversion or terrain-aware event solver is supplied. DEM voids
over water remain unknown; they are never filled with ellipsoidal zero. In
terrain mode the Sun's displayed horizontal angles use `calculate_topocentric`
so the object and skyline share observer height/parallax conventions. The normal
Sun output is unchanged when terrain is disabled. Daily solar events remain the
standard −0.833° calculation and may differ from terrain emergence/disappearance.

Synthetic regression tests cover ridges, depressed horizons, observer height,
north wrapping, mask/out-of-bounds coverage and malformed input. The bundled
example is generated artificial terrain, not geographically accurate elevation.
