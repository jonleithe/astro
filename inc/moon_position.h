/**
 * @file moon_position.h
 * @brief Approximate observer-relative lunar position and phase API.
 */

#pragma once

#include "sun_position.h"

/// @brief Observer-relative lunar direction, distance and geocentric phase.
struct MMoonHorizontalPosition
{
    /// @brief Azimuth in degrees clockwise from true north, [0, 360).
    double azimuth_deg;
    /// @brief Altitude in degrees above the geometric horizon; no refraction.
    double altitude_deg;
    /// @brief Observer-to-Moon-centre distance in metres.
    double distance_m;
    /// @brief Geocentric illuminated disc area, [0, 1].
    double illuminated_fraction;
    /// @brief Moon minus Sun ecliptic longitude, [0, 360).
    double phase_cycle_deg;
    /// @brief Cycle angle < 180 degrees.
    bool waxing;

    /// @brief Return the lunar phase display category.
    /// @return A static phase name; no ownership is transferred.
    /// @note Display categories: +/-5 degrees around each primary phase, intermediate
    /// names elsewhere. These are naming bands, not exact phase event times.
    const char* phase_name() const noexcept;
};

/// @brief Approximate Schlyter lunar orbit with major perturbations, equinox of date.
/// @note Includes WGS-84 observer parallax/height. Uses UTC as UT; omits TT, nutation,
/// aberration, light-time and terrain. Intended for visualization near 1900-2100,
/// not precision ephemerides. Invalid UTC fields throw std::invalid_argument.
class MMoonPosition
{
public:
    /// @brief Calculate lunar direction, distance and phase for an observer.
    /// @param observer WGS-84 location, including ellipsoidal height.
    /// @param utc Valid UTC calendar date and time.
    /// @return Topocentric direction/distance and geocentric phase quantities.
    /// @throws std::invalid_argument If UTC calendar fields are invalid.
    static MMoonHorizontalPosition calculate(const MGeoPoint& observer, const MDateTimeUtc& utc);
};
