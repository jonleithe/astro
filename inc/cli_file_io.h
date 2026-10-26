/**
 * @file cli_file_io.h
 * Created: 2026-04-25
 * Author: jole
 * @brief Declarations for CLI config-file and name-library file I/O.
 */

#pragma once

#include <string>


/// @brief File values with presence flags for merging CLI overrides.
struct MInputFileData
{
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

    /// @brief Geodetic latitude in decimal degrees.
    double lat_deg  = 0.0;
    /// @brief Longitude in decimal degrees, east positive.
    double lon_deg  = 0.0;
    /// @brief Ellipsoidal height in metres.
    double alt_m    = 0.0;

    /// @brief Unparsed UTC date string from the file.
    std::string date_value;
    /// @brief Unparsed UTC time string from the file.
    std::string time_value;
};


/// @brief Read latitude, longitude, height and UTC date/time settings.
/// @param file_path Settings file to read.
/// @param[in,out] data Parsed values; fields not present in the file are retained.
/// @throws std::invalid_argument If the file cannot be opened or contains invalid settings.
/// @note Calendar strings are stored for later parsing; coordinates use degrees/metres.
void read_config_file(const char* file_path, MInputFileData& data);

/// @brief Look up a location by case-insensitive, trimmed name.
/// @param file_path Location library to read.
/// @param location_name Name to find.
/// @param[in,out] data Receives matching coordinates; unspecified fields are retained.
/// @return True if a matching entry was found, false otherwise.
/// @throws std::invalid_argument If opening or parsing the library fails.
bool read_named_location_file(const char* file_path,
                              const char* location_name,
                              MInputFileData& data);

/// @brief Add a named location or confirm replacement of an existing entry.
/// @param file_path Location library to create or update.
/// @param location_name Name to store and match case-insensitively.
/// @param lat_deg Latitude in decimal degrees.
/// @param lon_deg Longitude in decimal degrees.
/// @param have_alt Whether to include ellipsoidal height in the stored entry.
/// @param alt_m Ellipsoidal height in metres, used only when have_alt is true.
/// @return True after writing; false if replacement confirmation is declined.
/// @throws std::invalid_argument If the output file cannot be opened.
/// @note Replacements comment out the old entry and insert the new entry after it.
bool upsert_named_location_file(const char* file_path,
                                const char* location_name,
                                double lat_deg,
                                double lon_deg,
                                bool have_alt,
                                double alt_m);
