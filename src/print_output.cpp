/**
 * @file print_output.cpp
 * Created: 2026-04-25
 * Author: jole
 * @brief CLI usage and formatted output helpers for the astro executable.
 */

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

#include "print_output.h"

namespace {

/// @brief Locale facet for space-separated groups of three numeric digits.
class DigitGrouping : public std::numpunct<char> {
protected:
    /// @brief Select the space character for digit grouping.
    /// @return A single space.
    char do_thousands_sep() const override { return ' '; }
    /// @brief Specify repeated groups of three digits.
    /// @return The standard locale grouping pattern for triples.
    std::string do_grouping() const override { return "\3"; }
};

/// @brief Format a fixed-precision value with space-separated digit groups.
/// @param value Value or string to parse/format.
/// @param precision Number of digits after the decimal point.
/// @return Formatted numeric string.
std::string format_number(double value, int precision)
{
    static const std::locale grouped_locale(std::locale::classic(), new DigitGrouping);
    std::ostringstream output;
    output.imbue(grouped_locale);
    output << std::fixed << std::setprecision(precision) << value;
    return output.str();
}

} // namespace



/// @brief Print CLI usage, substituting the executable name.
/// @param argv0 Executable name to show in examples.
void print_usage(const char* argv0)
{
    constexpr std::string_view placeholder = "{program}";
    std::string_view remaining = print_output_text::usage;
    auto position = remaining.find(placeholder);
    while(position != std::string_view::npos){
        std::cout << remaining.substr(0, position) << argv0;
        remaining.remove_prefix(position + placeholder.size());
        position = remaining.find(placeholder);
    }
    std::cout << remaining;
} // ——— END OF print_usage() ——————————————————————————————————————————————————



/// @brief Print an event time rounded to seconds and wrapped into one UTC day.
/// @param label Label printed before the time.
/// @param utc_hours Finite decimal UTC hours; day offsets are discarded for display.
static void print_hhmmss_utc(const char* label, double utc_hours)
{
    double wrapped = std::fmod(utc_hours, 24.0);
    if (wrapped < 0.0)
        wrapped += 24.0;

    int total_seconds = static_cast<int>(wrapped * 3600.0 + 0.5);
    total_seconds %= (24 * 3600);

    int hh  = total_seconds / 3600;
    int mm  = (total_seconds % 3600) / 60;
    int ss  = total_seconds % 60;

    printf("  %s: %02d:%02d:%02d UTC\n", label, hh, mm, ss);
} // ——— END OF print_hhmmss_utc() —————————————————————————————————————————————



/// @brief Test whether a Gregorian year is a leap year.
/// @param year Gregorian calendar year.
/// @return True if the year has 366 days.
static bool is_leap_year(int year)
{
    if(year % 400 == 0)
        return true;
    if(year % 100 == 0)
        return false;
    return (year % 4) == 0;
} // ——— END OF is_leap_year() ———————————————————————————————————————————————



/// @brief Compute the one-based Gregorian day of year.
/// @param utc_time Valid UTC calendar date and time.
/// @return Day number starting at 1 for January 1.
static int day_of_year(const MDateTimeUtc& utc_time)
{
    static const int days_before_month[12] = {0, 31, 59, 90, 120, 151,
                                              181, 212, 243, 273, 304, 334};

    int doy = days_before_month[utc_time.month - 1] + utc_time.day;
    if(utc_time.month > 2 && is_leap_year(utc_time.year))
        doy += 1;

    return doy;
} // ——— END OF day_of_year() ————————————————————————————————————————————————



