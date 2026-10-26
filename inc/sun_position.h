/**
 * @file sun_position.h
 * Created: 2026-04-25
 * Author: jole
 * @brief Public API for calculating the Sun's apparent sky position
 *              for a UTC date/time and an observer on Earth.
 */

#pragma once

#include "geo_point.h"
#include "observer_vector.h"


/// @brief UTC calendar date and time in the proleptic Gregorian calendar.
struct MDateTimeUtc
{
    /// @brief UTC year (Gregorian calendar).
    int year;

    /// @brief UTC month in range [1, 12].
    short month;

    /// @brief UTC day of month.
    short day;

    /// @brief UTC hour in range [0, 23].
    short hour;

    /// @brief UTC minute in range [0, 59].
    short minute;

    /// @brief UTC second in range [0, 59].
    short second;
};


/// @brief Solar horizontal angles and intermediate geocentric quantities.
struct MSunHorizontalPosition
{
    /// @brief Solar azimuth in degrees, clockwise from true north.
    double azimuth_deg;

    /// @brief Solar altitude in degrees above geometric horizon.
    double altitude_deg;

    /// @brief Apparent right ascension in decimal degrees.
    double right_ascension_deg;

    /// @brief Apparent declination in decimal degrees.
    double declination_deg;

    /// @brief Local hour angle in decimal degrees.
    double hour_angle_deg;

    /// @brief Julian day for the requested UTC date/time.
    double julian_day;

    /// @brief Modelled Earth-centre to Sun-centre distance in metres.
    double geocentric_distance_m;
};


/// @brief Solar transit and surrounding horizon-crossing events in UTC.
struct MSunDailyEventsUtc
{
    /// @brief True if sunrise and sunset exist for this date/location.
    bool has_sunrise_sunset;

    /// @brief Solar noon time in UTC decimal hours.
    double solar_noon_utc_hours;

    /// @brief Sunrise time in UTC decimal hours, or -1 when unavailable.
    double sunrise_utc_hours;

    /// @brief Sunset time in UTC decimal hours, or -1 when unavailable.
    double sunset_utc_hours;

    /// @brief Equation of time in minutes.
    double equation_of_time_minutes;

    /// @brief Solar declination at solar noon in decimal degrees.
    double declination_deg;
};


/// @brief Approximate solar position, vectors and daily event calculations.
class MSunPosition
{
    private:
        /// @brief Pi for degree/radian conversion.
        static constexpr double kPi = 3.14159265358979323846;
        /// @brief Julian day of the J2000 epoch.
        static constexpr double kJ2000 = 2451545.0;

        /// @brief Calculate approximate geocentric solar angles for a Julian day.
        /// @param observer WGS-84 observer, including ellipsoidal height.
        /// @param julian_day UTC observation time expressed as Julian day.
        /// @return Solar horizontal coordinates and geocentric intermediate quantities.
        static MSunHorizontalPosition calculate_at_julian_day(const MGeoPoint& observer, double julian_day);

        /// @brief Test whether a Gregorian year is a leap year.
        /// @param year Gregorian calendar year.
        /// @return True if the year has 366 days.
        static bool is_leap_year(int year) noexcept;
        /// @brief Validate Gregorian calendar and UTC clock fields.
        /// @param utc UTC calendar date and time.
        /// @return True if all calendar and clock fields are valid.
        static bool is_valid_utc(const MDateTimeUtc& utc) noexcept;
        /// @brief Convert degrees to radians.
        /// @param deg Input angle in degrees.
        /// @return Angle in radians.
        static double to_radians(double deg) noexcept;
        /// @brief Convert radians to degrees.
        /// @param rad Input angle in radians.
        /// @return Angle in degrees.
        static double to_degrees(double rad) noexcept;
        /// @brief Wrap an angle into [0, 360) degrees.
        /// @param deg Input angle in degrees.
        /// @return Angle in [0, 360) degrees.
        static double normalize_degrees(double deg) noexcept;
        /// @brief Wrap an angle into [-180, 180) degrees.
        /// @param deg Input angle in degrees.
        /// @return Angle in [-180, 180) degrees.
        static double normalize_signed_degrees(double deg) noexcept;
        /// @brief Compute the one-based Gregorian day of year.
        /// @param utc UTC calendar date and time.
        /// @return Day number starting at 1 for January 1.
        static int day_of_year(const MDateTimeUtc& utc) noexcept;
        /// @brief Convert a valid Gregorian UTC timestamp to Julian day.
        /// @param utc UTC calendar date and time.
        /// @return Julian day including the fractional UTC day.
        static double calculate_julian_day(const MDateTimeUtc& utc) noexcept;

