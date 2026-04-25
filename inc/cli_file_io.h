/*
 * File: cli_file_io.h
 * Created: 2026-04-25
 * Author: jole
 * Description: Declarations for CLI config-file and name-library file I/O.
 */

#pragma once

#include <string>


struct MInputFileData
{
    bool have_lat   = false;
    bool have_lon   = false;
    bool have_alt   = false;
    bool have_date  = false;
    bool have_time  = false;

    double lat_deg  = 0.0;
    double lon_deg  = 0.0;
    double alt_m    = 0.0;

    std::string date_value;
    std::string time_value;
};


void read_config_file(const char* file_path, MInputFileData& data);

bool read_named_location_file(const char* file_path,
                              const char* location_name,
                              MInputFileData& data);

bool upsert_named_location_file(const char* file_path,
                                const char* location_name,
                                double lat_deg,
                                double lon_deg,
                                bool have_alt,
                                double alt_m);