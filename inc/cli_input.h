/*
 * File: cli_input.h
 * Created: 2026-04-25
 * Author: jole
 * Description: Declarations for CLI option parsing and input resolution.
 */

#pragma once

#include <string>

#include "sun_position.h"


enum class MCliAction
{
    Continue,
    PrintHelp,
    PrintUsageError
};


struct MCliOptions
{
    MCliAction action = MCliAction::Continue;

    bool have_read  = false;
    bool have_name  = false;
    bool have_lat   = false;
    bool have_lon   = false;
    bool have_alt   = false;
    bool have_date  = false;
    bool have_time  = false;

    std::string read_file_arg;
    std::string name_arg;
    std::string date_arg;
    std::string time_arg;

    double lat_deg  = 0.0;
    double lon_deg  = 0.0;
    double alt_m    = 0.0;
};


struct MResolvedCliInput
{
    bool edit_name_mode = false;

    double lat_deg = 0.0;
    double lon_deg = 0.0;
    double alt_m   = 0.0;

    std::string name_file_arg;
    std::string observer_name;

    MDateTimeUtc utc_time{};
};


MCliOptions parse_cli_options(int argc, char** argv);

MResolvedCliInput resolve_cli_input(const MCliOptions& options);