    public:
        /*
         * API contract (v1)
         * - Input time is interpreted as UTC in the proleptic Gregorian calendar.
         * - Observer coordinates are taken from MGeoPoint in decimal degrees/metres.
         * - Returned azimuth is degrees clockwise from true north in [0, 360).
         * - Returned altitude is degrees above the geometric horizon.
         * - Throws std::invalid_argument if the UTC calendar fields are invalid.
         */
        
        /// @brief Calculate the Sun's apparent horizontal sky position.
        /// @param observer Observer geodetic coordinates.
        /// @param utc UTC date and time.
        /// @return Apparent solar horizontal coordinates and selected intermediates.
        /// @throws std::invalid_argument If UTC calendar values are invalid.
        static MSunHorizontalPosition calculate(const MGeoPoint& observer, const MDateTimeUtc& utc);

        /// @brief Observer-to-Sun-centre vector in local ENU, metres.
        /// @note Includes observer height and solar parallax. Uses the existing
        /// apparent solar direction and approximate orbital radius, not a precision
        /// geometric ephemeris; no atmospheric refraction or terrain is applied.
        /// @throws std::invalid_argument If UTC calendar values are invalid.
        /// @param observer WGS-84 location, including ellipsoidal height.
        /// @param utc Valid UTC calendar date and time.
        /// @return Observer-to-Sun-centre ENU vector in metres.
        static MObserverVector calculate_vector(const MGeoPoint& observer, const MDateTimeUtc& utc);

        /// @brief Calculate the observer-relative geometric Sun-centre direction.
        /// @param observer WGS-84 location, including ellipsoidal height.
        /// @param utc Valid UTC date and time.
        /// @return Azimuth/elevation in degrees and slant distance in metres.
        /// @throws std::invalid_argument If UTC fields are invalid.
        /// @note Derived from calculate_vector, including height and parallax;
        /// no atmospheric refraction or terrain is applied.
        static MHorizontalDirection calculate_topocentric(const MGeoPoint& observer,
                                                          const MDateTimeUtc& utc);



        /*
         * Calculates UTC solar noon and the surrounding sunrise/sunset pair.
         * - Input date uses year/month/day and ignores hour/minute/second.
         * - Sunrise/sunset are for apparent upper-limb crossing (90.833 deg zenith).
         * - Uses the detailed solar-position model with numerical horizon crossings.
         * - Events belong to the solar day around this date's upper transit;
         *   decimal UTC hours can be outside [0, 24) near the date line.
         * - If a complete pair does not exist, has_sunrise_sunset is false.
         * - Uses a fixed standard horizon; observer elevation/terrain are not applied.
         */
        /// @brief Calculate daily solar events for a UTC date and observer position.
        /// @param observer Observer geodetic coordinates.
        /// @param utc_date UTC calendar date (time fields are ignored).
        /// @return Daily solar-noon and sunrise/sunset information in UTC.
        /// @throws std::invalid_argument If UTC calendar values are invalid.
        static MSunDailyEventsUtc calculate_daily_events(const MGeoPoint& observer, const MDateTimeUtc& utc_date);
};
