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
    /// @brief ECEF X coordinate in metres.
    double x;
    /// @brief ECEF Y coordinate in metres.
    double y;
    /// @brief ECEF Z coordinate in metres.
    double z;
};



class MGeoPoint
{
    private:
        // Internal WGS-84 constants used by conversion algorithms.
        static constexpr double kWGS84_a    = 6378137.0;
        static constexpr double kWGS84_f    = 1.0 / 298.257223563;
        static constexpr double kWGS84_e2   = 2.0 * kWGS84_f - kWGS84_f * kWGS84_f;
        static constexpr double kPi         = 3.14159265358979323846;

        double lat_;  // geodetic latitude,  decimal degrees  [-90,  +90]
        double lon_;  // geodetic longitude, decimal degrees  [-180, +180]
        double alt_;  // altitude above WGS-84 ellipsoid, metres

    public:
        /*
         * API contract (v1)
         * - Units: latitude/longitude in decimal degrees, altitude in metres,
         *   ECEF x/y/z in metres, distances in metres.
         * - Valid constructor inputs: latitude in [-90, +90], longitude in
         *   [-180, +180], altitude as any finite double value.
         * - Error behavior: constructor throws std::domain_error for invalid
         *   latitude/longitude ranges.
         * - Numerical behavior: from_ecef(to_ecef(p)) is expected to be close
         *   to p within floating-point tolerance, not bit-identical.
         */
        /// @brief Construct a geodetic point in WGS-84 coordinates.
        /// @param lat_deg Latitude in decimal degrees.
        /// @param lon_deg Longitude in decimal degrees.
        /// @param alt_m Altitude above ellipsoid in metres.
        /// @throws std::domain_error If latitude/longitude are out of range.
        MGeoPoint(double lat_deg, double lon_deg, double alt_m = 0.0);

        /// @brief Get geodetic latitude in decimal degrees.
        inline double get_lat(void) const noexcept { return lat_; }
        /// @brief Get geodetic longitude in decimal degrees.
        inline double get_lon(void) const noexcept { return lon_; }
        /// @brief Get altitude above WGS-84 ellipsoid in metres.
        inline double get_alt(void) const noexcept { return alt_; }

        /// @brief Convert geodetic coordinates to ECEF Cartesian coordinates.
        /// @return ECEF coordinates in metres.
        MEcefPoint to_ecef(void) const noexcept;

        /// @brief Construct geodetic coordinates from ECEF Cartesian coordinates.
        /// @param x_m ECEF X coordinate in metres.
        /// @param y_m ECEF Y coordinate in metres.
        /// @param z_m ECEF Z coordinate in metres.
        /// @return Geodetic point in WGS-84 coordinates.
        /// @note Uses Bowring's iterative method for geodetic latitude.
        static MGeoPoint from_ecef(double x_m, double y_m, double z_m);

        /// @brief Great-circle distance between two geodetic points.
        /// @param a First geodetic point.
        /// @param b Second geodetic point.
        /// @return Great-circle distance in metres.
        /// @note Uses the Haversine formula with WGS-84 mean Earth radius.
        static double haversine_distance(const MGeoPoint& a, const MGeoPoint& b) noexcept;

    private:
        static inline double to_radians(double deg) noexcept { return deg * (kPi / 180.0); }
        static inline double to_degrees(double rad) noexcept { return rad * (180.0 / kPi); }
};

