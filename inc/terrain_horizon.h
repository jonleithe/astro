/**
 * @file terrain_horizon.h
 * @brief Sampled, observer-specific geometric terrain skyline.
 */
#pragma once

#include <string>
#include <vector>
#include <limits>
#include "geo_point.h"

/// @brief Terrain height along one sampled azimuth ray.
struct MTerrainRay
{
    double altitude_deg = -std::numeric_limits<double>::infinity(); ///< Highest sampled angle.
    bool complete = true; ///< All radial samples had elevation data.
};

/// @brief Sampled skyline value at an arbitrary azimuth.
struct MTerrainDirection
{
    double altitude_deg; ///< Conservative maximum of adjacent ray angles, degrees.
    bool complete; ///< Both adjacent rays have complete sample coverage.
};

/// @brief In-memory skyline cache built once for an observer and DEM.
/// @note ASCII input must be a north-up ESRI grid in WGS-84 longitude/latitude
/// degrees with ellipsoidal heights in metres. No vertical datum conversion,
/// sea-surface filling, refraction or terrain-aware event calculation is applied.
class MTerrainHorizon
{
public:
    std::vector<MTerrainRay> rays; ///< Uniform azimuth samples starting at north.
    double radius_m = 0; ///< Maximum radial sampling distance in metres.
    double step_m = 0; ///< Radial sample spacing in metres.
    std::string source; ///< Input raster path for provenance.

    /// @brief Load an ASCII DEM and build the observer skyline.
    /// @param path Local ESRI ASCII raster; heights must be WGS-84 ellipsoidal metres.
    /// @param observer WGS-84 eye/instrument position, not ground height.
    /// @param radius_m Search radius in metres, [100, 100000].
    /// @param step_m Radial spacing in metres, [10, radius_m].
    /// @return Cached 720-ray profile, including missing-data flags.
    /// @throws std::invalid_argument For malformed raster, parameters or observer.
    /// @note Ray locations use spherical forward sampling; angles and curvature
    /// use WGS-84 ECEF. This finite-resolution profile is not an exact skyline.
    static MTerrainHorizon load_ascii(const std::string& path, const MGeoPoint& observer,
                                      double radius_m = 20000, double step_m = 100);



    /// @brief Look up the sampled obstruction at an azimuth, wrapping north.
    /// @param azimuth_deg Finite clockwise bearing in degrees.
    /// @return Highest neighbouring ray angle and combined coverage status.
    /// @throws std::invalid_argument For nonfinite bearing or an empty profile.
    MTerrainDirection at(double azimuth_deg) const;
};
