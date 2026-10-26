/**
 * @file cli_input.cpp
 * Created: 2026-04-25
 * Author: jole
 * @brief CLI option parsing and merged runtime input resolution.
 */

#include <ctime>
#include <algorithm>
#include <cstdio>
#include <string>
#include <cstdlib>
#include <stdexcept>

#include <getopt.h>

#include "cli_input.h"
#include "cli_file_io.h"


/// @brief Parse a complete numeric argument using strtod.
/// @param arg_name Error message used when numeric parsing fails.
/// @param value Value or string to parse/format.
/// @return Parsed floating-point value.
/// @throws std::invalid_argument If parsing or validation fails.
static double parse_double(const char* arg_name, const char* value)
{
    char* endptr = nullptr;
    double parsed = std::strtod(value, &endptr);

    if(endptr == value || *endptr != '\0')
        throw std::invalid_argument(arg_name);
    
    return parsed;
} // ——— END OF parse_double() —————————————————————————————————————————————————



/// @brief Read the current system clock as UTC calendar fields.
/// @return Current UTC date and time.
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



/// @brief Parse a date string into UTC year, month and day fields.
/// @param utc_time Valid UTC calendar date and time.
/// @param date_arg Date string expected as YYYY-MM-DD.
/// @throws std::invalid_argument If parsing or validation fails.
/// @note Parses fields only; calendar range checks occur during position calculation.
static void apply_date_string(MDateTimeUtc& utc_time, const char* date_arg)
{
    int year    = 0;
    int month   = 0;
    int day     = 0;
    char tail   = '\0';

    if(std::sscanf(date_arg, "%d-%d-%d%c", &year, &month, &day, &tail) != 3)
        throw std::invalid_argument("date format must be YYYY-MM-DD");

    utc_time.year   = year;
    utc_time.month  = static_cast<short>(month);
    utc_time.day    = static_cast<short>(day);
} // ——— END OF apply_date_string() ————————————————————————————————————————————



/// @brief Parse a time string into UTC hour, minute and second fields.
/// @param utc_time Valid UTC calendar date and time.
/// @param time_arg Time string expected as HH:MM:SS.
/// @throws std::invalid_argument If parsing or validation fails.
/// @note Parses fields only; calendar range checks occur during position calculation.
static void apply_time_string(MDateTimeUtc& utc_time, const char* time_arg)
{
    int hour    = 0;
    int minute  = 0;
    int second  = 0;
    char tail   = '\0';

    if(std::sscanf(time_arg, "%d:%d:%d%c", &hour, &minute, &second, &tail) != 3)
        throw std::invalid_argument("time format must be HH:MM:SS");

    utc_time.hour   = static_cast<short>(hour);
    utc_time.minute = static_cast<short>(minute);
    utc_time.second = static_cast<short>(second);
} // ——— END OF apply_time_string() ————————————————————————————————————————————



/// @brief Parse and validate a comma-separated celestial-object selection.
/// @param options Parsed command-line state to update.
/// @param value Value or string to parse/format.
/// @throws std::invalid_argument If parsing or validation fails.
static void parse_objects(MCliOptions& options, const std::string& value)
{
    if(options.have_objects)
        throw std::invalid_argument("--objects may only be specified once");
    options.have_objects = true;
    std::size_t start = 0;
    for(;;){
        auto end = value.find(',', start);
        auto name = value.substr(start, end == std::string::npos ? end : end - start);
        if(name.empty())
            throw std::invalid_argument("--objects requires a nonempty comma-separated list: sun, moon, mercury, venus, mars");
        if(name == "sun") options.show_sun = true;
        else if(name == "moon") options.show_moon = true;
        else if(name == "mercury") options.planets.push_back(MPlanet::Mercury);
        else if(name == "venus") options.planets.push_back(MPlanet::Venus);
        else if(name == "mars") options.planets.push_back(MPlanet::Mars);
        else throw std::invalid_argument("Unknown object '" + name + "'. Available objects: sun, moon, mercury, venus, mars");
        if(end == std::string::npos) break;
        start = end + 1;
    }
    std::sort(options.planets.begin(), options.planets.end());
    options.planets.erase(std::unique(options.planets.begin(), options.planets.end()), options.planets.end());
}

