/**
 * @file test_sun_position.cpp
 * Created: 2026-04-25
 * Author: jole
 * @brief Lightweight sanity tests for MSunPosition calculations.
 */

#include <cmath>
#include <cstdio>
#include <stdexcept>

#include "geo_point.h"
#include "sun_position.h"


/// @brief Compare two floating-point values within an absolute tolerance.
/// @param value Value or string to parse/format.
/// @param expected Expected floating-point value.
/// @param tolerance Maximum absolute difference allowed.
/// @return True when the absolute difference does not exceed tolerance.
static bool approx(double value, double expected, double tolerance)
{
    return std::fabs(value - expected) <= tolerance;
} // ——— END OF approx() ———————————————————————————————————————————————————————


/// @brief Run regression checks for sun position.
/// @return Zero on success; nonzero for a failed request or regression check.
int main()
{
    int failures = 0;
    int tests_run = 0;

    printf("Sun position sanity tests\n");
    printf("=========================\n");

    // Test 1: Equator at March equinox close to local noon should place Sun near zenith.
    {
        const char* test_name = "Equinox noon altitude at equator";
        tests_run++;

        MGeoPoint observer(0.0, 0.0, 0.0);
        MDateTimeUtc utc = {2026, 3, 20, 12, 0, 0};

        MSunHorizontalPosition sun = MSunPosition::calculate(observer, utc);

        if (!approx(sun.altitude_deg, 90.0, 3.0))
        {
            printf("[FAIL] %s\n", test_name);
            printf("       expected altitude near 90.000 deg (+/- 3.000), got %.3f deg\n", sun.altitude_deg);
            failures++;
        }
        else
        {
            printf("[PASS] %s\n", test_name);
        }
    }

    // Test 2: June solstice declination should be near +23.4 deg.
    {
        const char* test_name = "June solstice declination";
        tests_run++;

        MGeoPoint observer(45.0, 0.0, 0.0);
        MDateTimeUtc utc = {2026, 6, 21, 12, 0, 0};

        MSunHorizontalPosition sun = MSunPosition::calculate(observer, utc);

        if (!approx(sun.declination_deg, 23.44, 1.0))
        {
            printf("[FAIL] %s\n", test_name);
            printf("       expected declination near +23.440 deg (+/- 1.000), got %.3f deg\n", sun.declination_deg);
            failures++;
        }
        else
        {
            printf("[PASS] %s\n", test_name);
        }
    }

    // Test 3: Daily events should return polar-day/polar-night indicator when applicable.
    {
        const char* test_name = "Polar night has no sunrise/sunset";
        tests_run++;

        MGeoPoint observer(78.24490, 15.49369, 0.0); // Longyearbyen
        MDateTimeUtc utc_date = {2026, 12, 21, 0, 0, 0};

        MSunDailyEventsUtc events = MSunPosition::calculate_daily_events(observer, utc_date);

        if (events.has_sunrise_sunset)
        {
            printf("[FAIL] %s\n", test_name);
            printf("       expected no sunrise/sunset, but has_sunrise_sunset was true\n");
            failures++;
        }
        else
        {
            printf("[PASS] %s\n", test_name);
        }
    }

    // Published Ny-Alesund times for 23 October 2026, converted from UTC+2.
    // The source displays whole minutes, so allow one minute of tolerance.
    // https://www.timeanddate.com/sun/norway/ny-alesund?month=10&year=2026
    {
        tests_run++;
        MGeoPoint observer(78.92423, 11.92082, 8.0);
        const auto events = MSunPosition::calculate_daily_events(
            observer, {2026, 10, 23, 0, 0, 0});
        const bool passed = events.has_sunrise_sunset
            && approx(events.sunrise_utc_hours, 9.0 + 51.0 / 60.0, 1.0 / 60.0)
            && approx(events.sunset_utc_hours, 11.0 + 59.0 / 60.0, 1.0 / 60.0)
            && approx(events.solar_noon_utc_hours, 10.0 + 56.0 / 60.0, 1.0 / 60.0)
            && approx(events.declination_deg, -11.49, 0.01);
        printf("[%s] Ny-Alesund October 23 reference times\n", passed ? "PASS" : "FAIL");
        if (!passed) failures++;
    }

    // Verify the last short day and the transition into polar night.
    {
        tests_run++;
        MGeoPoint observer(78.92423, 11.92082, 8.0);
        const auto last_day = MSunPosition::calculate_daily_events(
            observer, {2026, 10, 24, 12, 30, 0});
        const auto night = MSunPosition::calculate_daily_events(
            observer, {2026, 10, 25, 0, 0, 0});
        const bool passed = last_day.has_sunrise_sunset
            && approx(last_day.sunrise_utc_hours, 10.0 + 29.0 / 60.0, 1.0 / 60.0)
            && approx(last_day.sunset_utc_hours, 11.0 + 21.0 / 60.0, 1.0 / 60.0)
            && !night.has_sunrise_sunset
            && night.sunrise_utc_hours == -1.0 && night.sunset_utc_hours == -1.0;
        printf("[%s] Ny-Alesund polar-night transition\n", passed ? "PASS" : "FAIL");
        if (!passed) failures++;
    }

    {
        tests_run++;
        const auto events = MSunPosition::calculate_daily_events(
            MGeoPoint(78.92423, 11.92082), {2026, 6, 21, 0, 0, 0});
        const bool passed = !events.has_sunrise_sunset
            && std::isfinite(events.solar_noon_utc_hours);
        printf("[%s] Polar day has no horizon crossings\n", passed ? "PASS" : "FAIL");
        if (!passed) failures++;
    }

    // Independent axis geometry checks catch rotation and subtraction errors.
    {
        tests_run++;
        const MGeoPoint observer(0.0, 90.0, 100.0);
        const auto origin = observer.to_ecef();
        const auto vector = MObserverVector::from_ecef(observer,
            {origin.x - 3000.0, origin.y + 12000.0, origin.z + 4000.0});
        const auto unit = vector.unit_direction();
        const auto zero = MObserverVector{0.0, 0.0, 0.0}.unit_direction();
        const bool ok = approx(vector.east, 3000.0, 1e-8)
            && approx(vector.north, 4000.0, 1e-8)
            && approx(vector.up, 12000.0, 1e-8)
            && approx(vector.magnitude(), 13000.0, 1e-8)
            && approx(unit.magnitude(), 1.0, 1e-14)
            && std::isnan(zero.east) && std::isnan(zero.north) && std::isnan(zero.up);
        printf("[%s] ENU axes, distance, normalization and zero vector\n", ok ? "PASS" : "FAIL");
        if (!ok) failures++;
    }
    {
        tests_run++;
        const MGeoPoint observer(45.0, 10.0, 0.0);
        const MDateTimeUtc utc{2026, 1, 3, 12, 0, 0};
        const auto winter = MSunPosition::calculate_vector(observer, utc);
        const auto summer = MSunPosition::calculate_vector(observer, {2026, 7, 4, 12, 0, 0});
        const auto elevated = MSunPosition::calculate_vector(MGeoPoint(45.0, 10.0, 1000.0), utc);
        const auto later = MSunPosition::calculate_vector(observer, {2026, 1, 3, 13, 0, 0});
        const auto angles = MSunPosition::calculate(observer, utc);
        constexpr double degrees = 180.0 / 3.14159265358979323846;
        const double altitude = std::atan2(winter.up, std::hypot(winter.east, winter.north)) * degrees;
        const bool ok = approx(winter.magnitude(), 147100000000.0, 100000000.0)
            && approx(summer.magnitude(), 152100000000.0, 100000000.0)
            && approx(elevated.east, winter.east, 0.001)
            && approx(elevated.north, winter.north, 0.001)
            && approx(elevated.up, winter.up - 1000.0, 0.001)
            && std::fabs(altitude - angles.altitude_deg) < 0.003
            && std::fabs(later.east - winter.east) > 1e9;
        printf("[%s] Solar seasonal distances, height, parallax and time evolution\n", ok ? "PASS" : "FAIL");
        if (!ok) failures++;
    }
    {
        tests_run++;
        bool rejected = false;
        try { MSunPosition::calculate_vector(MGeoPoint(0, 0), {2026, 2, 30, 0, 0, 0}); }
        catch (const std::invalid_argument&) { rejected = true; }
        printf("[%s] Vector rejects invalid UTC\n", rejected ? "PASS" : "FAIL");
        if (!rejected) failures++;
    }

    // Terrain mode uses observer-relative geometry. Moving an observer upward
    // must lower the Sun-centre altitude near the horizon (increased parallax).
    {
        tests_run++;
        const MDateTimeUtc utc{2026, 2, 25, 8, 56, 7};
        const auto low = MSunPosition::calculate_topocentric(MGeoPoint(78.92808, 11.87239, 8), utc);
        const auto high = MSunPosition::calculate_topocentric(MGeoPoint(78.92808, 11.87239, 100000), utc);
        bool ok = low.azimuth_deg >= 0 && low.azimuth_deg < 360
               && low.elevation_deg < 0 && low.elevation_deg > -1
               && low.slant_distance_m > 140e9 && low.slant_distance_m < 160e9
               && high.elevation_deg < low.elevation_deg;
        printf("[%s] Topocentric solar direction includes observer height\n", ok ? "PASS" : "FAIL");
        if(!ok) ++failures;
    }

    const int passed = tests_run - failures;
    printf("-------------------------\n");
    printf("Summary: %d/%d tests passed\n", passed, tests_run);

    if (failures == 0)
    {
        return 0;
    }

    return 1;
} // ——— END OF main() —————————————————————————————————————————————————————————
