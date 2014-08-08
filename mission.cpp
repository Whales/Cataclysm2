#include "mission.h"
#include "rng.h"
#include "game.h"

Mission::Mission(Mission_type T, std::string T_N, int X, Time D)
{
  type = T;
  target_name = T_N;
  xp = X;
  deadline = D;
  if (deadline.get_turn() == -1) {
    deadline = GAME.time + HOURS(1);
  }
}

bool Mission::set_from_template(const Mission_template& temp)
{
  type = temp.type;
  if (type == MISSION_NULL) {
    debugmsg("Copied MISSION_NULL from template!");
    return false;
  }

  target_name = temp.target_name;
  xp = rng(temp.xp_min, temp.xp_max);
  int time_to_finish = rng(temp.time_min, temp.time_max);
  deadline = GAME.time + HOURS(time_to_finish);

  return true;
}
