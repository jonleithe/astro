#pragma once

#include <cmath>
#include <limits>
#include "geo_point.h"

/// @brief Observer-relative Cartesian vector in local east–north–up axes.
/// Components are metres for positions, dimensionless for unit_direction().
/// No epoch is stored: callers must supply target and observer at the same time.
struct MObserverVector
{
    double east;  ///< East component.
    double north; ///< North component.
    double up;    ///< Up component along the WGS-84 ellipsoid normal.

    /// @brief Euclidean length, in the same units as the components.
    double magnitude() const noexcept { return std::hypot(east, north, up); }

    /// @brief Unit direction; all components are NaN for a zero vector.
    MObserverVector unit_direction() const noexcept
    {
        const double length = magnitude();
        if (length == 0.0)
        {
            const double nan = std::numeric_limits<double>::quiet_NaN();
            return {nan, nan, nan};
        }
        return {east / length, north / length, up / length};
    }

    /// @brief Subtract observer ECEF position and rotate into local ENU, metres.
    /// @param observer WGS-84 observer, including ellipsoidal height.
    /// @param target Target ECEF position in metres at the observer's epoch.
    static MObserverVector from_ecef(const MGeoPoint& observer, const MEcefPoint& target) noexcept
    {
        constexpr double radians = 3.14159265358979323846 / 180.0;
        const double latitude = observer.get_lat() * radians;
        const double longitude = observer.get_lon() * radians;
        const auto origin = observer.to_ecef();
        const double x = target.x - origin.x;
        const double y = target.y - origin.y;
        const double z = target.z - origin.z;
        const double radial = std::cos(longitude) * x + std::sin(longitude) * y;
        return {
            -std::sin(longitude) * x + std::cos(longitude) * y,
            -std::sin(latitude) * radial + std::cos(latitude) * z,
            std::cos(latitude) * radial + std::sin(latitude) * z
        };
    }
};
