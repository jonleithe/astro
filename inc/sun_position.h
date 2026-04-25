/*
 * File: sun_position.h
 * Created: 2026-04-25
 * Author: jole
 * Description: Public API for calculating the Sun's apparent sky position
 *              for a UTC date/time and an observer on Earth.
 */

#pragma once

#include "geo_point.h"


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
};


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
    /// @brief Solar declination in decimal degrees.
    double declination_deg;
};


class MSunPosition
{
    private:
        static constexpr double kPi = 3.14159265358979323846;
        static constexpr double kJ2000 = 2451545.0;

        static bool is_leap_year(int year) noexcept;
        static bool is_valid_utc(const MDateTimeUtc& utc) noexcept;
        static double to_radians(double deg) noexcept;
        static double to_degrees(double rad) noexcept;
        static double normalize_degrees(double deg) noexcept;
        static double normalize_signed_degrees(double deg) noexcept;
        static int day_of_year(const MDateTimeUtc& utc) noexcept;
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

        /*
         * Calculates UTC solar noon and sunrise/sunset for a UTC calendar date.
         * - Input date uses year/month/day and ignores hour/minute/second.
         * - Sunrise/sunset are for apparent upper-limb crossing (90.833 deg zenith).
         * - If no sunrise/sunset exists for the date/latitude, has_sunrise_sunset is false.
         */
        /// @brief Calculate daily solar events for a UTC date and observer position.
        /// @param observer Observer geodetic coordinates.
        /// @param utc_date UTC calendar date (time fields are ignored).
        /// @return Daily solar-noon and sunrise/sunset information in UTC.
        /// @throws std::invalid_argument If UTC calendar values are invalid.
        static MSunDailyEventsUtc calculate_daily_events(const MGeoPoint& observer, const MDateTimeUtc& utc_date);
};
