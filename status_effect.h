#ifndef _STATUS_EFFECT_H_
#define _STATUS_EFFECT_H_

#include <string>
#include <istream>

enum Status_effect_id
{
  STATUS_NULL = 0,
  STATUS_BLIND,     // "blind" - lose sense of sight
  STATUS_CAFFEINE,  // "caffeine" - minor speed & stat boost
  STATUS_PAINKILL_MILD, // "painkill_mild" - lift painkill to 10
  STATUS_PAINKILL_MED,  // "painkill_med" - lift painkill to 50
  STATUS_PAINKILL_HEAVY,// "painkill_heavy" - lift painkill to 100
  STATUS_MAX
};

Status_effect_id lookup_status_effect(std::string name);
std::string status_effect_name(Status_effect_id id);

struct Status_effect
{
  Status_effect();
  Status_effect(Status_effect_id _id, int _duration, int _level = 1);
  ~Status_effect();

  bool load_data(std::istream& data, std::string owner_name);
  std::string get_name();

  Status_effect_id id;
  int duration;
  int level;
};

#endif
