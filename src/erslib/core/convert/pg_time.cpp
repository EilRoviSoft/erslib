#include "erslib/core/convert/pg_time.hpp"

// std
#include <chrono>
#include <cstddef>
#include <format>
#include <ratio>

// ers
#include <erslib/core/stdlib/ctype.hpp>


// Details

namespace {
    using pg_precision_t = std::chrono::microseconds;
    using pg_time_t = std::chrono::sys_time<pg_precision_t>;

    inline constexpr size_t pg_max_offset_hours = 15;
    inline constexpr size_t pg_timestamp_max_size = 34;
}

namespace {
    constexpr std::size_t count_digits(std::string_view s) noexcept {
        std::size_t n = 0;
        while (n < s.size() && ers::is_digit(s[n]))
            ++n;
        return n;
    }

    constexpr bool read_digits(std::string_view& s, std::size_t n, int& out) noexcept {
        if (n == 0 || n > 9 || s.size() < n)
            return false;

        int value = 0;
        for (std::size_t i = 0; i < n; ++i) {
            if (!ers::is_digit(s[i]))
                return false;
            value = value * 10 + (s[i] - '0');
        }

        out = value;
        s.remove_prefix(n);
        return true;
    }

    constexpr bool read_char(std::string_view& s, char c) noexcept {
        if (s.empty() || s.front() != c)
            return false;
        s.remove_prefix(1);
        return true;
    }

    bool fits_timestamp(pg_time_t tp) noexcept {
        if constexpr (!std::ratio_less_equal_v<typename ers::duration_t::period, std::micro>)
            return true;

        auto lo = std::chrono::ceil<pg_precision_t>(ers::timestamp_t::min());
        auto hi = std::chrono::floor<pg_precision_t>(ers::timestamp_t::max());

        return tp > lo && tp < hi;
    }

    char* write_timestamp(char* begin, char* end, ers::timestamp_t what) {
        auto put = [&](std::string_view sv) {
            return std::format_to_n(begin, end - begin, "{}", sv).out;
        };


        if (what == ers::timestamp_t::max())
            return put("infinity");

        if (what == ers::timestamp_t::min())
            return put("-infinity");


        auto day = floor<std::chrono::days>(what);
        std::chrono::year_month_day ymd { day };
        std::chrono::hh_mm_ss hms { std::chrono::floor<pg_precision_t>(what - day) };

        int raw_year = static_cast<int>(ymd.year());
        bool is_bc = raw_year <= 0;
        int year = is_bc ? 1 - raw_year : raw_year;


        return std::format_to_n(
            begin, end - begin,
            "{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:06}+00{}",
            year,
            static_cast<unsigned>(ymd.month()),
            static_cast<unsigned>(ymd.day()),
            hms.hours().count(),
            hms.minutes().count(),
            hms.seconds().count(),
            hms.subseconds().count(),
            is_bc ? "BC" : ""
        ).out;
    }
}


// Implementation

ers::timestamp_t ers::impl::floor_to_pg_precision(timestamp_t what) {
    if constexpr (!std::ratio_less_v<typename duration_t::period, std::micro>)
        return what;
    return std::chrono::floor<duration_t>(std::chrono::floor<pg_precision_t>(what));
}

std::string ers::impl::timestamp_to_iso8601(timestamp_t what) {
    char buffer[pg_timestamp_max_size];
    char* last = write_timestamp(buffer, buffer + sizeof buffer, what);
    return std::string(buffer, last);
}

ers::Result<std::chrono::time_point<std::chrono::system_clock>> ers::impl::timestamp_from_iso8601(std::string_view text) {
    if (text == "infinity")
        return timestamp_t::max();

    if (text == "-infinity")
        return timestamp_t::min();


    std::string_view s = text;
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;


    size_t year_digits = count_digits(s);
    if (year_digits < 4 || year_digits > 5)
        return make_error("Malformed timestamp '{}': bad year.", text);


    read_digits(s, year_digits, year);

    bool date_time_ok = read_char(s, '-')
        && read_digits(s, 2, month)
        && read_char(s, '-')
        && read_digits(s, 2, day)
        && (read_char(s, ' ') || read_char(s, 'T'))
        && read_digits(s, 2, hour)
        && read_char(s, ':')
        && read_digits(s, 2, minute)
        && read_char(s, ':')
        && read_digits(s, 2, second);

    if (!date_time_ok)
        return make_error("Malformed timestamp '{}'.", text);


    int microsecond = 0;
    if (read_char(s, '.')) {
        const size_t n = count_digits(s);
        if (n == 0 || n > 6)
            return make_error("Malformed timestamp '{}': bad fractional seconds.", text);

        read_digits(s, n, microsecond);
        for (size_t i = n; i < 6; ++i)
            microsecond *= 10;
    }


    int sign = 0;
    if (read_char(s, '+'))
        sign = +1;
    else if (read_char(s, '-'))
        sign = -1;
    else if (!read_char(s, 'Z'))
        return make_error("Malformed timestamp '{}': missing UTC offset.", text);


    int out_hour = 0, out_minute = 0, out_second = 0;
    if (sign != 0) {
        bool offset_ok = read_digits(s, 2, out_hour);
        if (offset_ok && read_char(s, ':')) {
            offset_ok = read_digits(s, 2, out_minute);
            if (offset_ok && read_char(s, ':'))
                offset_ok = read_digits(s, 2, out_second);
        }

        if (!offset_ok || out_hour > pg_max_offset_hours || out_minute > 59 || out_second > 59)
            return make_error("Malformed timestamp '{}': bad UTC offset.", text);
    }


    if (read_char(s, ' ')) {
        if (s != "BC" || year == 0)
            return make_error("Malformed timestamp '{}'.", text);
        s = {};
        year = 1 - year;
    }

    if (!s.empty())
        return make_error("Malformed timestamp '{}': trailing characters.", text);


    std::chrono::year_month_day ymd(
        std::chrono::year(year),
        std::chrono::month(static_cast<unsigned>(month)),
        std::chrono::day(static_cast<unsigned>(day))
    );

    if (!ymd.ok() || hour > 23 || minute > 59 || second > 59)
        return make_error("Malformed timestamp '{}': field out of range.", text);


    pg_time_t local = std::chrono::sys_days(ymd)
        + std::chrono::hours(hour)
        + std::chrono::minutes(minute)
        + std::chrono::seconds(second)
        + std::chrono::microseconds(microsecond);

    std::chrono::seconds offset(sign * (out_hour * 3600 + out_minute * 60 + out_second));

    pg_time_t utc = local - offset;

    if (!fits_timestamp(utc))
        return make_error("Timestamp '{}' is out of range.", text);


    return timestamp_t(std::chrono::floor<duration_t>(utc.time_since_epoch()));
}

ers::timestamp_t ers::impl::pg_now() {
    return floor_to_pg_precision(clock_t::now());
}
