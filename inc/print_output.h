/**
 * @file print_output.h
 * Created: 2026-04-25
 * Author: jole
 * @brief Declarations for CLI usage and result-printing helpers.
 */

#pragma once

#include <string_view>

#include "geo_point.h"
#include "sun_position.h"
#include "moon_position.h"
#include "terrain_horizon.h"


namespace print_output_text {

/// @brief CLI help template; {program} is replaced by the executable name.
inline constexpr std::string_view usage = R"help(Usage:
  {program} [--read <file>] --lat <deg> --lon <deg> [--alt <m>] [--date YYYY-MM-DD] [--time HH:MM:SS]
  {program} --read <name-file> --name <location> [--alt <m>] [--date YYYY-MM-DD] [--time HH:MM:SS]
  {program} --name <location> --lat <deg> --lon <deg> [--alt <m>]

Required:
  --lat,  -a   Observer latitude in decimal degrees (required unless in --read file)
  --lon,  -o   Observer longitude in decimal degrees (required unless in --read file)

Optional:
  --read, -r   Read settings file or name-library file
  --name, -n   Location name to look up from name-library file
  --alt,  -z   Observer altitude in metres (default: 0.0)
  --date, -d   UTC date as YYYY-MM-DD (default: current UTC date)
  --time, -t   UTC time as HH:MM:SS (default: current UTC time)
  --objects    Comma-separated objects: sun,moon (text default: sun; TUI: both)
  --terrain-dem  Local WGS-84 lon/lat ESRI ASCII elevation grid
  --terrain-ellipsoid  Confirm DEM heights are WGS-84 ellipsoidal metres
  --terrain-radius  Search radius in metres (default 20000; maximum 100000)
  --terrain-step    Radial sample spacing in metres (default 100; minimum 10)
  --tui        Interactive sky chart (requires ncurses and a terminal)
  --help, -h   Print this help

Read-file formats:
  1) Config mode (without --name): key = value (lat, lon, alt, date, time)
  2) Name mode (with --name):      location = lat, lon[, alt]
  If --name is used without --read, default name file is names.ini
  comments start with # or ;

Example:
  {program} --read astro.conf --lat 78.24490 --lon 15.49369
  {program} --read locations.ini --name ny_aalesund
  {program} --name ny_aalesund --lat 78.12345678 --lon 11.98765432 --alt 8.2
)help";

} // namespace print_output_text


/// @brief Print CLI usage, substituting the executable name.
/// @param argv0 Executable name to show in examples.
void print_usage(const char* argv0);

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
                  const MSunDailyEventsUtc& sun_daily);

/// @brief Print lunar direction, phase and observer distance.
/// @param observer WGS-84 observer for the supplied result.
/// @param observer_name Optional label; nullptr or empty omits it.
/// @param utc_time Valid UTC timestamp matching moon.
/// @param moon Calculated lunar position and phase.
/// @param include_observer Whether to print observer details before the Moon block.
void print_moon_output(const MGeoPoint& observer, const char* observer_name,
                       const MDateTimeUtc& utc_time, const MMoonHorizontalPosition& moon,
                       bool include_observer);



/// @brief Format centre clearance above the sampled terrain profile.
/// @param terrain Cached skyline for the current observer.
/// @param azimuth_deg Object bearing in degrees.
/// @param altitude_deg Object centre altitude in degrees.
/// @return Short geometric clearance/coverage description, without refraction.
std::string terrain_clearance(const MTerrainHorizon& terrain, double azimuth_deg, double altitude_deg);



/// @brief Print terrain provenance and sampling limitations.
/// @param terrain Cached skyline for the current observer.
void print_terrain_info(const MTerrainHorizon& terrain);
