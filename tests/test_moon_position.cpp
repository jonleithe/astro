/**
 * @file test_moon_position.cpp
 * @brief Regression checks for lunar position, parallax and phase.
 */

#include "moon_position.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdexcept>

/// @brief Run regression checks for moon position.
/// @return Zero on success; nonzero for a failed request or regression check.
int main()
{
    int failures = 0;
    auto check = [&](bool ok, const char* name){
        std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", name);
        if(!ok) ++failures;
    };
    // Published topocentric RA/Dec fixture, converted independently to ENU:
    // https://stjarnhimlen.se/comp/tutorial.html section 9
    // 1990-04-19 00:00 UT, 60 N 15 E; LST 221.8388 deg,
    // topocentric RA 310.0017 deg, declination -19.8790 deg.
    constexpr double rad = 3.14159265358979323846 / 180;
    const double hour = (221.8388 - 310.0017) * rad;
    const double dec = -19.8790 * rad;
    const double east = -std::cos(dec) * std::sin(hour);
    const double north = std::cos(60 * rad) * std::sin(dec)
                       - std::sin(60 * rad) * std::cos(dec) * std::cos(hour);
    const double up = std::sin(60 * rad) * std::sin(dec)
                    + std::cos(60 * rad) * std::cos(dec) * std::cos(hour);
    const auto reference = MMoonPosition::calculate(MGeoPoint(60, 15), {1990, 4, 19, 0, 0, 0});
    check(std::abs(reference.azimuth_deg - std::atan2(east, north) / rad) < 0.06,
          "Published lunar fixture azimuth");
    check(std::abs(reference.altitude_deg - std::atan2(up, std::hypot(east, north)) / rad) < 0.06,
          "Published lunar fixture altitude including parallax");
    MGeoPoint observer(78.92808, 11.87239, 8);
    const auto moon = MMoonPosition::calculate(observer, {2026, 2, 25, 8, 56, 7});
    const auto later = MMoonPosition::calculate(observer, {2026, 2, 25, 9, 56, 7});
    check(std::abs(moon.azimuth_deg - later.azimuth_deg) > 1, "Moon moves when stepping time");
    for(double lat : {-90., 0., 90.}){
        for(int month = 1; month <= 12; ++month){
            const auto value = MMoonPosition::calculate(MGeoPoint(lat, -180), {2026, static_cast<short>(month), 15, 0, 0, 0});
            check(std::isfinite(value.altitude_deg) && value.altitude_deg >= -90 && value.altitude_deg <= 90
                  && value.azimuth_deg >= 0 && value.azimuth_deg < 360
                  && value.distance_m > 340e6 && value.distance_m < 420e6, "Finite polar/equatorial positions and lunar distance");
        }
    }
    const auto high = MMoonPosition::calculate(MGeoPoint(78.92808, 11.87239, 100000), {2026, 2, 25, 8, 56, 7});
    check(std::abs(high.altitude_deg - moon.altitude_deg) > 0.001, "Observer height affects lunar parallax");
    bool rejected = false;
    try { MMoonPosition::calculate(observer, {2026, 2, 30, 0, 0, 0}); }
    catch(const std::invalid_argument&) { rejected = true; }
    check(rejected, "Invalid UTC rejected");
    // Primary phase fixtures from Fred Espenak's published 2026 ephemeris:
    // https://astropixels.com/ephemeris/moon/phases2001.html
    struct PhaseFixture { MDateTimeUtc utc; double fraction; const char* name; };
    const PhaseFixture phases[] = {
        {{2026, 2, 17, 12, 1, 0}, 0, "New Moon"},
        {{2026, 2, 24, 12, 28, 0}, .5, "First quarter"},
        {{2026, 3, 3, 11, 38, 0}, 1, "Full Moon"},
        {{2026, 3, 11, 9, 39, 0}, .5, "Last quarter"}
    };
    for(const auto& fixture : phases){
        const auto value = MMoonPosition::calculate(observer, fixture.utc);
        check(std::abs(value.illuminated_fraction - fixture.fraction) < .01,
              "Published primary-phase illumination within one percentage point");
        check(std::strcmp(value.phase_name(), fixture.name) == 0, "Primary phase name");
        const auto elsewhere = MMoonPosition::calculate(MGeoPoint(-60, -90, 1000), fixture.utc);
        check(value.illuminated_fraction == elsewhere.illuminated_fraction,
              "Geocentric phase independent of observer");
    }
    const char* names[] = {"New Moon", "Waxing crescent", "First quarter", "Waxing gibbous",
                          "Full Moon", "Waning gibbous", "Last quarter", "Waning crescent"};
    for(int index = 0; index < 8; ++index){
        MMoonHorizontalPosition value{};
        value.phase_cycle_deg = index * 45.;
        check(std::strcmp(value.phase_name(), names[index]) == 0, "All eight phase names");
    }
    MMoonHorizontalPosition band{};
    for(double angle : {4.999, 5., 84.999, 85., 94.999, 95., 174.999, 175., 184.999, 185., 264.999, 265., 274.999, 275., 354.999, 355.}){
        band.phase_cycle_deg = angle;
        int category = angle < 5 || angle >= 355 ? 0 : angle < 85 ? 1 : angle < 95 ? 2
                     : angle < 175 ? 3 : angle < 185 ? 4 : angle < 265 ? 5 : angle < 275 ? 6 : 7;
        check(std::strcmp(band.phase_name(), names[category]) == 0, "Phase naming boundary");
    }
    const auto waxing = MMoonPosition::calculate(observer, {2026, 2, 25, 0, 0, 0});
    const auto waning = MMoonPosition::calculate(observer, {2026, 3, 6, 0, 0, 0});
    const auto next_hour = MMoonPosition::calculate(observer, {2026, 2, 25, 1, 0, 0});
    check(waxing.waxing && !waning.waxing, "Waxing versus waning");
    check(next_hour.illuminated_fraction > waxing.illuminated_fraction
          && next_hour.illuminated_fraction - waxing.illuminated_fraction < .02,
          "Illumination changes continuously when stepping time");
    return failures ? 1 : 0;
}
