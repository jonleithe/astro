/**
 * @file planet_position.cpp
 * @brief Keplerian planet orbits and observer-relative horizontal geometry.
 */
#include "planet_position.h"

#include <cmath>
#include <stdexcept>

namespace {
/// @brief Radians per degree.
constexpr double rad = 3.14159265358979323846 / 180.0;
/// @brief Metres per astronomical unit.
constexpr double au = 149597870700.0;

/// @brief Orbital elements at the observation date; angles are radians.
struct Elements
{
    double node; ///< Ascending-node longitude.
    double inclination; ///< Orbital inclination.
    double perihelion; ///< Argument of perihelion.
    double axis; ///< Semi-major axis in AU.
    double eccentricity; ///< Orbital eccentricity.
    double anomaly; ///< Mean anomaly.
};



/// @brief Convert orbital elements to heliocentric ecliptic coordinates.
/// @param orbit Elements for the observation date.
/// @return Cartesian vector in AU, equinox of date.
MEcefPoint orbit_vector(const Elements& orbit)
{
    double mean = std::remainder(orbit.anomaly, 2 * 3.14159265358979323846);
    double eccentric = mean;
    for(int iteration = 0; iteration < 15; ++iteration){
        double correction = (eccentric - orbit.eccentricity * std::sin(eccentric) - mean)
                          / (1 - orbit.eccentricity * std::cos(eccentric));
        eccentric -= correction;
        if(std::abs(correction) < 1e-12) break;
    }
    double x = orbit.axis * (std::cos(eccentric) - orbit.eccentricity);
    double y = orbit.axis * std::sqrt(1 - orbit.eccentricity * orbit.eccentricity) * std::sin(eccentric);
    double argument = std::atan2(y, x) + orbit.perihelion;
    double radius = std::hypot(x, y);
    return {radius * (std::cos(orbit.node) * std::cos(argument)
                   - std::sin(orbit.node) * std::sin(argument) * std::cos(orbit.inclination)),
            radius * (std::sin(orbit.node) * std::cos(argument)
                   + std::cos(orbit.node) * std::sin(argument) * std::cos(orbit.inclination)),
            radius * std::sin(argument) * std::sin(orbit.inclination)};
} // ——— END OF orbit_vector() —————————————————————————————————————————————————



/// @brief Evaluate Schlyter's orbital elements at the observation date.
/// @param planet Supported planet identifier.
/// @param d Days since 1999-12-31 00:00 UTC.
/// @return Orbital elements with angles converted to radians.
/// @throws std::invalid_argument If planet is unsupported.
Elements elements(MPlanet planet, double d)
{
    // https://stjarnhimlen.se/comp/ppcomp.html, orbital elements section.
    switch(planet){
        case MPlanet::Mercury:
            return {(48.3313 + 3.24587e-5*d)*rad, (7.0047 + 5e-8*d)*rad,
                    (29.1241 + 1.01444e-5*d)*rad, .387098, .205635 + 5.59e-10*d,
                    (168.6562 + 4.0923344368*d)*rad};
        case MPlanet::Venus:
            return {(76.6799 + 2.46590e-5*d)*rad, (3.3946 + 2.75e-8*d)*rad,
                    (54.8910 + 1.38374e-5*d)*rad, .723330, .006773 - 1.302e-9*d,
                    (48.0052 + 1.6021302244*d)*rad};
        case MPlanet::Mars:
            return {(49.5574 + 2.11081e-5*d)*rad, (1.8497 - 1.78e-8*d)*rad,
                    (286.5016 + 2.92961e-5*d)*rad, 1.523688, .093405 + 2.516e-9*d,
                    (18.6021 + .5240207766*d)*rad};
    }
    throw std::invalid_argument("unsupported planet identifier");
} // ——— END OF elements() —————————————————————————————————————————————————————
} // namespace



/// @brief Calculate a planet's direction and distance.
/// @param planet Supported planet identifier.
/// @param observer WGS-84 observer, including ellipsoidal height.
/// @param utc Valid UTC date and time.
/// @return Topocentric horizontal coordinates and distance in metres.
/// @throws std::invalid_argument If planet or UTC fields are invalid.
MPlanetHorizontalPosition MPlanetPosition::calculate(MPlanet planet, const MGeoPoint& observer,
                                                    const MDateTimeUtc& utc)
{
    double jd = MSunPosition::calculate(observer, utc).julian_day;
    double d = jd - 2451543.5;
    auto position = orbit_vector(elements(planet, d));
    // The solar orbit vector is geocentric, so adding it subtracts Earth's
    // heliocentric position in the same equinox-of-date ecliptic frame.
    auto sun = orbit_vector({0, 0, (282.9404 + 4.70935e-5*d)*rad, 1,
                             .016709 - 1.151e-9*d, (356.0470 + .9856002585*d)*rad});
    double x = (position.x + sun.x) * au;
    double y = (position.y + sun.y) * au;
    double z = position.z * au;
    double obliquity = (23.4393 - 3.563e-7*d)*rad;
    double eq_y = y * std::cos(obliquity) - z * std::sin(obliquity);
    double eq_z = y * std::sin(obliquity) + z * std::cos(obliquity);
    double t = (jd - 2451545.0) / 36525.0;
    double sidereal = std::remainder(280.46061837 + 360.98564736629*(jd - 2451545.0)
                      + .000387933*t*t - t*t*t/38710000.0, 360.0)*rad;
    const MEcefPoint ecef{x*std::cos(sidereal) + eq_y*std::sin(sidereal),
                         -x*std::sin(sidereal) + eq_y*std::cos(sidereal), eq_z};
    auto enu = MObserverVector::from_ecef(observer, ecef);
    double azimuth = std::atan2(enu.east, enu.north)/rad;
    if(azimuth < 0) azimuth += 360;
    return {azimuth, std::atan2(enu.up, std::hypot(enu.east, enu.north))/rad, enu.magnitude()};
} // ——— END OF MPlanetPosition::calculate() ———————————————————————————————————



/// @brief Get the planet's display name.
/// @param planet Supported planet identifier.
/// @return Static English name.
/// @throws std::invalid_argument If planet is unsupported.
const char* MPlanetPosition::name(MPlanet planet)
{
    switch(planet){
        case MPlanet::Mercury: return "Mercury";
        case MPlanet::Venus: return "Venus";
        case MPlanet::Mars: return "Mars";
    }
    throw std::invalid_argument("unsupported planet identifier");
} // ——— END OF MPlanetPosition::name() ————————————————————————————————————————



/// @brief Get a distinct ASCII sky marker.
/// @param planet Supported planet identifier.
/// @return 'm' for Mercury, 'V' for Venus or 'M' for Mars.
/// @throws std::invalid_argument If planet is unsupported.
char MPlanetPosition::marker(MPlanet planet)
{
    switch(planet){
        case MPlanet::Mercury: return 'm';
        case MPlanet::Venus: return 'V';
        case MPlanet::Mars: return 'M';
    }
    throw std::invalid_argument("unsupported planet identifier");
} // ——— END OF MPlanetPosition::marker() ——————————————————————————————————————
