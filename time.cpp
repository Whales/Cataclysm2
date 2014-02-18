#include <sstream>
#include "time.h"
#include "stringfunc.h"

Time::Time()
{
  year = STARTING_YEAR;
  season = SEASON_SPRING;
  day = 0;
  hour = 0;
  minute = 0;
  second = 0;
  turn = 0;
}

Time::~Time()
{
}

Time::Time(const Time& copy)
{
  second = copy.second;
  minute = copy.minute;
  hour   = copy.hour;
  day    = copy.day;
  season = copy.season;
  year   = copy.year;
  turn   = copy.turn;
}

Time::Time(int _second, int _minute, int _hour, int _day, Season _season,
           int _year)
{
  second = _second;
  minute = _minute;
  hour   = _hour;
  day    = _day;
  season = _season;
  year   = _year;
  set_turn_from_date();
}

Time::Time(int _turn)
{
  turn = _turn;
  set_date_from_turn();
}

bool Time::operator==(const Time &other) const
{
  return turn == other.turn;
}

bool Time::operator!=(const Time &other) const
{
  return !(*this == other);
}

Time& Time::operator+=(const Time &rhs)
{
  second += rhs.second;
  minute += rhs.minute;
  hour   += rhs.hour;
  day    += rhs.day;
  year   += rhs.year;
  turn   += rhs.turn;

  int tmpseason = int(season) + int(rhs.season);
  while (tmpseason >= 4) {
    tmpseason -= 4;
    year++;
  }
  season = Season(tmpseason);

  standardize();
  return *this;
}

Time& Time::operator+=(const int &rhs)
{
  turn += rhs;
  set_date_from_turn();
  return *this;
}

Time& Time::operator-=(const Time &rhs)
{
  second -= rhs.second;
  minute -= rhs.minute;
  hour   -= rhs.hour;
  day    -= rhs.day;
  year   -= rhs.year;
  turn   -= rhs.turn;

  int tmpseason = int(season) - int(rhs.season);
  while (tmpseason < 0) {
    tmpseason += 4;
    year--;
  }
  season = Season(tmpseason);

  standardize();
  return *this;
}

Time& Time::operator-=(const int &rhs)
{
  turn -= rhs;
  set_date_from_turn();
  return *this;
}

Time::operator int() const
{
  return get_turn();
}

int Time::get_turn() const
{
  return turn;
}

int Time::get_second()
{
  if (second < 0 || second >= 60) {
    standardize();
  }
  return second;
}

int Time::get_minute()
{
  if (minute < 0 || minute >= 60) {
    standardize();
  }
  return minute;
}

int Time::get_hour()
{
  if (hour < 0 || hour >= 24) {
    standardize();
  }
  return hour;
}

int Time::get_day()
{
  if (day < 0 || day >= DAYS_IN_SEASON) {
    standardize();
  }
  return day;
}

Season Time::get_season() const
{
  return season;
}

int Time::get_year() const
{
  return year;
}

std::string Time::get_text(bool twentyfour)
{
  std::stringstream ret;
  int hour_output = hour;
  std::string period;
  if (!twentyfour) {
    period = " AM";
    if (hour == 0) {
      hour_output = 12;
    } else if (hour == 12) {
      period = " PM";
    } else if (hour > 12) {
      hour_output -= 12;
      period = " PM";
    }
  }
  ret << season_name(season) << " " << year << ", Day " << day + 1 <<
         hour << ":";
  if (minute < 10) {
    ret << "0";
  }
  ret << minute << period;

  return ret.str();
}

void Time::increment()
{
  turn++;
  second += SECONDS_IN_TURN;
  standardize();
}

void Time::standardize()
{
  int tmpseason = season;
  if (second >= 60) {
    minute += second / 60;
    second %= 60;
  } else if (second < 0) {
    minute += second / 60 - 1;
    second = 60 + (second % 60);
  }
  if (minute >= 60) {
    hour += minute / 60;
    minute %= 60;
  } else if (minute < 0) {
    hour += minute / 60 - 1;
    minute = 60 + (minute % 60);
  }
  if (hour >= 24) {
    day += hour / 24;
    hour %= 24;
  } else if (hour < 0) {
    day += hour / 24 - 1;
    hour = 24 + (hour % 24);
  }
  if (day >= DAYS_IN_SEASON) {
    tmpseason += day / DAYS_IN_SEASON;
    day %= DAYS_IN_SEASON;
  } else if (day < 0) {
    tmpseason += day / DAYS_IN_SEASON - 1;
    day = DAYS_IN_SEASON + (day % DAYS_IN_SEASON);
  }
  if (tmpseason >= 4) {
    year += tmpseason / 4;
    tmpseason %= 4;
  } else if (tmpseason < 0) {
    year += tmpseason / 4 - 1;
    tmpseason = 4 + (tmpseason % 4);
  }
  season = Season(tmpseason);
}

void Time::set_date_from_turn()
{
  second = turn * SECONDS_IN_TURN;
  minute = 0;
  hour   = 0;
  day    = 0;
  season = SEASON_SPRING;
  year   = STARTING_YEAR;

// Standardize turns lots of seconds to minutes, lots of minutes to hours, etc.
  standardize();
}

void Time::set_turn_from_date()
{
  turn = second / SECONDS_IN_TURN;
  turn += (60 * minute) / SECONDS_IN_TURN;
  turn += (3600 * hour) / SECONDS_IN_TURN;
  turn += (86400 * day) / SECONDS_IN_TURN;
  turn += (86400 * DAYS_IN_SEASON * season) / SECONDS_IN_TURN;
  turn += (345600 * DAYS_IN_SEASON * season) / SECONDS_IN_TURN;
}

Season lookup_season(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < 4; i++) {
    Season ret = Season(i);
    if ( no_caps( season_name(ret) ) == name ) {
      return ret;
    }
  }
  return SEASON_SPRING;
}

std::string season_name(Season season)
{
  switch (season) {
    case SEASON_SPRING: return "Spring";
    case SEASON_SUMMER: return "Summer";
    case SEASON_AUTUMN: return "Autumn";
    case SEASON_WINTER: return "Winter";
    default:            return "BUG - Mystery season";
  }
  return "BUG - Escaped season_name switch";
}
