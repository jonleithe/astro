/*
 * File: geo_point.cpp
 * Created: 2026-04-25
 * Author: jole
 * Description: Implementation of MGeoPoint geodetic coordinate class.
 *              ECEF conversions follow WGS-84. Haversine distance uses the
 *              WGS-84 mean radius (6371008.8 m).
 */

#include <cmath>
#include <stdexcept>
#include <limits>

#include "geo_point.h"



MGeoPoint::MGeoPoint(double lat_deg, double lon_deg, double alt_m)
    : lat_(lat_deg), lon_(lon_deg), alt_(alt_m)
{
    if (lat_deg < -90.0 || lat_deg > 90.0)
        throw std::domain_error("MGeoPoint: latitude must be in [-90, +90] degrees");
    if (lon_deg < -180.0 || lon_deg > 180.0)
        throw std::domain_error("MGeoPoint: longitude must be in [-180, +180] degrees");
} // ——— END OF MGeoPoint::MGeoPoint() —————————————————————————————————————————



MEcefPoint MGeoPoint::to_ecef(void) const noexcept
{
    const double phi    = to_radians(lat_);
    const double lambda = to_radians(lon_);

    // Prime vertical radius of curvature
    const double N = kWGS84_a / std::sqrt(1.0 - kWGS84_e2 * std::sin(phi) * std::sin(phi));

    const double x = (N + alt_) * std::cos(phi) * std::cos(lambda);
    const double y = (N + alt_) * std::cos(phi) * std::sin(lambda);
    const double z = (N * (1.0 - kWGS84_e2) + alt_) * std::sin(phi);

    return MEcefPoint{x, y, z};
} // ——— END OF MGeoPoint::to_ecef() ———————————————————————————————————————————



MHorizontalDirection MGeoPoint::direction_to(const MGeoPoint& target) const noexcept
{
    const auto observer_ecef = to_ecef();
    const auto target_ecef = target.to_ecef();
    const double dx = target_ecef.x - observer_ecef.x;
    const double dy = target_ecef.y - observer_ecef.y;
    const double dz = target_ecef.z - observer_ecef.z;
    const double distance = std::hypot(std::hypot(dx, dy), dz);
    if (distance == 0.0)
    {
        const double undefined = std::numeric_limits<double>::quiet_NaN();
        return {undefined, undefined, 0.0};
    }

    // Equal latitude/longitude places both points along the ellipsoid normal.
    if (lat_ == target.lat_ && lon_ == target.lon_)
        return {0.0, target.alt_ > alt_ ? 90.0 : -90.0, distance};

    const double phi = to_radians(lat_);
    const double lambda = to_radians(lon_);
    const double east = -std::sin(lambda) * dx + std::cos(lambda) * dy;
    const double north = -std::sin(phi) * std::cos(lambda) * dx
                       - std::sin(phi) * std::sin(lambda) * dy
                       + std::cos(phi) * dz;
    const double up = std::cos(phi) * std::cos(lambda) * dx
                    + std::cos(phi) * std::sin(lambda) * dy
                    + std::sin(phi) * dz;
    double azimuth = to_degrees(std::atan2(east, north));
    if (azimuth < 0.0)
        azimuth += 360.0;
    if (azimuth >= 360.0)
        azimuth = 0.0;
    return {azimuth, to_degrees(std::atan2(up, std::hypot(east, north))), distance};
}


MGeoPoint MGeoPoint::from_ecef(double x_m, double y_m, double z_m)
{
    const double p = std::sqrt(x_m * x_m + y_m * y_m);
    const double lon_rad = std::atan2(y_m, x_m);

    // Bowring's iterative method for geodetic latitude
    double lat_rad = std::atan2(z_m, p * (1.0 - kWGS84_e2));

    for (int i = 0; i < 10; ++i)
    {
        const double sin_lat = std::sin(lat_rad);
        const double N = kWGS84_a / std::sqrt(1.0 - kWGS84_e2 * sin_lat * sin_lat);
        lat_rad = std::atan2(z_m + kWGS84_e2 * N * sin_lat, p);
    }

    const double sin_lat = std::sin(lat_rad);
    const double N = kWGS84_a / std::sqrt(1.0 - kWGS84_e2 * sin_lat * sin_lat);
    const double alt_m = p / std::cos(lat_rad) - N;

    return MGeoPoint(to_degrees(lat_rad), to_degrees(lon_rad), alt_m);
} // ——— END OF MGeoPoint::from_ecef() —————————————————————————————————————————



double MGeoPoint::haversine_distance(const MGeoPoint& a, const MGeoPoint& b) noexcept
{
    // WGS-84 mean radius
    static constexpr double R = 6371008.8;

    const double phi1 = to_radians(a.lat_);
    const double phi2 = to_radians(b.lat_);
    const double d_phi    = to_radians(b.lat_ - a.lat_);
    const double d_lambda = to_radians(b.lon_ - a.lon_);

    const double sin_dphi    = std::sin(d_phi    / 2.0);
    const double sin_dlambda = std::sin(d_lambda / 2.0);

    const double h = sin_dphi * sin_dphi
                   + std::cos(phi1) * std::cos(phi2) * sin_dlambda * sin_dlambda;

    return 2.0 * R * std::asin(std::sqrt(h));
} // ——— END OF MGeoPoint::haversine_distance() ————————————————————————————————
