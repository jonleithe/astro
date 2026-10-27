/**
 * @file cli_input.h
 * Created: 2026-04-25
 * Author: jole
 * @brief Declarations for CLI option parsing and input resolution.
 */

#pragma once

#include <string>
#include <vector>

#include "sun_position.h"
#include "planet_position.h"


/// @brief Action requested by argument parsing.
enum class MCliAction
{
    /// @brief Continue to resolve and execute the request.
    Continue,
    /// @brief Display help and exit successfully.
    PrintHelp,
    /// @brief Display usage and exit with an error.
    PrintUsageError
};


/// @brief Explicit command-line values before file/default resolution.
struct MCliOptions
{
    /// @brief Action requested by the parser.
    MCliAction action = MCliAction::Continue;

    /// @brief Whether interactive terminal output was requested.
    bool tui = false;
    /// @brief Local WGS-84 ASCII DEM path, empty to disable terrain.
    std::string terrain_dem;
    /// @brief Explicit confirmation that DEM heights are ellipsoidal metres.
    bool terrain_ellipsoid = false;
    /// @brief Maximum terrain sampling distance in metres.
    double terrain_radius_m = 20000;
    /// @brief Terrain radial sampling step in metres.
    double terrain_step_m = 100;
    /// @brief Whether terrain sampling parameters were explicitly supplied.
    bool have_terrain_sampling = false;
    /// @brief Whether an explicit object list was supplied.
    bool have_objects = false;
    /// @brief Whether the Sun is selected.
    bool show_sun = false;
    /// @brief Whether the Moon is selected.
    bool show_moon = false;
    /// @brief Explicitly selected planets in fixed display order.
    std::vector<MPlanet> planets;

    /// @brief Whether read was explicitly supplied.
    bool have_read  = false;
    /// @brief Whether name was explicitly supplied.
    bool have_name  = false;
    /// @brief Whether lat was explicitly supplied.
    bool have_lat   = false;
    /// @brief Whether lon was explicitly supplied.
    bool have_lon   = false;
    /// @brief Whether alt was explicitly supplied.
    bool have_alt   = false;
    /// @brief Whether date was explicitly supplied.
    bool have_date  = false;
    /// @brief Whether time was explicitly supplied.
    bool have_time  = false;

    /// @brief Path supplied with --read.
    std::string read_file_arg;
    /// @brief Location name supplied with --name.
    std::string name_arg;
    /// @brief UTC date argument, expected as YYYY-MM-DD.
    std::string date_arg;
    /// @brief UTC time argument, expected as HH:MM:SS.
    std::string time_arg;

    /// @brief Geodetic latitude in decimal degrees.
    double lat_deg  = 0.0;
    /// @brief Longitude in decimal degrees, east positive.
    double lon_deg  = 0.0;
    /// @brief Ellipsoidal height in metres.
    double alt_m    = 0.0;
};


/// @brief Resolved display input or a request to edit the location library.
struct MResolvedCliInput
{
    /// @brief Whether to update the named-location library instead of calculating.
    bool edit_name_mode = false;
    /// @brief Whether the Sun is selected.
    bool show_sun = true;
    /// @brief Whether the Moon is selected.
    bool show_moon = false;
    /// @brief Explicitly selected planets in fixed display order.
    std::vector<MPlanet> planets;

    /// @brief Geodetic latitude in decimal degrees.
    double lat_deg = 0.0;
    /// @brief Longitude in decimal degrees, east positive.
    double lon_deg = 0.0;
    /// @brief Ellipsoidal height in metres.
    double alt_m   = 0.0;

    /// @brief Resolved location-library path.
    std::string name_file_arg;
    /// @brief Observer label; empty when no name was supplied.
    std::string observer_name;

    /// @brief Resolved UTC timestamp; unused in name-edit mode.
    MDateTimeUtc utc_time{};
};


/// @brief Read the current system clock as UTC calendar fields.
/// @return Current UTC date and time.
MDateTimeUtc now();


/// @brief Parse command-line options without loading files.
/// @param argc Number of command-line arguments, including the executable name.
/// @param argv Argument strings supplied to main.
/// @return Parsed values and help/usage action.
/// @throws std::invalid_argument For invalid values, object lists or positional arguments.
MCliOptions parse_cli_options(int argc, char** argv);

/// @brief Merge file settings, explicit options and current UTC defaults.
/// @param options Parsed command-line options.
/// @return Resolved observer/time/object selection or name-edit request.
/// @throws std::invalid_argument For missing coordinates, file errors or incompatible modes.
/// @note Name-edit results do not contain resolved observation coordinates or time.
MResolvedCliInput resolve_cli_input(const MCliOptions& options);
