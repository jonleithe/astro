
/*
 * File: main.cpp
 * Created: 2026-04-24
 * Author: jole
 * Description: Entry point for constructing and printing sample MGeoPoint values.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <getopt.h>
#include <stdexcept>

#include "geo_point.h"
#include "sun_position.h"



static void print_usage(const char* argv0)
{
    printf("Usage:\n");
    printf("  %s --lat <deg> --lon <deg> [--alt <m>] [--date YYYY-MM-DD] [--time HH:MM:SS]\n", argv0);
    printf("\n");
    printf("Required:\n");
    printf("  --lat,  -a   Observer latitude in decimal degrees\n");
    printf("  --lon,  -o   Observer longitude in decimal degrees\n");
    printf("\n");
    printf("Optional:\n");
    printf("  --alt,  -z   Observer altitude in metres (default: 0.0)\n");
    printf("  --date, -d   UTC date as YYYY-MM-DD (default: current UTC date)\n");
    printf("  --time, -t   UTC time as HH:MM:SS (default: current UTC time)\n");
    printf("  --help, -h   Print this help\n");
    printf("\n");
    printf("Example:\n");
    printf("  %s --lat 78.24490 --lon 15.49369 --alt 0.0 --date 2026-06-21 --time 12:00:00\n", argv0);
} // ——— END OF print_usage() ——————————————————————————————————————————————————



static double parse_double(const char* arg_name, const char* value)
{
    char* endptr = nullptr;
    double parsed = std::strtod(value, &endptr);

    if (endptr == value || *endptr != '\0')
        throw std::invalid_argument(arg_name);

    return parsed;
} // ——— END OF parse_double() —————————————————————————————————————————————————



static MDateTimeUtc current_utc_datetime()
{
    std::time_t now = std::time(nullptr);
    std::tm utc_tm{};

    #if defined(_WIN32)
        gmtime_s(&utc_tm, &now);
    #else
        gmtime_r(&now, &utc_tm);
    #endif

    return MDateTimeUtc{utc_tm.tm_year + 1900,
                        static_cast<short>(utc_tm.tm_mon + 1),
                        static_cast<short>(utc_tm.tm_mday),
                        static_cast<short>(utc_tm.tm_hour),
                        static_cast<short>(utc_tm.tm_min),
                        static_cast<short>(utc_tm.tm_sec)};
} // ——— END OF current_utc_datetime() —————————————————————————————————————————



static void apply_date_string(MDateTimeUtc& utc_time, const char* date_arg)
{
    int year    = 0;
    int month   = 0;
    int day     = 0;
    char tail   = '\0';

    if (std::sscanf(date_arg, "%d-%d-%d%c", &year, &month, &day, &tail) != 3)
        throw std::invalid_argument("date format must be YYYY-MM-DD");

    utc_time.year   = year;
    utc_time.month  = static_cast<short>(month);
    utc_time.day    = static_cast<short>(day);
} // ——— END OF apply_date_string() ————————————————————————————————————————————



static void apply_time_string(MDateTimeUtc& utc_time, const char* time_arg)
{
    int hour    = 0;
    int minute  = 0;
    int second  = 0;
    char tail   = '\0';

    if (std::sscanf(time_arg, "%d:%d:%d%c", &hour, &minute, &second, &tail) != 3)
        throw std::invalid_argument("time format must be HH:MM:SS");

    utc_time.hour   = static_cast<short>(hour);
    utc_time.minute = static_cast<short>(minute);
    utc_time.second = static_cast<short>(second);
} // ——— END OF apply_time_string() ————————————————————————————————————————————



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



int main(int argc, char** argv)
{
    static struct option long_options[] = { {"lat", required_argument, nullptr, 'a'},
                                            {"lon", required_argument, nullptr, 'o'},
                                            {"alt", required_argument, nullptr, 'z'},
                                            {"date", required_argument, nullptr, 'd'},
                                            {"time", required_argument, nullptr, 't'},
                                            {"help", no_argument, nullptr, 'h'},
                                            {nullptr, 0, nullptr, 0}};

    const char* short_options = "a:o:z:d:t:h";

    bool have_lat   = false;
    bool have_lon   = false;
    bool have_date  = false;
    bool have_time  = false;

    const char* date_arg = nullptr;
    const char* time_arg = nullptr;

    double lat_deg  = 0.0;
    double lon_deg  = 0.0;
    double alt_m    = 0.0;

    int option_index = 0;
    int c = 0;

    try{
        while((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1){
            switch(c){
                case 'a':       lat_deg = parse_double("invalid --lat value", optarg);
                                have_lat = true;
                                break;
                case 'o':       lon_deg = parse_double("invalid --lon value", optarg);
                                have_lon = true;
                                break;
                case 'z':       alt_m = parse_double("invalid --alt value", optarg);
                                break;
                case 'd':       date_arg = optarg;
                                have_date = true;
                                break;
                case 't':       time_arg = optarg;
                                have_time = true;
                                break;
                case 'h':       print_usage(argv[0]);
                                return 0;
                default:        print_usage(argv[0]);
                                return 1;
            }
        }

        if(optind < argc)
            throw std::invalid_argument("unexpected positional arguments");

        if(!have_lat || !have_lon)
            throw std::invalid_argument("missing required options (--lat, --lon)");

        MDateTimeUtc utc_time = current_utc_datetime();
        if(have_date)
            apply_date_string(utc_time, date_arg);
        if(have_time)
            apply_time_string(utc_time, time_arg);

        MGeoPoint observer(lat_deg, lon_deg, alt_m);
        MSunHorizontalPosition sun_now = MSunPosition::calculate(observer, utc_time);
        MSunDailyEventsUtc sun_daily = MSunPosition::calculate_daily_events(observer, utc_time);

        printf("Observer -> lat = %.5f deg, lon = %.5f deg, alt = %.1f m\n",
                observer.get_lat(), observer.get_lon(), observer.get_alt());

        printf("Sun @ %04d-%02d-%02d %02d:%02d:%02d UTC\n",
                        utc_time.year, utc_time.month, utc_time.day,
                        utc_time.hour, utc_time.minute, utc_time.second);
        printf("  azimuth = %.2f deg, altitude = %.2f deg\n",
                        sun_now.azimuth_deg, sun_now.altitude_deg);
        printf("  RA = %.2f deg, Dec = %.2f deg, HA = %.2f deg\n",
                        sun_now.right_ascension_deg, sun_now.declination_deg, sun_now.hour_angle_deg);

        printf("Daily events (UTC)\n");
        print_hhmmss_utc("solar noon", sun_daily.solar_noon_utc_hours);

        if(sun_daily.has_sunrise_sunset){
            print_hhmmss_utc("sunrise", sun_daily.sunrise_utc_hours);
            print_hhmmss_utc("sunset", sun_daily.sunset_utc_hours);
        }
        else{
            printf("  sunrise/sunset: none on this date at this latitude (polar day/night)\n");
        }

        printf("  equation of time = %.2f min\n", sun_daily.equation_of_time_minutes);
        printf("  declination = %.2f deg\n", sun_daily.declination_deg);
    }
    catch (const std::exception& ex){
        printf("Error: %s\n", ex.what());
        print_usage(argv[0]);
        return 1;
    }

    return 0;
} // ——— END OF main() —————————————————————————————————————————————————————————
