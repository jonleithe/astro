/*
 * File: test_sun_position.cpp
 * Created: 2026-04-25
 * Author: jole
 * Description: Lightweight sanity tests for MSunPosition calculations.
 */

#include <cmath>
#include <cstdio>

#include "geo_point.h"
#include "sun_position.h"


static bool approx(double value, double expected, double tolerance)
{
    return std::fabs(value - expected) <= tolerance;
} // ——— END OF approx() ———————————————————————————————————————————————————————


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

    const int passed = tests_run - failures;
    printf("-------------------------\n");
    printf("Summary: %d/%d tests passed\n", passed, tests_run);

    if (failures == 0)
    {
        return 0;
    }

    return 1;
} // ——— END OF main() —————————————————————————————————————————————————————————
