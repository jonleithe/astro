/*
 * File: print_output.h
 * Created: 2026-04-25
 * Author: jole
 * Description: Declarations for CLI usage and result-printing helpers.
 */

#pragma once

#include "geo_point.h"
#include "sun_position.h"


void print_usage(const char* argv0);

void print_output(const MGeoPoint& observer,
                  const char* observer_name,
                  const MDateTimeUtc& utc_time,
                  const MSunHorizontalPosition& sun_now,
                  const MSunDailyEventsUtc& sun_daily);