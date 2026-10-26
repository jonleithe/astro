/*
 * File: print_output.h
 * Created: 2026-04-25
 * Author: jole
 * Description: Declarations for CLI usage and result-printing helpers.
 */

#pragma once

#include <string_view>

#include "geo_point.h"
#include "sun_position.h"


namespace print_output_text {

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


void print_usage(const char* argv0);

void print_output(const MGeoPoint& observer,
                  const char* observer_name,
                  const MDateTimeUtc& utc_time,
                  const MSunHorizontalPosition& sun_now,
                  const MSunDailyEventsUtc& sun_daily);
