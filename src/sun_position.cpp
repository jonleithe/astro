/*
 * File: sun_position.cpp
 * Created: 2026-04-25
 * Author: jole
 * Description: Meeus/NOAA-style solar position calculations for converting
 *              UTC date/time and observer coordinates into horizontal sky
 *              coordinates for the Sun.
 */

#include "sun_position.h"

#include <cmath>
#include <stdexcept>


bool MSunPosition::is_leap_year(int year) noexcept
{
    return ((year % 4) == 0 && (year % 100) != 0) || ((year % 400) == 0);
} // ——— END OF MSunPosition::is_leap_year() ———————————————————————————————————


bool MSunPosition::is_valid_utc(const MDateTimeUtc& utc) noexcept
{
    static constexpr short days_per_month[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    if (utc.month < 1 || utc.month > 12)
        return false;
    if (utc.hour < 0 || utc.hour > 23)
        return false;
    if (utc.minute < 0 || utc.minute > 59)
        return false;
    if (utc.second < 0 || utc.second > 59)
        return false;

    short max_day = days_per_month[utc.month - 1];
    if (utc.month == 2 && is_leap_year(utc.year))
        max_day = 29;

    return utc.day >= 1 && utc.day <= max_day;
} // ——— END OF MSunPosition::is_valid_utc() ———————————————————————————————————


double MSunPosition::to_radians(double deg) noexcept
{
    return deg * (kPi / 180.0);
} // ——— END OF MSunPosition::to_radians() —————————————————————————————————————


double MSunPosition::to_degrees(double rad) noexcept
{
    return rad * (180.0 / kPi);
} // ——— END OF MSunPosition::to_degrees() —————————————————————————————————————


double MSunPosition::normalize_degrees(double deg) noexcept
{
    double normalized = std::fmod(deg, 360.0);

    if (normalized < 0.0)
        normalized += 360.0;

    return normalized;
} // ——— END OF MSunPosition::normalize_degrees() ——————————————————————————————


double MSunPosition::normalize_signed_degrees(double deg) noexcept
{
    double normalized = normalize_degrees(deg);

    if (normalized >= 180.0)
        normalized -= 360.0;

    return normalized;
} // ——— END OF MSunPosition::normalize_signed_degrees() ———————————————————————


int MSunPosition::day_of_year(const MDateTimeUtc& utc) noexcept
{
    static constexpr int days_before_month[12] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };

    int doy = days_before_month[utc.month - 1] + utc.day;
    if (utc.month > 2 && is_leap_year(utc.year))
        doy += 1;

    return doy;
} // ——— END OF MSunPosition::day_of_year() ————————————————————————————————————


double MSunPosition::calculate_julian_day(const MDateTimeUtc& utc) noexcept
{
    int year = utc.year;
    int month = utc.month;

    if (month <= 2)
    {
        year -= 1;
        month += 12;
    }

    const int a = static_cast<int>(std::floor(year / 100.0));
    const int b = 2 - a + static_cast<int>(std::floor(a / 4.0));
    const double day_fraction = (utc.hour + (utc.minute / 60.0) + (utc.second / 3600.0)) / 24.0;

    return std::floor(365.25 * (year + 4716.0))
         + std::floor(30.6001 * (month + 1.0))
         + utc.day + b - 1524.5 + day_fraction;
} // ——— END OF MSunPosition::calculate_julian_day() ———————————————————————————


