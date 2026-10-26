/**
 * @file tui.cpp
 * @brief Interactive terminal sky chart, lunar phase and time controls.
 */

#include "tui.h"
#include "moon_position.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <string>

#ifdef ASTRO_HAVE_CURSES
#include <curses.h>
#include <unistd.h>

namespace {

chtype sun_style = A_NORMAL;
chtype moon_style = A_NORMAL;

/// @brief Own the curses session and restore terminal settings on scope exit.
struct Terminal
{
    /// @brief Initialize terminal input, drawing and optional object colours.
    /// @throws std::runtime_error If input/output are not terminals, TERM is
    /// unusable, or curses initialization fails.
    /// @note Terminals without colour support use uncoloured object markers.
    Terminal()
    {
        if(!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
            throw std::runtime_error("--tui requires an interactive terminal");
        const char* term = std::getenv("TERM");
        if(!term || std::string(term) == "dumb")
            throw std::runtime_error("--tui requires a usable TERM setting");
        if(!initscr())
            throw std::runtime_error("cannot initialize terminal");
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        if(has_colors() && start_color() != ERR){
            // Preserve the terminal background when supported by ncurses.
            short background = use_default_colors() == OK ? -1 : COLOR_BLACK;
            if(init_pair(1, COLOR_YELLOW, background) != ERR)
                sun_style = COLOR_PAIR(1);
            if(init_pair(2, COLOR_WHITE, background) != ERR)
                moon_style = COLOR_PAIR(2);
        }
    } // ——— END OF Terminal() —————————————————————————————————————————————————



    /// @brief Restore terminal settings on normal exit or exception unwinding.
    ~Terminal()
    {
        endwin();
    } // ——— END OF ~Terminal() ————————————————————————————————————————————————
};



/// @brief Draw a string within the terminal bounds.
/// @param y Zero-based terminal row, increasing downwards.
/// @param x Zero-based terminal column, increasing to the right.
/// @param value Text to display, clipped to the available columns.
/// @note Out-of-bounds starting positions are ignored; the final column is
/// left free to avoid wrapping at the terminal edge.
void text(int y, int x, const std::string& value)
{
    if(y >= 0 && y < LINES && x >= 0 && x < COLS)
        mvaddnstr(y, x, value.c_str(), COLS - x - 1);
} // ——— END OF text() —————————————————————————————————————————————————————————



/// @brief Format a UTC event time with its offset from the selected date.
/// @param hours Event time in decimal UTC hours relative to the selected date.
/// May be negative or exceed 24; supply a finite value within long's range
/// after conversion to seconds.
/// @return Time rounded to seconds as HH:MM:SS UTC with a signed day offset.
std::string event_time(double hours)
{
    long seconds = std::lround(hours * 3600.0);
    long day = static_cast<long>(std::floor(seconds / 86400.0));
    seconds -= day * 86400;
    char result[80];
    std::snprintf(result, sizeof(result), "%02ld:%02ld:%02ld UTC (%+ldd)",
                  seconds / 3600, seconds / 60 % 60, seconds % 60, day);
    return result;
} // ——— END OF event_time() ———————————————————————————————————————————————————



/// @brief Step UTC time and normalize calendar fields across date boundaries.
/// @param utc Valid starting UTC calendar date and time.
/// @param minutes Signed number of minutes to add.
/// @return Normalized UTC date and time, preserving the seconds field.
/// @throws std::runtime_error If conversion back to UTC calendar fields fails.
/// @note Uses the platform time_t range and is independent of local timezone.
MDateTimeUtc shift_time(MDateTimeUtc utc, int minutes)
{
    // Normalize in UTC, independently of the machine's local timezone.
    std::tm calendar{};
    calendar.tm_year = utc.year - 1900;
    calendar.tm_mon = utc.month - 1;
    calendar.tm_mday = utc.day;
    calendar.tm_hour = utc.hour;
    calendar.tm_min = utc.minute + minutes;
    calendar.tm_sec = utc.second;
    std::time_t stamp = timegm(&calendar);
    std::tm normalized{};
    if(!gmtime_r(&stamp, &normalized))
        throw std::runtime_error("time step outside supported calendar range");
    return {normalized.tm_year + 1900, static_cast<short>(normalized.tm_mon + 1),
            static_cast<short>(normalized.tm_mday), static_cast<short>(normalized.tm_hour),
            static_cast<short>(normalized.tm_min), static_cast<short>(normalized.tm_sec)};
} // ——— END OF shift_time() ———————————————————————————————————————————————————



/// @brief Calculate selected objects and refresh the terminal display.
/// @param observer WGS-84 observer location, including ellipsoidal height.
/// @param name Optional observer label; an empty string omits the label.
/// @param utc UTC date and time used for positions, phase and solar events.
/// @param show_sun Whether to draw the Sun and its daily event information.
/// @param show_moon Whether to draw the Moon and its schematic phase disc.
/// @throws std::invalid_argument If a selected calculation rejects the UTC input.
/// @note Requires an active curses session. Below 76 columns by 36 rows,
/// shows a resize prompt. Phase orientation is schematic, not observer-oriented.
void draw(const MGeoPoint& observer,
          const std::string& name,
          const MDateTimeUtc& utc,
          bool show_sun,
          bool show_moon)
{
    auto sun = show_sun ? MSunPosition::calculate(observer, utc) : MSunHorizontalPosition{};
    auto moon = show_moon ? MMoonPosition::calculate(observer, utc) : MMoonHorizontalPosition{};
    auto daily = show_sun ? MSunPosition::calculate_daily_events(observer, utc) : MSunDailyEventsUtc{};
    erase();
    if(COLS < 76 || LINES < 36){
        text(0, 0, "astro: enlarge terminal to at least 76 columns x 36 rows");
        text(2, 0, "q quit | arrows step time | r reset");
        refresh();
        return;
    }
    char line[256];
    text(0, 2, "ASTRO / SUN & MOON                                      UTC EXPLORER");
    std::snprintf(line, sizeof(line), "%04d-%02d-%02d  %02d:%02d:%02d UTC",
                  utc.year, utc.month, utc.day, utc.hour, utc.minute, utc.second);
    text(2, 2, line);
    std::snprintf(line, sizeof(line), "Observer: %.5f, %.5f | h %.1f m", observer.get_lat(),
                  observer.get_lon(), observer.get_alt());
    text(3, 2, line);
    if(!name.empty()) text(4, 2, name);

    const int radius = std::min((LINES - 21) / 2, (COLS - 40) / 4);
    const int cx = 2 * radius + 4, cy = radius + 7;
    constexpr double pi = 3.14159265358979323846;
    for(int deg = 0; deg < 360; ++deg){
        double angle = deg * pi / 180.0;
        mvaddch(cy - std::lround(radius * std::cos(angle)),
                cx + std::lround(2 * radius * std::sin(angle)), '.');
    }
    text(cy - radius - 1, cx, "N");
    text(cy + radius + 1, cx, "S");
    text(cy, cx - 2 * radius - 2, "W");
    text(cy, cx + 2 * radius + 2, "E");
    mvaddch(cy, cx, '+');
    if(show_sun && sun.altitude_deg >= 0){
        double r = radius * (90.0 - sun.altitude_deg) / 90.0;
        double az = sun.azimuth_deg * pi / 180.0;
        mvaddch(cy - std::lround(r * std::cos(az)),
                cx + std::lround(2 * r * std::sin(az)), '*' | sun_style);
    }
    if(show_moon && moon.altitude_deg >= 0){
        double r = radius * (90.0 - moon.altitude_deg) / 90.0;
        double az = moon.azimuth_deg * pi / 180.0;
        int y = cy - std::lround(r * std::cos(az));
        int x = cx + std::lround(2 * r * std::sin(az));
        // A shared terminal cell is marked explicitly, rather than hiding a body.
        bool overlap = (mvinch(y, x) & A_CHARTEXT) == '*';
        mvaddch(y, x, (overlap ? '@' : 'o') | moon_style);
    }
    int right = 4 * radius + 9;
    if(show_sun){
        text(6, right, "SUN / ");
        mvaddch(6, right + 6, '*' | sun_style);
        std::snprintf(line, sizeof(line), "Azimuth  %8.3f deg", sun.azimuth_deg);
        text(8, right, line);
        std::snprintf(line, sizeof(line), "Altitude %+8.3f deg", sun.altitude_deg);
        text(9, right, line);
        text(10, right, sun.altitude_deg < 0 ? "BELOW geometric horizon" : "ABOVE geometric horizon");
    }
    if(show_moon){
        text(12, right, "MOON / ");
        mvaddch(12, right + 7, 'o' | moon_style);
        std::snprintf(line, sizeof(line), "Az %6.2f  Alt %+6.2f deg", moon.azimuth_deg, moon.altitude_deg);
        text(13, right, line);
        text(14, right, moon.altitude_deg < 0 ? "BELOW geometric horizon" : "ABOVE geometric horizon");
    }
    if(show_moon){
        text(15, right, moon.phase_name());
        std::snprintf(line, sizeof(line), "%.1f%% illuminated", 100 * moon.illuminated_fraction);
        text(16, right, line);
        // Orthographic sphere: light direction is schematic, not sky orientation.
        // '#' is lit and '.' is dark, readable in monochrome terminals too.
        int top = 2 * radius + 10;
        double light_x = (moon.waxing ? 1 : -1)
            * std::sqrt(std::max(0.0, 1 - std::pow(2 * moon.illuminated_fraction - 1, 2)));
        double light_z = 2 * moon.illuminated_fraction - 1;
        for(int row = 0; row < 5; ++row){
            for(int col = 0; col < 11; ++col){
                double x = (col - 5) / 5.5;
                double y = (row - 2) / 2.5;
                double square = x * x + y * y;
                if(square <= 1){
                    bool lit = x * light_x + std::sqrt(1 - square) * light_z > 0;
                    mvaddch(top + row, 4 + col, (lit ? '#' : '.') | moon_style);
                }
            }
        }
        text(top, 17, "Phase schematic");
        text(top + 1, 17, "# lit / . dark");
        text(top + 3, 17, moon.waxing ? "Waxing: right lit" : "Waning: left lit");
    }
    text(17, right, "HORIZON ZOOM (+/-5 deg)");
    for(int row = 0; row <= 10; ++row)
        text(19 + row, right, row == 5 ? "  0 |----------------" : "    |                ");
    text(19, right, "+5");
    text(29, right, "-5");
    if(show_sun && std::abs(sun.altitude_deg) <= 5)
        mvaddch(24 - static_cast<int>(std::copysign(std::ceil(std::abs(sun.altitude_deg)), sun.altitude_deg)), right + 11, '*' | sun_style);
    if(show_moon && std::abs(moon.altitude_deg) <= 5)
        mvaddch(24 - static_cast<int>(std::copysign(std::ceil(std::abs(moon.altitude_deg)), moon.altitude_deg)), right + 17, 'o' | moon_style);
    text(30, right, "Outside +/-5: no zoom marker");
    int bottom = LINES - 5;
    if(show_sun){
        text(bottom, 2, "Solar noon: " + event_time(daily.solar_noon_utc_hours));
        if(daily.has_sunrise_sunset){
            text(bottom + 1, 2, "Rise: " + event_time(daily.sunrise_utc_hours)
                               + "  Set: " + event_time(daily.sunset_utc_hours));
        }else{
            text(bottom + 1, 2, "No complete sunrise/sunset pair for this solar day");
        }
    }
    text(bottom + 2, 2, "Geometric sky; events use standard -0.833 deg threshold. No terrain.");
    text(LINES - 1, 2, "Left/right: -/+1 min | Down/up: -/+1 hour | r: reset | q: quit");
    refresh();
} // ——— END OF draw() —————————————————————————————————————————————————————————
} // namespace
#endif



/// @brief Open the interactive sky chart until the user quits.
/// @param observer WGS-84 observer, including ellipsoidal height.
/// @param input Resolved starting UTC time, observer label and object selection.
/// @throws std::invalid_argument If the starting UTC calendar fields are invalid.
/// @throws std::runtime_error If TUI/terminal support or UTC time conversion fails.
void run_tui(const MGeoPoint& observer, const MResolvedCliInput& input)
{
#ifdef ASTRO_HAVE_CURSES
    // Validate before taking over the terminal.
    MSunPosition::calculate(observer, input.utc_time);
    Terminal terminal;
    MDateTimeUtc utc = input.utc_time;
    for(;;){
        draw(observer, input.observer_name, utc, input.show_sun, input.show_moon);
        int key = getch();
        if(key == 'q' || key == 'Q') break;
        if(key == 'r' || key == 'R') utc = input.utc_time;
        if(key == KEY_LEFT) utc = shift_time(utc, -1);
        if(key == KEY_RIGHT) utc = shift_time(utc, 1);
        if(key == KEY_DOWN) utc = shift_time(utc, -60);
        if(key == KEY_UP) utc = shift_time(utc, 60);
    }
#else
    (void)observer;
    (void)input;
    throw std::runtime_error("TUI support unavailable: rebuild with ncurses development files installed");
#endif
} // ——— END OF run_tui() ——————————————————————————————————————————————————————