/// @brief Print observer details, solar position/vector and daily solar events.
/// @param observer WGS-84 observer, including ellipsoidal height.
/// @param observer_name Optional label; nullptr or empty omits it.
/// @param utc_time Valid UTC timestamp matching the supplied solar results.
/// @param sun_now Solar position at utc_time.
/// @param sun_daily Solar events for the selected UTC date.
/// @throws std::invalid_argument If vector calculation rejects utc_time.
void print_output(const MGeoPoint& observer,
                  const char* observer_name,
                  const MDateTimeUtc& utc_time,
                  const MSunHorizontalPosition& sun_now,
                  const MSunDailyEventsUtc& sun_daily)
{
    const int doy = day_of_year(utc_time);

    if(observer_name != nullptr && observer_name[0] != '\0')
        printf("Name     -> %s\n", observer_name);

    printf("Observer -> lat = %.5f deg, lon = %.5f deg, alt = %s m\n",
           observer.get_lat(), observer.get_lon(), format_number(observer.get_alt(), 1).c_str());

    printf("Sun @ %04d-%02d-%02d %02d:%02d:%02d UTC (DOY %d)\n",
           utc_time.year, utc_time.month, utc_time.day,
           utc_time.hour, utc_time.minute, utc_time.second, doy);
    printf("  azimuth = %.2f deg, altitude = %.2f deg\n",
           sun_now.azimuth_deg, sun_now.altitude_deg);
    printf("  RA = %.2f deg, Dec = %.2f deg, HA = %.2f deg\n",
           sun_now.right_ascension_deg, sun_now.declination_deg, sun_now.hour_angle_deg);

    const auto vector = MSunPosition::calculate_vector(observer, utc_time);
    const auto direction = vector.unit_direction();
    printf("  observer-to-Sun distance = %s km\n", format_number(vector.magnitude() / 1000.0, 3).c_str());
    printf("  Sun vector ENU (m) = (%s, %s, %s)\n",
           format_number(vector.east, 3).c_str(),
           format_number(vector.north, 3).c_str(),
           format_number(vector.up, 3).c_str());
    printf("  Sun unit direction ENU = (%.9f, %.9f, %.9f)\n", direction.east, direction.north, direction.up);

    printf("\nDaily events (UTC)\n");
    print_hhmmss_utc("solar noon", sun_daily.solar_noon_utc_hours);

    if(sun_daily.has_sunrise_sunset){
        print_hhmmss_utc("sunrise", sun_daily.sunrise_utc_hours);
        print_hhmmss_utc("sunset", sun_daily.sunset_utc_hours);
    }
    else{
        printf("  sunrise/sunset: none on this date at this latitude (polar day/night)\n");
    }

    printf("  equation of time = %.2f min\n", sun_daily.equation_of_time_minutes);
    printf("       declination = %.2f deg\n", sun_daily.declination_deg);
} // ——— END OF print_output() —————————————————————————————————————————————————

/// @brief Print lunar direction, phase and observer distance.
/// @param observer WGS-84 observer for the supplied result.
/// @param observer_name Optional label; nullptr or empty omits it.
/// @param utc_time Valid UTC timestamp matching moon.
/// @param moon Calculated lunar position and phase.
/// @param include_observer Whether to print observer details before the Moon block.
void print_moon_output(const MGeoPoint& observer, const char* observer_name,
                       const MDateTimeUtc& utc_time, const MMoonHorizontalPosition& moon,
                       bool include_observer)
{
    if(include_observer){
        if(observer_name != nullptr && observer_name[0] != '\0')
            printf("Name     -> %s\n", observer_name);
        printf("Observer -> lat = %.5f deg, lon = %.5f deg, alt = %s m\n",
               observer.get_lat(), observer.get_lon(), format_number(observer.get_alt(), 1).c_str());
    }else{
        printf("\n");
    }
    printf("Moon @ %04d-%02d-%02d %02d:%02d:%02d UTC (DOY %d)\n",
           utc_time.year, utc_time.month, utc_time.day,
           utc_time.hour, utc_time.minute, utc_time.second, day_of_year(utc_time));
    printf("  azimuth = %.2f deg, altitude = %.2f deg\n", moon.azimuth_deg, moon.altitude_deg);
    printf("  phase = %s, %.1f%% illuminated\n", moon.phase_name(), moon.illuminated_fraction * 100);
    printf("  observer-to-Moon distance = %s km\n", format_number(moon.distance_m / 1000.0, 3).c_str());
}
