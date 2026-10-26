/**
 * @file planet_position.h
 * @brief Approximate observer-relative positions of the nearby planets.
 */
#pragma once

#include "sun_position.h"

/// @brief Supported planets in fixed display order.
enum class MPlanet { Mercury, Venus, Mars };

/// @brief Planet centre direction and distance from the observer.
struct MPlanetHorizontalPosition
{
    double azimuth_deg; ///< Degrees clockwise from true north, [0, 360).
    double altitude_deg; ///< Signed degrees above the geometric horizon.
    double distance_m; ///< Observer-to-planet-centre distance in metres.
};

/// @brief Approximate Keplerian planetary positions, equinox of date.
/// @note Uses Schlyter elements near 1900-2100; includes WGS-84 observer
/// parallax/height, but omits light-time, aberration, nutation, refraction,
/// terrain and TT/UT differences. Not a precision ephemeris.
class MPlanetPosition
{
public:
    /// @brief Calculate a planet's direction and distance.
    /// @param planet Supported planet identifier.
    /// @param observer WGS-84 observer, including ellipsoidal height.
    /// @param utc Valid UTC date and time.
    /// @return Topocentric horizontal coordinates and distance in metres.
    /// @throws std::invalid_argument If planet or UTC fields are invalid.
    static MPlanetHorizontalPosition calculate(MPlanet planet, const MGeoPoint& observer,
                                               const MDateTimeUtc& utc);

    /// @brief Get the planet's display name.
    /// @param planet Supported planet identifier.
    /// @return Static English name.
    /// @throws std::invalid_argument If planet is unsupported.
    static const char* name(MPlanet planet);

    /// @brief Get a distinct ASCII sky marker.
    /// @param planet Supported planet identifier.
    /// @return 'm' for Mercury, 'V' for Venus or 'M' for Mars.
    /// @throws std::invalid_argument If planet is unsupported.
    static char marker(MPlanet planet);
};
