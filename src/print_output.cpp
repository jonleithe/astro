/*
 * File: print_output.cpp
 * Created: 2026-04-25
 * Author: jole
 * Description: CLI usage and formatted output helpers for the astro executable.
 */

#include <cmath>
#include <cstdio>

#include "print_output.h"



void print_usage(const char* argv0)
{
    printf("Usage:\n");
    printf("  %s [--read <file>] --lat <deg> --lon <deg> [--alt <m>] [--date YYYY-MM-DD] [--time HH:MM:SS]\n", argv0);
    printf("  %s --read <name-file> --name <location> [--alt <m>] [--date YYYY-MM-DD] [--time HH:MM:SS]\n", argv0);
    printf("  %s --name <location> --lat <deg> --lon <deg> [--alt <m>]\n", argv0);
    printf("\n");
    printf("Required:\n");
    printf("  --lat,  -a   Observer latitude in decimal degrees (required unless in --read file)\n");
    printf("  --lon,  -o   Observer longitude in decimal degrees (required unless in --read file)\n");
    printf("\n");
    printf("Optional:\n");
    printf("  --read, -r   Read settings file or name-library file\n");
    printf("  --name, -n   Location name to look up from name-library file\n");
    printf("  --alt,  -z   Observer altitude in metres (default: 0.0)\n");
    printf("  --date, -d   UTC date as YYYY-MM-DD (default: current UTC date)\n");
    printf("  --time, -t   UTC time as HH:MM:SS (default: current UTC time)\n");
    printf("  --help, -h   Print this help\n");
    printf("\n");
    printf("Read-file formats:\n");
    printf("  1) Config mode (without --name): key = value (lat, lon, alt, date, time)\n");
    printf("  2) Name mode (with --name):      location = lat, lon[, alt]\n");
    printf("  If --name is used without --read, default name file is names.ini\n");
    printf("  comments start with # or ;\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s --read astro.conf --lat 78.24490 --lon 15.49369\n", argv0);
    printf("  %s --read locations.ini --name ny_aalesund\n", argv0);
    printf("  %s --name ny_aalesund --lat 78.12345678 --lon 11.98765432 --alt 8.2\n", argv0);
} // ——— END OF print_usage() ——————————————————————————————————————————————————



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



static bool is_leap_year(int year)
{
    if(year % 400 == 0)
        return true;
    if(year % 100 == 0)
        return false;
    return (year % 4) == 0;
} // ——— END OF is_leap_year() ———————————————————————————————————————————————



static int day_of_year(const MDateTimeUtc& utc_time)
{
    static const int days_before_month[12] = {0, 31, 59, 90, 120, 151,
                                              181, 212, 243, 273, 304, 334};

    int doy = days_before_month[utc_time.month - 1] + utc_time.day;
    if(utc_time.month > 2 && is_leap_year(utc_time.year))
        doy += 1;

    return doy;
} // ——— END OF day_of_year() ————————————————————————————————————————————————



void print_output(const MGeoPoint& observer,
                  const char* observer_name,
                  const MDateTimeUtc& utc_time,
                  const MSunHorizontalPosition& sun_now,
                  const MSunDailyEventsUtc& sun_daily)
{
    const int doy = day_of_year(utc_time);

    if(observer_name != nullptr && observer_name[0] != '\0')
        printf("Name     -> %s\n", observer_name);

    printf("Observer -> lat = %.5f deg, lon = %.5f deg, alt = %.1f m\n",
           observer.get_lat(), observer.get_lon(), observer.get_alt());

    printf("Sun @ %04d-%02d-%02d %02d:%02d:%02d UTC (DOY %d)\n",
           utc_time.year, utc_time.month, utc_time.day,
           utc_time.hour, utc_time.minute, utc_time.second, doy);
    printf("  azimuth = %.2f deg, altitude = %.2f deg\n",
           sun_now.azimuth_deg, sun_now.altitude_deg);
    printf("  RA = %.2f deg, Dec = %.2f deg, HA = %.2f deg\n",
           sun_now.right_ascension_deg, sun_now.declination_deg, sun_now.hour_angle_deg);

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