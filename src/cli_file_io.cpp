/**
 * @file cli_file_io.cpp
 * Created: 2026-04-25
 * Author: jole
 * @brief CLI config-file and name-library file parsing and update helpers.
 */

#include <string>
#include <vector>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

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



/// @brief Remove leading and trailing whitespace.
/// @param input Input string to transform.
/// @return Copy without surrounding whitespace.
static std::string trim(const std::string& input)
{
    std::size_t first = 0;
    while(first < input.size() && std::isspace(static_cast<unsigned char>(input[first])))
        first++;

    std::size_t last = input.size();
    while(last > first && std::isspace(static_cast<unsigned char>(input[last - 1])))
        last--;

    return input.substr(first, last - first);
} // ——— END OF trim() —————————————————————————————————————————————————————————



/// @brief Convert a string to lowercase using the current C locale.
/// @param input Input string to transform.
/// @return Lowercase copy of the input.
static std::string to_lowercase(const std::string& input)
{
    std::string lowered = input;
    for(char& ch : lowered)
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

    return lowered;
} // ——— END OF to_lowercase() —————————————————————————————————————————————————



/// @brief Read latitude, longitude, height and UTC date/time settings.
/// @param file_path Settings file to read.
/// @param[in,out] data Parsed values; fields not present in the file are retained.
/// @throws std::invalid_argument If the file cannot be opened or contains invalid settings.
/// @note Calendar strings are stored for later parsing; coordinates use degrees/metres.
void read_config_file(const char* file_path, MInputFileData& data)
{
    std::ifstream input(file_path);
    if(!input)
        throw std::invalid_argument(std::string("cannot open read file: ") + file_path);

    std::string line;
    int line_number = 0;

    while(std::getline(input, line)){
        line_number++;

        std::string stripped = trim(line);
        if(stripped.empty())
            continue;

        if(stripped[0] == '#' || stripped[0] == ';')
            continue;

        std::size_t separator = stripped.find('=');
        if(separator == std::string::npos)
            throw std::invalid_argument("read file parse error at line " + std::to_string(line_number) + ": missing '='");

        std::string key = to_lowercase(trim(stripped.substr(0, separator)));
        std::string value = trim(stripped.substr(separator + 1));

        if(value.empty())
            throw std::invalid_argument("read file parse error at line " + std::to_string(line_number) + ": empty value");

        if(key == "lat"){
            data.lat_deg = parse_double("invalid lat in --read file", value.c_str());
            data.have_lat = true;
        }
        else if(key == "lon"){
            data.lon_deg = parse_double("invalid lon in --read file", value.c_str());
            data.have_lon = true;
        }
        else if(key == "alt"){
            data.alt_m = parse_double("invalid alt in --read file", value.c_str());
            data.have_alt = true;
        }
        else if(key == "date"){
            data.date_value = value;
            data.have_date = true;
        }
        else if(key == "time"){
            data.time_value = value;
            data.have_time = true;
        }
        else{
            throw std::invalid_argument("read file parse error at line " + std::to_string(line_number) + ": unknown key '" + key + "'");
        }
    }
} // ——— END OF read_config_file() ———————————————————————————————————————————



/// @brief Look up a location by case-insensitive, trimmed name.
/// @param file_path Location library to read.
/// @param location_name Name to find.
/// @param[in,out] data Receives matching coordinates; unspecified fields are retained.
/// @return True if a matching entry was found, false otherwise.
/// @throws std::invalid_argument If opening or parsing the library fails.
bool read_named_location_file(const char* file_path,
                              const char* location_name,
                              MInputFileData& data)
{
    std::ifstream input(file_path);
    if(!input)
        throw std::invalid_argument(std::string("cannot open read file: ") + file_path);

    const std::string wanted = to_lowercase(trim(location_name));
    std::string line;
    int line_number = 0;

    while(std::getline(input, line)){
        line_number++;

        std::string stripped = trim(line);
        if(stripped.empty())
            continue;

        if(stripped[0] == '#' || stripped[0] == ';')
            continue;

        std::size_t separator = stripped.find('=');
        if(separator == std::string::npos)
            throw std::invalid_argument("name file parse error at line " + std::to_string(line_number) + ": missing '='");

        std::string key = to_lowercase(trim(stripped.substr(0, separator)));
        std::string value = trim(stripped.substr(separator + 1));

        if(key != wanted)
            continue;

        std::size_t comma1 = value.find(',');
        if(comma1 == std::string::npos)
            throw std::invalid_argument("name file parse error at line " + std::to_string(line_number) + ": expected 'lat, lon[, alt]'");

        std::string lat_text = trim(value.substr(0, comma1));
        std::string rest = trim(value.substr(comma1 + 1));

        std::size_t comma2 = rest.find(',');
        std::string lon_text;
        std::string alt_text;

        if(comma2 == std::string::npos)
            lon_text = trim(rest);
        else{
            lon_text = trim(rest.substr(0, comma2));
            alt_text = trim(rest.substr(comma2 + 1));
        }

        if(lat_text.empty() || lon_text.empty())
            throw std::invalid_argument("name file parse error at line " + std::to_string(line_number) + ": empty latitude/longitude");

        data.lat_deg = parse_double("invalid latitude in name file", lat_text.c_str());
        data.lon_deg = parse_double("invalid longitude in name file", lon_text.c_str());
        data.have_lat = true;
        data.have_lon = true;

        if(!alt_text.empty()){
            data.alt_m = parse_double("invalid altitude in name file", alt_text.c_str());
            data.have_alt = true;
        }

        return true;
    }

    return false;
} // ——— END OF read_named_location_file() ———————————————————————————————————



