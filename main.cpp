/**
 * @file main.cpp
 * Created: 2026-04-24
 * Author: jole
 * @brief CLI orchestration for observer geometry and solar/lunar displays.
 */

#include <cstdio>
#include <stdexcept>

#include "cli_input.h"
#include "geo_point.h"
#include "cli_file_io.h"
#include "print_output.h"
#include "sun_position.h"
#include "tui.h"



/// @brief Resolve CLI input and dispatch location editing, text output or TUI.
/// @param argc Number of command-line arguments.
/// @param argv Command-line argument strings.
/// @return Zero on success; nonzero for a failed request or regression check.
int main(int argc, char** argv)
{
    try{
        MCliOptions options = parse_cli_options(argc, argv);

        if(options.action == MCliAction::PrintHelp){
            print_usage(argv[0]);
            return 0;
        }
        if(options.action == MCliAction::PrintUsageError){
            print_usage(argv[0]);
            return 1;
        }

        MResolvedCliInput resolved = resolve_cli_input(options);

        if(resolved.edit_name_mode){
            upsert_named_location_file(resolved.name_file_arg.c_str(),
                                       options.name_arg.c_str(),
                                       options.lat_deg,
                                       options.lon_deg,
                                       options.have_alt,
                                       options.alt_m);
            return 0;
        }

        MGeoPoint observer(resolved.lat_deg, resolved.lon_deg, resolved.alt_m);
        if(options.tui){
            run_tui(observer, resolved);
            return 0;
        }
        if(resolved.show_sun){
            MSunHorizontalPosition sun_now = MSunPosition::calculate(observer, resolved.utc_time);
            MSunDailyEventsUtc sun_daily = MSunPosition::calculate_daily_events(observer, resolved.utc_time);

            print_output(observer,
                         resolved.observer_name.empty() ? nullptr : resolved.observer_name.c_str(),
                         resolved.utc_time,
                         sun_now,
                         sun_daily);
        }
        if(resolved.show_moon){
            auto moon = MMoonPosition::calculate(observer, resolved.utc_time);
            print_moon_output(observer,
                              resolved.observer_name.empty() ? nullptr : resolved.observer_name.c_str(),
                              resolved.utc_time, moon, !resolved.show_sun);
        }

    } // ——— END OF try block ——————————————————————————————————————————————————

    catch (const std::exception& ex){
        printf("Error: %s\n", ex.what());
        print_usage(argv[0]);
        return 1;
    } // ——— END OF catch block ————————————————————————————————————————————————

    return 0;
} // ——— END OF main() —————————————————————————————————————————————————————————
