/**
 * @file moon_position.cpp
 * @brief Approximate lunar orbit, observer parallax and phase calculations.
 */

#include "moon_position.h"

#include <cmath>
#include <algorithm>

namespace {
/// @brief Radians per degree.
constexpr double rad = 3.14159265358979323846 / 180.0;
/// @brief Calculate the sine of an angle in degrees.
/// @param x Input angle.
/// @return Dimensionless sine value.
double sind(double x) { return std::sin(x * rad); }
/// @brief Calculate the cosine of an angle in degrees.
/// @param x Input angle.
/// @return Dimensionless cosine value.
double cosd(double x) { return std::cos(x * rad); }
/// @brief Convert radians to degrees.
/// @param x Input angle.
/// @return Angle in degrees.
double degrees(double x) { return x / rad; }
/// @brief Wrap an angle into [0, 360) degrees.
/// @param x Input angle.
/// @return Angle in [0, 360) degrees.
double wrap(double x) { return x - 360.0 * std::floor(x / 360.0); }
}

/// @brief Calculate lunar direction, distance and phase for an observer.
/// @param observer WGS-84 location, including ellipsoidal height.
/// @param utc Valid UTC calendar date and time.
/// @return Topocentric direction/distance and geocentric phase quantities.
/// @throws std::invalid_argument If UTC calendar fields are invalid.
MMoonHorizontalPosition MMoonPosition::calculate(const MGeoPoint& observer, const MDateTimeUtc& utc)
{
    // Reuse the validated Gregorian UTC -> Julian day conversion.
    const double jd = MSunPosition::calculate(observer, utc).julian_day;
    const double d = jd - 2451543.5;
    // Orbital elements and perturbations: Paul Schlyter, sections 4, 6-9:
    // https://stjarnhimlen.se/comp/ppcomp.html
    const double node = wrap(125.1228 - 0.0529538083 * d);
    const double peri = wrap(318.0634 + 0.1643573223 * d);
    const double mean = wrap(115.3654 + 13.0649929509 * d);
    constexpr double eccentricity = 0.054900;
    double anomaly = mean * rad;
    for(int iteration = 0; iteration < 10; ++iteration){
        double correction = (anomaly - eccentricity * std::sin(anomaly) - mean * rad)
                          / (1 - eccentricity * std::cos(anomaly));
        anomaly -= correction;
        if(std::abs(correction) < 1e-12) break;
    }
    double x = 60.2666 * (std::cos(anomaly) - eccentricity);
    double y = 60.2666 * std::sqrt(1 - eccentricity * eccentricity) * std::sin(anomaly);
    double distance = std::hypot(x, y);
    const double argument = degrees(std::atan2(y, x)) + peri;
    x = distance * (cosd(node) * cosd(argument) - sind(node) * sind(argument) * cosd(5.1454));
    y = distance * (sind(node) * cosd(argument) + cosd(node) * sind(argument) * cosd(5.1454));
    double z = distance * sind(argument) * sind(5.1454);
    double longitude = degrees(std::atan2(y, x));
    double latitude = degrees(std::atan2(z, std::hypot(x, y)));
    const double solar_mean = wrap(356.0470 + 0.9856002585 * d);
    const double solar_peri = 282.9404 + 4.70935e-5 * d;
    const double elongation = mean + peri + node - solar_mean - solar_peri;
    const double f = mean + peri;
    longitude += -1.274 * sind(mean - 2 * elongation) + 0.658 * sind(2 * elongation)
        - 0.186 * sind(solar_mean) - 0.059 * sind(2 * mean - 2 * elongation)
        - 0.057 * sind(mean - 2 * elongation + solar_mean) + 0.053 * sind(mean + 2 * elongation)
        + 0.046 * sind(2 * elongation - solar_mean) + 0.041 * sind(mean - solar_mean)
        - 0.035 * sind(elongation) - 0.031 * sind(mean + solar_mean)
        - 0.015 * sind(2 * f - 2 * elongation) + 0.011 * sind(mean - 4 * elongation);
    latitude += -0.173 * sind(f - 2 * elongation) - 0.055 * sind(mean - f - 2 * elongation)
        - 0.046 * sind(mean + f - 2 * elongation) + 0.033 * sind(f + 2 * elongation)
        + 0.017 * sind(2 * mean + f);
    distance += -0.58 * cosd(mean - 2 * elongation) - 0.46 * cosd(2 * elongation);
    distance *= 6378137.0; // Earth equatorial radii -> metres.
    x = distance * cosd(longitude) * cosd(latitude);
    y = distance * sind(longitude) * cosd(latitude);
    z = distance * sind(latitude);
    // Solar true longitude from the same approximate orbital model.
    const double solar_e = 0.016709 - 1.151e-9 * d;
    const double solar_anomaly = solar_mean * rad
        + solar_e * sind(solar_mean) * (1 + solar_e * cosd(solar_mean));
    const double solar_x = std::cos(solar_anomaly) - solar_e;
    const double solar_y = std::sqrt(1 - solar_e * solar_e) * std::sin(solar_anomaly);
    const double solar_longitude = degrees(std::atan2(solar_y, solar_x)) + solar_peri;
    const double cycle = wrap(longitude - solar_longitude);
    const double cos_elongation = std::clamp(cosd(cycle) * cosd(latitude), -1.0, 1.0);
    // Earth-Moon-Sun triangle; retain finite solar distance for phase angle.
    const double solar_distance = std::hypot(solar_x, solar_y) * 149597870700.0;
    const double moon_sun_distance = std::sqrt(solar_distance * solar_distance
        + distance * distance - 2 * solar_distance * distance * cos_elongation);
    const double cos_phase = std::clamp((distance - solar_distance * cos_elongation)
                                        / moon_sun_distance, -1.0, 1.0);
    const double illumination = (1 + cos_phase) / 2;
    const double obliquity = 23.4393 - 3.563e-7 * d;
    const double equatorial_y = y * cosd(obliquity) - z * sind(obliquity);
    const double equatorial_z = y * sind(obliquity) + z * cosd(obliquity);
    const double t = (jd - 2451545.0) / 36525.0;
    const double sidereal = wrap(280.46061837 + 360.98564736629 * (jd - 2451545.0)
                              + 0.000387933 * t * t - t * t * t / 38710000.0);
    // Rotate equatorial-of-date into Earth-fixed axes, then subtract observer.
    const MEcefPoint target{x * cosd(sidereal) + equatorial_y * sind(sidereal),
                           -x * sind(sidereal) + equatorial_y * cosd(sidereal), equatorial_z};
    const auto enu = MObserverVector::from_ecef(observer, target);
    return {wrap(degrees(std::atan2(enu.east, enu.north))),
            degrees(std::atan2(enu.up, std::hypot(enu.east, enu.north))), enu.magnitude(),
            illumination, cycle, cycle < 180.0};
}

/// @brief Classify the lunar cycle using the documented primary-phase bands.
/// @return Static name for one of the eight display phases.
const char* MMoonHorizontalPosition::phase_name() const noexcept
{
    if(phase_cycle_deg < 5 || phase_cycle_deg >= 355) return "New Moon";
    if(phase_cycle_deg < 85) return "Waxing crescent";
    if(phase_cycle_deg < 95) return "First quarter";
    if(phase_cycle_deg < 175) return "Waxing gibbous";
    if(phase_cycle_deg < 185) return "Full Moon";
    if(phase_cycle_deg < 265) return "Waning gibbous";
    if(phase_cycle_deg < 275) return "Last quarter";
    return "Waning crescent";
}
