/**
 * @file tui.h
 * @brief Entry point for the interactive terminal sky display.
 */

#pragma once

#include "cli_input.h"
#include "geo_point.h"



/// @brief Open the interactive sky chart until the user quits.
/// @param observer WGS-84 observer, including ellipsoidal height.
/// @param input Resolved starting UTC time, observer label and object selection.
/// @throws std::invalid_argument If the starting UTC calendar fields are invalid.
/// @throws std::runtime_error If TUI/terminal support or UTC time conversion fails.
void run_tui(const MGeoPoint& observer, const MResolvedCliInput& input);