/// @brief Parse command-line options without loading files.
/// @param argc Number of command-line arguments, including the executable name.
/// @param argv Argument strings supplied to main.
/// @return Parsed values and help/usage action.
/// @throws std::invalid_argument For invalid values, object lists or positional arguments.
MCliOptions parse_cli_options(int argc, char** argv)
{
    static struct option long_options[] = { {"read",    required_argument,  nullptr,    'r'},
                                            {"name",    required_argument,  nullptr,    'n'},
                                            {"lat",     required_argument,  nullptr,    'a'},
                                            {"lon",     required_argument,  nullptr,    'o'},
                                            {"alt",     required_argument,  nullptr,    'z'},
                                            {"date",    required_argument,  nullptr,    'd'},
                                            {"time",    required_argument,  nullptr,    't'},
                                            {"terrain-dem", required_argument, nullptr, 258},
                                            {"terrain-ellipsoid", no_argument, nullptr, 259},
                                            {"terrain-radius", required_argument, nullptr, 260},
                                            {"terrain-step", required_argument, nullptr, 261},
                                            {"objects", required_argument,  nullptr,    257},
                                            {"tui",     no_argument,        nullptr,    256},
                                            {"help",    no_argument,        nullptr,    'h'},
                                            {nullptr, 0, nullptr, 0}};

    const char* short_options = "r:n:a:o:z:d:t:h";

    MCliOptions options;
    int option_index = 0;
    int c = 0;

    optind = 1;

    while((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1){
        switch(c){
            case 258:   options.terrain_dem = optarg;
                        if(options.terrain_dem.empty())
                            throw std::invalid_argument("--terrain-dem requires a file path");
                        break;
            case 259:   options.terrain_ellipsoid = true;
                        break;
            case 260:   options.terrain_radius_m = parse_double("invalid terrain radius", optarg);
                        options.have_terrain_sampling = true;
                        break;
            case 261:   options.terrain_step_m = parse_double("invalid terrain step", optarg);
                        options.have_terrain_sampling = true;
                        break;
            case 257:   parse_objects(options, optarg);
                        break;
            case 256:   options.tui = true;
                        break;
            case 'r':   options.have_read = true;
                        options.read_file_arg = optarg;
                        break;
            case 'n':   options.have_name = true;
                        options.name_arg = optarg;
                        break;
            case 'a':   options.lat_deg = parse_double("invalid --lat value", optarg);
                        options.have_lat = true;
                        break;
            case 'o':   options.lon_deg = parse_double("invalid --lon value", optarg);
                        options.have_lon = true;
                        break;
            case 'z':   options.alt_m = parse_double("invalid --alt value", optarg);
                        options.have_alt = true;
                        break;
            case 'd':   options.date_arg = optarg;
                        options.have_date = true;
                        break;
            case 't':   options.time_arg = optarg;
                        options.have_time = true;
                        break;
            case 'h':   options.action = MCliAction::PrintHelp;
                        return options;
            default:    options.action = MCliAction::PrintUsageError;
                        return options;
        } // ——— END OF switch for option parsing ——————————————————————————————
    } // ——— END OF while loop for option parsing ——————————————————————————————

    if(optind < argc)
        throw std::invalid_argument("unexpected positional arguments");

    return options;
} // ——— END OF parse_cli_options() ————————————————————————————————————————————



/// @brief Merge file settings, explicit options and current UTC defaults.
/// @param options Parsed command-line options.
/// @return Resolved observer/time/object selection or name-edit request.
/// @throws std::invalid_argument For missing coordinates, file errors or incompatible modes.
/// @note Name-edit results do not contain resolved observation coordinates or time.
MResolvedCliInput resolve_cli_input(const MCliOptions& options)
{
    MResolvedCliInput resolved;
    MInputFileData config_data;
    if(!options.terrain_dem.empty() && !options.terrain_ellipsoid)
        throw std::invalid_argument("terrain DEM requires --terrain-ellipsoid to confirm WGS-84 ellipsoidal heights in metres");
    if(options.terrain_dem.empty() && (options.terrain_ellipsoid || options.have_terrain_sampling))
        throw std::invalid_argument("terrain options require --terrain-dem");
    resolved.planets = options.planets;
    resolved.show_sun = options.have_objects ? options.show_sun : true;
    resolved.show_moon = options.have_objects ? options.show_moon : options.tui;

    resolved.name_file_arg = options.have_read ? options.read_file_arg : "names.ini";
    resolved.observer_name = options.have_name ? options.name_arg : "";
    resolved.edit_name_mode = options.have_name && options.have_lat && options.have_lon;

    if(resolved.edit_name_mode){
        if(!options.terrain_dem.empty())
            throw std::invalid_argument("terrain cannot be combined with name edit mode");
        if(options.have_objects)
            throw std::invalid_argument("--objects cannot be combined with name edit mode");
        if(options.tui)
            throw std::invalid_argument("--tui cannot be combined with name edit mode");
        return resolved;
    }

    if(options.have_name){
        bool found = read_named_location_file(resolved.name_file_arg.c_str(),
                                              options.name_arg.c_str(),
                                              config_data);

        if(!found)
            throw std::invalid_argument(std::string("name not found in name file: ") + options.name_arg);
    }
    else if(options.have_read)
        read_config_file(options.read_file_arg.c_str(), config_data);

    if(config_data.have_lat)
        resolved.lat_deg = config_data.lat_deg;
    if(config_data.have_lon)
        resolved.lon_deg = config_data.lon_deg;
    if(config_data.have_alt)
        resolved.alt_m = config_data.alt_m;

    if(options.have_lat)
        resolved.lat_deg = options.lat_deg;
    if(options.have_lon)
        resolved.lon_deg = options.lon_deg;
    if(options.have_alt)
        resolved.alt_m = options.alt_m;

    if(!(config_data.have_lat || options.have_lat) || !(config_data.have_lon || options.have_lon))
        throw std::invalid_argument("missing required options (--lat, --lon) or values in --read file");

    resolved.utc_time = current_utc_datetime();

    if(config_data.have_date)
        apply_date_string(resolved.utc_time, config_data.date_value.c_str());
    if(config_data.have_time)
        apply_time_string(resolved.utc_time, config_data.time_value.c_str());
    if(options.have_date)
        apply_date_string(resolved.utc_time, options.date_arg.c_str());
    if(options.have_time)
        apply_time_string(resolved.utc_time, options.time_arg.c_str());

    return resolved;
} // ——— END OF resolve_cli_input() ————————————————————————————————————————————