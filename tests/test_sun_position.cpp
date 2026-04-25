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

    const int passed = tests_run - failures;
    printf("-------------------------\n");
    printf("Summary: %d/%d tests passed\n", passed, tests_run);

    if (failures == 0)
    {
        return 0;
    }

    return 1;
} // ——— END OF main() —————————————————————————————————————————————————————————