MSunHorizontalPosition MSunPosition::calculate(const MGeoPoint& observer, const MDateTimeUtc& utc)
{
    if (!is_valid_utc(utc))
        throw std::invalid_argument("MSunPosition: invalid UTC date/time supplied");

    const double julian_day = calculate_julian_day(utc);
    const double t = (julian_day - kJ2000) / 36525.0;

    const double mean_longitude = normalize_degrees(280.46646 + t * (36000.76983 + t * 0.0003032));
    const double mean_anomaly = normalize_degrees(357.52911 + t * (35999.05029 - t * 0.0001537));
    const double eccentricity = 0.016708634 - t * 0.000042037 - t * t * 0.0000001267;

    const double mean_anomaly_rad = to_radians(mean_anomaly);
    const double equation_of_center =
          (1.914602 - t * (0.004817 + 0.000014 * t)) * std::sin(mean_anomaly_rad)
        + (0.019993 - 0.000101 * t) * std::sin(2.0 * mean_anomaly_rad)
        + 0.000289 * std::sin(3.0 * mean_anomaly_rad);

    const double true_longitude = mean_longitude + equation_of_center;
    const double omega = 125.04 - 1934.136 * t;
    const double apparent_longitude = true_longitude - 0.00569 - 0.00478 * std::sin(to_radians(omega));

    const double mean_obliquity = 23.0
                                + (26.0 + (21.448 - t * (46.815 + t * (0.00059 - 0.001813 * t))) / 60.0) / 60.0;
    const double obliquity = mean_obliquity + 0.00256 * std::cos(to_radians(omega));

    const double apparent_longitude_rad = to_radians(apparent_longitude);
    const double obliquity_rad = to_radians(obliquity);

    const double right_ascension = normalize_degrees(to_degrees(std::atan2(
        std::cos(obliquity_rad) * std::sin(apparent_longitude_rad),
        std::cos(apparent_longitude_rad))));

    const double declination = to_degrees(std::asin(
        std::sin(obliquity_rad) * std::sin(apparent_longitude_rad)));

    const double gmst = normalize_degrees(
        280.46061837
        + 360.98564736629 * (julian_day - kJ2000)
        + 0.000387933 * t * t
        - (t * t * t) / 38710000.0);

    const double local_sidereal_time = normalize_degrees(gmst + observer.get_lon());
    const double hour_angle = normalize_signed_degrees(local_sidereal_time - right_ascension);

    const double latitude_rad = to_radians(observer.get_lat());
    const double declination_rad = to_radians(declination);
    const double hour_angle_rad = to_radians(hour_angle);

    const double altitude = to_degrees(std::asin(
        std::sin(latitude_rad) * std::sin(declination_rad)
        + std::cos(latitude_rad) * std::cos(declination_rad) * std::cos(hour_angle_rad)));

    const double azimuth = normalize_degrees(to_degrees(std::atan2(
        std::sin(hour_angle_rad),
        std::cos(hour_angle_rad) * std::sin(latitude_rad)
        - std::tan(declination_rad) * std::cos(latitude_rad))) + 180.0);

    (void)eccentricity;

    return MSunHorizontalPosition{
        azimuth,
        altitude,
        right_ascension,
        declination,
        hour_angle,
        julian_day
    };
} // ——— END OF MSunPosition::calculate() ——————————————————————————————————————


MSunDailyEventsUtc MSunPosition::calculate_daily_events(const MGeoPoint& observer, const MDateTimeUtc& utc_date)
{
    MDateTimeUtc midnight = utc_date;
    midnight.hour = 0;
    midnight.minute = 0;
    midnight.second = 0;

    if (!is_valid_utc(midnight))
        throw std::invalid_argument("MSunPosition: invalid UTC date supplied for daily events");

    const double latitude_rad = to_radians(observer.get_lat());
    const int doy = day_of_year(midnight);
    const double gamma = 2.0 * kPi / 365.0 * (static_cast<double>(doy) - 1.0);

    const double equation_of_time_minutes = 229.18 * (
          0.000075
        + 0.001868 * std::cos(gamma)
        - 0.032077 * std::sin(gamma)
        - 0.014615 * std::cos(2.0 * gamma)
        - 0.040849 * std::sin(2.0 * gamma));

    const double declination_rad =
          0.006918
        - 0.399912 * std::cos(gamma)
        + 0.070257 * std::sin(gamma)
        - 0.006758 * std::cos(2.0 * gamma)
        + 0.000907 * std::sin(2.0 * gamma)
        - 0.002697 * std::cos(3.0 * gamma)
        + 0.00148 * std::sin(3.0 * gamma);

    const double solar_noon_minutes = 720.0 - 4.0 * observer.get_lon() - equation_of_time_minutes;

    const double cos_ha =
        (std::cos(to_radians(90.833)) / (std::cos(latitude_rad) * std::cos(declination_rad)))
        - std::tan(latitude_rad) * std::tan(declination_rad);

    if (cos_ha < -1.0 || cos_ha > 1.0)
    {
        return MSunDailyEventsUtc{
            false,
            solar_noon_minutes / 60.0,
            -1.0,
            -1.0,
            equation_of_time_minutes,
            to_degrees(declination_rad)
        };
    }

    const double hour_angle_deg = to_degrees(std::acos(cos_ha));
    const double sunrise_minutes = solar_noon_minutes - 4.0 * hour_angle_deg;
    const double sunset_minutes = solar_noon_minutes + 4.0 * hour_angle_deg;

    return MSunDailyEventsUtc{
        true,
        solar_noon_minutes / 60.0,
        sunrise_minutes / 60.0,
        sunset_minutes / 60.0,
        equation_of_time_minutes,
        to_degrees(declination_rad)
    };
} // ——— END OF MSunPosition::calculate_daily_events() —————————————————————————