/// @brief Recognize a trimmed, case-insensitive y or yes response.
/// @param answer User response to the confirmation prompt.
/// @return True for y or yes; false otherwise.
static bool is_yes_response(const std::string& answer)
{
    const std::string lowered = to_lowercase(trim(answer));
    return lowered == "y" || lowered == "yes";
} // ——— END OF is_yes_response() ————————————————————————————————————————————



/// @brief Format a named-location entry in degrees and metres.
/// @param location_name Name of the location.
/// @param lat_deg Geodetic latitude in decimal degrees.
/// @param lon_deg Longitude in decimal degrees, east positive.
/// @param have_alt Whether ellipsoidal height is included.
/// @param alt_m Ellipsoidal height in metres.
/// @return Location-library entry without a trailing newline.
static std::string format_location_line(const char* location_name,
                                        double lat_deg,
                                        double lon_deg,
                                        bool have_alt,
                                        double alt_m)
{
    char buffer[256];

    if(have_alt)
        std::snprintf(buffer, sizeof(buffer), "%s = %.8f, %.8f, %.1f", location_name, lat_deg, lon_deg, alt_m);
    else
        std::snprintf(buffer, sizeof(buffer), "%s = %.8f, %.8f", location_name, lat_deg, lon_deg);

    return std::string(buffer);
} // ——— END OF format_location_line() ———————————————————————————————————————



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
                                double alt_m)
{
    std::vector<std::string> lines;
    bool file_exists = false;
    {
        std::ifstream input(file_path);
        file_exists = static_cast<bool>(input);
        std::string line;
        while(std::getline(input, line))
            lines.push_back(line);
    }

    if(!file_exists || lines.empty()){
        lines.push_back("# names.ini");
        lines.push_back("# Location library format:");
        lines.push_back("#   name = lat, lon[, alt]");
        lines.push_back("#");
        lines.push_back("# Edit from CLI:");
        lines.push_back("#   ./bin/astro --name <name> --lat <lat> --lon <lon> [--alt <alt>]");
        lines.push_back("# If name exists, astro asks for confirmation, comments the old line,");
        lines.push_back("# and appends the new value.");
        lines.push_back("");
    }

    const std::string wanted = to_lowercase(trim(location_name));
    int existing_index = -1;
    std::string existing_line;

    for(std::size_t i = 0; i < lines.size(); ++i){
        std::string stripped = trim(lines[i]);
        if(stripped.empty())
            continue;

        if(stripped[0] == '#' || stripped[0] == ';')
            continue;

        std::size_t separator = stripped.find('=');
        if(separator == std::string::npos)
            continue;

        std::string key = to_lowercase(trim(stripped.substr(0, separator)));
        if(key == wanted){
            existing_index = static_cast<int>(i);
            existing_line = stripped;
            break;
        }
    }

    const std::string new_line = format_location_line(location_name, lat_deg, lon_deg, have_alt, alt_m);

    if(existing_index >= 0){
        printf("Location '%s' already exists:\n", location_name);
        printf("  %s\n", existing_line.c_str());
        printf("Update this location with:\n");
        printf("  %s\n", new_line.c_str());
        printf("Proceed? [y/N]: ");

        std::string answer;
        std::getline(std::cin, answer);

        if(!is_yes_response(answer)){
            printf("No changes made.\n");
            return false;
        }

        lines[existing_index] = "# " + existing_line;
        lines.insert(lines.begin() + existing_index + 1, new_line);
    }
    else{
        lines.push_back(new_line);
        printf("Added new location '%s'.\n", location_name);
    }

    std::ofstream output(file_path, std::ios::trunc);
    if(!output)
        throw std::invalid_argument(std::string("cannot write name file: ") + file_path);

    for(const std::string& line : lines)
        output << line << "\n";

    if(existing_index >= 0)
        printf("Location '%s' updated in %s\n", location_name, file_path);
    else
        printf("Location '%s' written to %s\n", location_name, file_path);

    return true;
} // ——— END OF upsert_named_location_file() —————————————————————————————————