/*
 * File: geo_point.h
 * Created: 2026-04-25
 * Author: jole
 * Description: Declaration of MGeoPoint, a geodetic coordinate class
 *              representing a location on Earth's surface (WGS-84 ellipsoid).
 *              Latitude and longitude are in decimal degrees; altitude in metres.
 *              Provides explicit conversions to/from ECEF Cartesian coordinates.
 */

#pragma once


struct MEcefPoint
{
    double x;
    double y;
    double z;
};



class MGeoPoint
{
    public:
        // WGS-84 defining parameters
        static constexpr double kWGS84_a  = 6378137.0;           // semi-major axis, metres
        static constexpr double kWGS84_f  = 1.0 / 298.257223563; // flattening
        static constexpr double kWGS84_b  = kWGS84_a * (1.0 - kWGS84_f); // semi-minor axis
        static constexpr double kWGS84_e2 = 2.0 * kWGS84_f - kWGS84_f * kWGS84_f; // first eccentricity squared

    private:
        static constexpr double kPi = 3.14159265358979323846;

        double lat_;  // geodetic latitude,  decimal degrees  [-90,  +90]
        double lon_;  // geodetic longitude, decimal degrees  [-180, +180]
        double alt_;  // altitude above WGS-84 ellipsoid, metres

    public:
        MGeoPoint(double lat_deg, double lon_deg, double alt_m = 0.0);

        inline double get_lat(void) const noexcept { return lat_; }
        inline double get_lon(void) const noexcept { return lon_; }
        inline double get_alt(void) const noexcept { return alt_; }

        // Convert to Earth-Centered Earth-Fixed (ECEF) Cartesian coordinates, metres.
        MEcefPoint to_ecef(void) const noexcept;

        // Construct an MGeoPoint from ECEF Cartesian coordinates (metres).
        // Uses Bowring's iterative method for geodetic latitude.
        static MGeoPoint from_ecef(double x_m, double y_m, double z_m);

        // Great-circle (spherical) distance between two geographic points, metres.
        // Uses the Haversine formula with WGS-84 mean radius.
        static double haversine_distance(const MGeoPoint& a, const MGeoPoint& b) noexcept;

    private:
        static inline double to_radians(double deg) noexcept { return deg * (kPi / 180.0); }
        static inline double to_degrees(double rad) noexcept { return rad * (180.0 / kPi); }
};

