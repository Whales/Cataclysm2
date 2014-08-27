#include "mission.h"
#include "rng.h"
#include "game.h"
#include <sstream>

void Mission_template::assign_uid(int ID)
{
  uid = ID;
}

std::string Mission_template::get_data_name()
{
  std::stringstream ret;
  ret << mission_type_name(type) << "_" << target_name;
  return ret.str();
}

std::string Mission_template::get_name()
{
  if (type == MISSION_NULL) {
    return "NULL";
  }

  std::stringstream ret;
  ret << mission_type_display_name(type) << " " << target_name;
  return ret.str();
}

bool Mission_template::load_data(std::istream& data)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident)) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "type:") {
      std::string type_name;
      std::getline(data, type_name);
      type = lookup_mission_type(type_name);
      if (type == MISSION_NULL) {
        debugmsg("Unknown Mission_type '%s'", type_name.c_str());
        return false;
      }

    } else if (ident == "target:") {
      std::getline(data, target_name);
      target_name = no_caps( trim(target_name) );
      if (target_name.empty()) {
        debugmsg("Empty target name (%s).", mission_type_name(type).c_str());
        return false;
      }

    } else if (ident == "count:") {
      data >> count_min >> count_max;
      if (count_min < 1 || count_max < count_min) {
        debugmsg("Invalid count (%d to %d) (%s).", count_min, count_max,
                 get_data_name().c_str());
        return false;
      }

    } else if (ident == "xp:") {
      data >> xp_min >> xp_max;
      if (xp_min < 0 || xp_max < xp_min) {
        debugmsg("Invalid XP (%d to %d) (%s).", xp_min, xp_max,
                 get_data_name().c_str());
        return false;
      }

    } else if (ident == "count_xp:") {
      data >> count_xp_bonus;
      if (count_xp_bonus < 0) {
        debugmsg("Invalid count_xp (%d) (%s).", count_xp_bonus,
                 get_data_name().c_str());
        return false;
      }

    } else if (ident == "time:") {
      data >> time_min >> time_max;
      if (time_min < 0 || time_max < time_min) {
        debugmsg("Invalid time (%d to %d) (%s).", time_min, time_max,
                 get_data_name().c_str());
        return false;
      }

    } else if (ident != "done") {
      debugmsg("Unknown Mission_template property '%s' (%s).", ident.c_str(),
               get_name().c_str());
    }
  }

  return true;
}

Mission::Mission(Mission_type T, std::string T_N, int T_C, int X, Time D,
                 bool P)
{
  type = T;
  target_name  = T_N;
  target_count = T_C;
  original_target_count = T_C;
  xp = X;
  deadline = D;
  personal = P;
  if (deadline.get_turn() == -1) {
    deadline = GAME.time + HOURS(1);
  }
}

bool Mission::set_from_template(Mission_template* temp)
{
  type = temp->type;
  if (type == MISSION_NULL) {
    debugmsg("Copied MISSION_NULL from template!");
    return false;
  }

  target_name = temp->target_name;
  target_count = rng(temp->count_min, temp->count_max);
  original_target_count = target_count;
  xp = rng(temp->xp_min, temp->xp_max);
  if (target_count > 1) {
    xp += temp->count_xp_bonus * (target_count - 1);
  }
  int time_to_finish = rng(temp->time_min, temp->time_max);
  deadline = GAME.time + HOURS(time_to_finish);
  deadline.standardize();

  return true;
}

Time Mission::get_time_left()
{
  Time ret = deadline - GAME.time;
  ret.standardize();
  return ret;
}

std::string Mission::get_description()
{
  std::stringstream ret;
  ret << mission_type_display_name(type) << "\"" << target_name << "\"";
  if (original_target_count > 1) {
    ret << " x " << target_count;
  }
  return ret.str();
}

std::string Mission::get_deadline_text()
{
  std::stringstream ret;
  Time time_left = get_time_left();
  if (time_left <= HOURS(6)) { // Six hours or less!
    ret << "<c=ltred>";
  }
  ret << deadline.get_text() << "<c=/>";
  return ret.str();
}

std::string Mission::get_time_left_text()
{
  std::stringstream ret;
  Time time_left = get_time_left();
  if (time_left <= HOURS(6)) {
    ret << "<c=ltred>";
  }
  ret << time_left.get_text() << "<c=/>";
  return ret.str();
}

std::string Mission::get_experience_text()
{
  std::stringstream ret;
  if (xp >= 100) {
    ret << "<c=yellow>";
  }
  ret << xp << "<c=/>";
  return ret.str();
}

Mission_type lookup_mission_type(std::string name)
{
  name = no_caps( trim( name ) );
  for (int i = 0; i < MISSION_MAX; i++) {
    Mission_type ret = Mission_type(i);
    if (name == no_caps( mission_type_name(ret) )) {
      return ret;
    }
  }
  return MISSION_NULL;
}

std::string mission_type_name(Mission_type type)
{
  switch (type) {
    case MISSION_NULL:        return "NULL";
    case MISSION_EAT:         return "eat";
    case MISSION_READ_GENRE:  return "read_genre";
    case MISSION_MAX:         return "BUG - MISSION_MAX";
    default:                  return "BUG - Unnamed Mission_type";
  }
  return "BUG - Escaped mission_type_name() switch";
}

std::string mission_type_display_name(Mission_type type)
{
  switch (type) {
    case MISSION_NULL:        return "NULL";
    case MISSION_EAT:         return "Eat";
    case MISSION_READ_GENRE:  return "Read genre";
    case MISSION_MAX:         return "BUG - MISSION_MAX";
    default:                  return "BUG - Unnamed Mission_type";
  }
  return "BUG - Escaped mission_type_display_name() switch";
}
