/**
 * @file terrain_horizon.cpp
 * @brief ASCII raster loading and sampled WGS-84 observer skyline geometry.
 */
#include "terrain_horizon.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <cmath>
#include <fstream>
#include <sstream>
#include <map>

namespace {
/// @brief Radians per degree.
constexpr double rad = 3.14159265358979323846 / 180;

/// @brief Regular longitude/latitude raster with north-to-south row storage.
struct Raster
{
    int cols = 0; ///< Raster column count.
    int rows = 0; ///< Raster row count.
    double west = 0; ///< Western cell edge in degrees.
    double south = 0; ///< Southern cell edge in degrees.
    double cell = 0; ///< Cell angular size in degrees.
    double nodata = -9999; ///< Missing elevation sentinel.
    std::vector<double> heights; ///< Ellipsoidal heights in metres.

    /// @brief Sample the cell containing a longitude/latitude point.
    /// @param lat Latitude in degrees.
    /// @param lon Longitude in degrees.
    /// @return Cell height in metres, or NaN for absent/masked data.
    double sample(double lat, double lon) const
    {
        // Permit a continuous longitude grid across the date line.
        lon += 360 * std::round((west + cols * cell / 2 - lon) / 360);
        double x = (lon - west) / cell;
        double y = (south + rows * cell - lat) / cell;
        if(x < 0 || y < 0 || x >= cols || y >= rows)
            return std::numeric_limits<double>::quiet_NaN();
        double height = heights[static_cast<int>(y) * cols + static_cast<int>(x)];
        return height == nodata ? std::numeric_limits<double>::quiet_NaN() : height;
    } // ——— END OF Raster::sample() ———————————————————————————————————————————
};



/// @brief Read a bounded ESRI ASCII grid with explicit geographic coordinates.
/// @param path Raster filename.
/// @return Parsed grid; CRS and vertical datum are the caller's input contract.
/// @throws std::invalid_argument If the raster is malformed or too large.
Raster read_raster(const std::string& path)
{
    std::ifstream input(path);
    if(!input) throw std::invalid_argument("cannot open terrain DEM: " + path);
    Raster raster;
    std::map<std::string, double> header;
    std::string line;
    std::streampos data_start{};
    while(true){
        data_start = input.tellg();
        if(!std::getline(input, line)) throw std::invalid_argument("terrain DEM has no raster data");
        std::istringstream fields(line);
        std::string key;
        double value;
        fields >> key;
        if(key.empty()) continue;
        for(char& ch : key) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        if(key != "ncols" && key != "nrows" && key != "xllcorner" && key != "yllcorner"
           && key != "xllcenter" && key != "yllcenter" && key != "cellsize" && key != "nodata_value"){
            input.clear();
            input.seekg(data_start);
            break;
        }
        std::string extra;
        if(!(fields >> value) || !std::isfinite(value) || (fields >> extra) || header.count(key))
            throw std::invalid_argument("invalid terrain DEM header: " + key);
        header[key] = value;
    }
    auto required = [&](const char* key){
        auto found = header.find(key);
        if(found == header.end()) throw std::invalid_argument(std::string("missing terrain DEM header: ") + key);
        return found->second;
    };
    double cols = required("ncols"), rows = required("nrows");
    if(cols < 1 || rows < 1 || cols > 100000 || rows > 100000 || std::floor(cols) != cols
       || std::floor(rows) != rows || cols * rows > 10000000)
        throw std::invalid_argument("terrain DEM must contain 1 to 10000000 cells");
    raster.cols = static_cast<int>(cols);
    raster.rows = static_cast<int>(rows);
    raster.cell = required("cellsize");
    if(raster.cell <= 0 || raster.cell > 1)
        throw std::invalid_argument("terrain cellsize must be in longitude/latitude degrees (0, 1]");
    if(header.count("xllcenter") == header.count("xllcorner")
       || header.count("yllcenter") == header.count("yllcorner"))
        throw std::invalid_argument("terrain DEM needs exactly one lower-left x and y origin");
    raster.west = header.count("xllcorner") ? header.at("xllcorner") : header.at("xllcenter") - raster.cell / 2;
    raster.south = header.count("yllcorner") ? header.at("yllcorner") : header.at("yllcenter") - raster.cell / 2;
    if(raster.south < -90 || raster.south + rows * raster.cell > 90
       || raster.west < -180 || raster.west > 180 || cols * raster.cell > 360)
        throw std::invalid_argument("terrain DEM bounds must be WGS-84 longitude/latitude degrees");
    if(header.count("nodata_value")) raster.nodata = header.at("nodata_value");
    raster.heights.resize(raster.cols * raster.rows);
    for(double& height : raster.heights){
        if(!(input >> height) || !std::isfinite(height))
            throw std::invalid_argument("terrain DEM has missing or nonfinite raster values");
    }
    std::string extra;
    if(input >> extra) throw std::invalid_argument("terrain DEM has excess raster values");
    return raster;
} // ——— END OF read_raster() ——————————————————————————————————————————————————
} // namespace



