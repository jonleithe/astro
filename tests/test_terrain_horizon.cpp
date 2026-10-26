/**
 * @file test_terrain_horizon.cpp
 * @brief Synthetic terrain profile geometry, missing-data and input checks.
 */
#include "terrain_horizon.h"

#include <cmath>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <functional>

/// @brief Write a small north-up geographic raster fixture.
/// @param path Temporary output file.
/// @param missing Whether to mask the northern half of the raster.
/// @param ridge Whether to add a raised northern ridge.
void write_grid(const char* path, bool missing, bool ridge)
{
    std::ofstream out(path);
    out << "ncols 81\nnrows 81\nxllcenter -0.04\nyllcenter -0.04\ncellsize 0.001\nNODATA_value -9999\n";
    for(int row = 0; row < 81; ++row){
        for(int col = 0; col < 81; ++col)
            out << (missing && row < 30 ? -9999 : ridge && row >= 17 && row <= 20 ? 500 : 0) << ' ';
        out << '\n';
    }
} // ——— END OF write_grid() ———————————————————————————————————————————————————



/// @brief Exercise sampled skyline geometry and error handling.
/// @return Zero when all checks pass, nonzero otherwise.
int main()
{
    int failures = 0;
    auto check = [&](bool ok, const char* name){
        std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", name);
        if(!ok) ++failures;
    };
    auto rejects = [&](const std::function<void()>& action){
        try { action(); } catch(const std::invalid_argument&) { return true; }
        return false;
    };
    const char* path = "terrain_test.asc";
    MGeoPoint observer(0, 0, 2);
    write_grid(path, false, false);
    const auto flat = MTerrainHorizon::load_ascii(path, observer, 3000, 100);
    check(flat.rays.size() == 720, "Uniform half-degree profile");
    check(flat.at(0).complete && flat.at(180).complete, "Complete raster coverage");
    check(flat.at(0).altitude_deg < 0 && flat.at(0).altitude_deg > -.2,
          "Depressed geometric horizon is preserved");
    check(flat.at(360).altitude_deg == flat.at(0).altitude_deg
          && flat.at(-.1).altitude_deg == flat.at(359.9).altitude_deg, "North wrapping");
    const auto elevated = MTerrainHorizon::load_ascii(path, MGeoPoint(0, 0, 100), 3000, 100);
    check(elevated.at(0).altitude_deg < flat.at(0).altitude_deg, "Observer height lowers terrain skyline");
    write_grid(path, false, true);
    const auto ridge = MTerrainHorizon::load_ascii(path, observer, 3000, 100);
    check(ridge.at(0).altitude_deg > 10 && ridge.at(0).altitude_deg < 15, "North ridge angle");
    check(ridge.at(180).altitude_deg < 0, "Opposite direction unaffected by ridge");
    write_grid(path, true, false);
    const auto gaps = MTerrainHorizon::load_ascii(path, observer, 3000, 100);
    check(!gaps.at(0).complete && gaps.at(180).complete, "Masked cells mark only affected rays unknown");
    const auto edge = MTerrainHorizon::load_ascii(path, observer, 10000, 100);
    check(!edge.at(90).complete, "Search outside DEM marks coverage incomplete");
    check(rejects([&]{ MTerrainHorizon::load_ascii(path, observer, 3000, 0); }), "Zero step rejected");
    check(rejects([&]{ MTerrainHorizon::load_ascii(path, observer, NAN, 100); }), "Nonfinite radius rejected");
    check(rejects([&]{ MTerrainHorizon::load_ascii(path, MGeoPoint(0, 0, -1), 3000, 100); }), "Observer below ground rejected");
    check(rejects([&]{ MTerrainHorizon::load_ascii(path, MGeoPoint(10, 10), 3000, 100); }), "Observer outside DEM rejected");
    check(rejects([&]{ MTerrainHorizon{}.at(0); }), "Empty profile rejected");
    check(rejects([&]{ flat.at(NAN); }), "Nonfinite bearing rejected");
    { std::ofstream bad(path); bad << "ncols 2\nnrows 2\nxllcorner 0\nyllcorner 0\ncellsize 0.01\n1 2 3\n"; }
    check(rejects([&]{ MTerrainHorizon::load_ascii(path, observer); }), "Truncated raster rejected");
    { std::ofstream bad(path); bad << "ncols 2\nnrows 2\nxllcorner 500000\nyllcorner 8000000\ncellsize 10\n1 2 3 4\n"; }
    check(rejects([&]{ MTerrainHorizon::load_ascii(path, observer); }), "Projected coordinates rejected");
    std::remove(path);
    return failures ? 1 : 0;
} // ——— END OF main() —————————————————————————————————————————————————————————
