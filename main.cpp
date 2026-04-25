
/*
 * File: main.cpp
 * Created: 2026-04-24
 * Author: jole
 * Description: Entry point for constructing and printing sample MGeoPoint values.
 */

#include <cstdio>

#include "geo_point.h"


int main()
{
       printf("--- MGeoPoint ---\n");

    // Stockholm and Oslo: known great-circle distance ~415 km
    MGeoPoint stockholm( 59.3293,  18.0686, 0.0);
    MGeoPoint oslo      ( 59.9139,  10.7522, 0.0);

    printf("Stockholm  -> lat = %.4f deg, lon = %.4f deg, alt = %.1f m\n",
           stockholm.get_lat(), stockholm.get_lon(), stockholm.get_alt());

    double dist_m = MGeoPoint::haversine_distance(stockholm, oslo);
    printf("Haversine distance Stockholm -> Oslo: %.1f km\n", dist_m / 1000.0);

       MEcefPoint ecef = stockholm.to_ecef();
       printf("ECEF (double) -> x = %.1f m, y = %.1f m, z = %.1f m\n",
           ecef.x, ecef.y, ecef.z);

    MGeoPoint roundtrip = MGeoPoint::from_ecef(ecef.x, ecef.y, ecef.z);
    printf("Roundtrip  -> lat = %.4f deg, lon = %.4f deg\n",
           roundtrip.get_lat(), roundtrip.get_lon());
}