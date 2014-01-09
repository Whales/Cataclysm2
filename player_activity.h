#ifndef _PLAYER_activity_H_
#define _PLAYER_activity_H_

#include <string>
#include <vector>

enum Player_activity_type
{
  PLAYER_ACTIVITY_NULL,
  PLAYER_ACTIVITY_WAIT,
  PLAYER_ACTIVITY_RELOAD,
  PLAYER_ACTIVITY_MAX
};

std::string get_activity_name      (Player_activity_type type);
std::string get_activity_participle(Player_activity_type type);

struct Player_activity
{
  Player_activity()
  {
    type = PLAYER_ACTIVITY_NULL;
    duration = 0;
    primary_item_uid = -1;
    secondary_item_uid = -1;
    active = false;
  }

  Player_activity(Player_activity_type _type, int _duration,
                  int _primary_uid = -1, int _secondary_uid = -1) :
    type (_type),
    duration (_duration),
    primary_item_uid (_primary_uid),
    secondary_item_uid (_secondary_uid)
  { active = true; };

  Player_activity_type type;
  int duration;
  int primary_item_uid;
  int secondary_item_uid;
  bool active;

  std::string get_name();       // Proper name e.g. "wait"
  std::string get_participle(); // "You stop ______" e.g. waiting
  bool is_active() { return active && (type != PLAYER_ACTIVITY_NULL); }
};

#endif