/// @brief Load an ASCII DEM and build the observer skyline.
/// @param path Local ESRI ASCII raster; heights must be WGS-84 ellipsoidal metres.
/// @param observer WGS-84 eye/instrument position, not ground height.
/// @param radius_m Search radius in metres, [100, 100000].
/// @param step_m Radial spacing in metres, [10, radius_m].
/// @return Cached 720-ray profile, including missing-data flags.
/// @throws std::invalid_argument For malformed raster, parameters or observer.
/// @note Ray locations use spherical forward sampling; angles and curvature
/// use WGS-84 ECEF. This finite-resolution profile is not an exact skyline.
MTerrainHorizon MTerrainHorizon::load_ascii(const std::string& path, const MGeoPoint& observer,
                                          double radius_m, double step_m)
{
    if(!std::isfinite(radius_m) || !std::isfinite(step_m) || radius_m < 100
       || radius_m > 100000 || step_m < 10 || step_m > radius_m)
        throw std::invalid_argument("terrain radius must be 100..100000 m; step must be 10..radius m");
    if(!std::isfinite(observer.get_lat()) || !std::isfinite(observer.get_lon()) || !std::isfinite(observer.get_alt()))
        throw std::invalid_argument("terrain observer coordinates must be finite");
    auto raster = read_raster(path);
    double ground = raster.sample(observer.get_lat(), observer.get_lon());
    if(!std::isfinite(ground)) throw std::invalid_argument("terrain DEM has no data at observer position");
    if(observer.get_alt() < ground)
        throw std::invalid_argument("observer is below DEM ground: --alt must be ellipsoidal eye/instrument height");
    MTerrainHorizon horizon;
    horizon.radius_m = radius_m;
    horizon.step_m = step_m;
    horizon.source = path;
    horizon.rays.resize(720);
    double lat = observer.get_lat() * rad, lon = observer.get_lon() * rad;
    int count = static_cast<int>(std::ceil(radius_m / step_m));
    for(std::size_t bin = 0; bin < horizon.rays.size(); ++bin){
        double bearing = bin * (360.0 / horizon.rays.size()) * rad;
        auto& ray = horizon.rays[bin];
        for(int sample = 1; sample <= count; ++sample){
            double angle = std::min(sample * step_m, radius_m) / 6371008.8;
            double target_lat = std::asin(std::clamp(std::sin(lat) * std::cos(angle)
                + std::cos(lat) * std::sin(angle) * std::cos(bearing), -1.0, 1.0));
            double target_lon = lon + std::atan2(std::sin(bearing) * std::sin(angle) * std::cos(lat),
                                                std::cos(angle) - std::sin(lat) * std::sin(target_lat));
            double longitude = std::remainder(target_lon / rad, 360.0);
            double height = raster.sample(target_lat / rad, longitude);
            if(!std::isfinite(height)){
                ray.complete = false;
                continue;
            }
            MGeoPoint terrain(target_lat / rad, longitude, height);
            ray.altitude_deg = std::max(ray.altitude_deg, observer.direction_to(terrain).elevation_deg);
        }
    }
    return horizon;
} // ——— END OF MTerrainHorizon::load_ascii() ——————————————————————————————————



/// @brief Look up the sampled obstruction at an azimuth, wrapping north.
/// @param azimuth_deg Finite clockwise bearing in degrees.
/// @return Highest neighbouring ray angle and combined coverage status.
/// @throws std::invalid_argument For nonfinite bearing or an empty profile.
MTerrainDirection MTerrainHorizon::at(double azimuth_deg) const
{
    if(rays.empty() || !std::isfinite(azimuth_deg))
        throw std::invalid_argument("terrain lookup requires a profile and finite azimuth");
    double wrapped = azimuth_deg - 360 * std::floor(azimuth_deg / 360);
    double index = wrapped * rays.size() / 360;
    std::size_t left = static_cast<std::size_t>(std::floor(index)) % rays.size();
    std::size_t right = (left + 1) % rays.size();
    // No smoothing downwards: the higher neighbouring sample is used.
    return {std::max(rays[left].altitude_deg, rays[right].altitude_deg),
            rays[left].complete && rays[right].complete};
} // ——— END OF MTerrainHorizon::at() ——————————————————————————————————————————
