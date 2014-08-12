#ifndef _MISSION_H_
#define _MISSION_H_

#include "time.h"
#include <string>
#include <istream>

enum Mission_type
{
  MISSION_NULL = 0,
  MISSION_EAT,        // "eat" - eat a specific food item.
  MISSION_READ_GENRE, // "read" - read a particular genre of book
  MISSION_MAX
};

Mission_type lookup_mission_type(std::string name);
std::string mission_type_name(Mission_type type);
std::string mission_type_display_name(Mission_type type);

struct Mission_template
{
  Mission_template(Mission_type T = MISSION_NULL, std::string T_N = "",
                   int X_min = 0, int X_max = 0, int T_min = 1, int T_max = 1) :
    type (T), target_name (T_N), xp_min (X_min), xp_max (X_max),
    time_min (T_min), time_max (T_max)
    { uid = -1; }

  void assign_uid(int ID);

  std::string get_data_name();
  std::string get_name(); // type + target_name
  bool load_data(std::istream& data);

  int uid;
  Mission_type type;
  std::string target_name;  // Can refer to a item, monster, genre, anything...
  int xp_min, xp_max; // Range of XP we get
  int time_min, time_max; // Amount of time to complete, in hours.
};

// Actual, active mission.
struct Mission
{
  Mission(Mission_type T = MISSION_NULL, std::string T_N = "", int X = 0,
          Time D = Time(-1));

  Mission_type type;
  std::string target_name;
  int xp;
  Time deadline;

  bool set_from_template(const Mission_template& temp);
};

#endif
