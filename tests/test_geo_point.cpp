#include "geo_point.h"

#include <cmath>
#include <cstdio>

int main()
{
    int failures = 0;
    const auto check = [&](bool condition, const char* name) {
        if (!condition)
        {
            std::printf("[FAIL] %s\n", name);
            ++failures;
        }
    };
    const auto near = [](double a, double b, double tolerance = 1e-7) {
        return std::fabs(a - b) <= tolerance;
    };

    // Analytic equatorial geometry: a 2-degree arc subtends a -1-degree
    // elevation and a chord of 2*a*sin(1 degree).
    const MGeoPoint origin(0.0, 0.0);
    const auto east = origin.direction_to(MGeoPoint(0.0, 2.0));
    check(near(east.azimuth_deg, 90.0), "east azimuth");
    check(near(east.elevation_deg, -1.0), "curvature produces negative elevation");
    check(near(east.slant_distance_m,
               2.0 * 6378137.0 * std::sin(std::acos(-1.0) / 180.0)),
          "distance is chord rather than surface arc");
    check(near(origin.direction_to(MGeoPoint(0.0, -2.0)).azimuth_deg, 270.0),
          "west azimuth wraps to positive degrees");
    check(near(origin.direction_to(MGeoPoint(2.0, 0.0)).azimuth_deg, 0.0),
          "north azimuth");
    check(near(origin.direction_to(MGeoPoint(-2.0, 0.0)).azimuth_deg, 180.0),
          "south azimuth");

    const MGeoPoint observer(67.0, 23.0, 200.0);
    const auto up = observer.direction_to(MGeoPoint(67.0, 23.0, 300.0));
    const auto down = observer.direction_to(MGeoPoint(67.0, 23.0, 100.0));
    check(near(up.elevation_deg, 90.0) && near(up.slant_distance_m, 100.0),
          "vertical target above non-equatorial observer");
    check(near(down.elevation_deg, -90.0) && near(down.slant_distance_m, 100.0),
          "vertical target below observer");
    check(up.azimuth_deg == 0.0, "vertical azimuth convention");
    const auto same = observer.direction_to(observer);
    check(std::isnan(same.azimuth_deg) && std::isnan(same.elevation_deg)
          && same.slant_distance_m == 0.0, "coincident direction is undefined");

    // At 90E, local east points towards decreasing ECEF X.
    const auto rotated = MGeoPoint(0.0, 90.0).direction_to(MGeoPoint(0.0, 92.0));
    check(near(rotated.azimuth_deg, east.azimuth_deg)
          && near(rotated.elevation_deg, east.elevation_deg), "rotated local frame");
    const auto dateline = MGeoPoint(0.0, 179.0).direction_to(MGeoPoint(0.0, -179.0));
    check(near(dateline.azimuth_deg, 90.0) && near(dateline.elevation_deg, -1.0),
          "dateline crossing");
    return failures == 0 ? 0 : 1;
}
