#ifndef _STATUS_EFFECT_H_
#define _STATUS_EFFECT_H_

#include <string>
#include <istream>
#include <vector>

struct Stats;

enum Status_effect_type
{
  STATUS_NULL = 0,
  STATUS_BLIND,     // "blind" - lose sense of sight
  STATUS_CAFFEINE,  // "caffeine" - minor speed & stat boost
  STATUS_NICOTINE,  // "nicotine" - minor stat boost
  STATUS_STIMULANT, // "stimulant" - larger speed & stat boost
  STATUS_SLEEP_AID, // "sleep_aid" - gain fatigue faster
  STATUS_PAINKILL_MILD, // "painkill_mild"  - lift painkill to 10
  STATUS_PAINKILL_MED,  // "painkill_med"   - lift painkill to 50
  STATUS_PAINKILL_LONG, // "painkill_long"  - lift painkill to 40 & stay there
  STATUS_PAINKILL_HEAVY,// "painkill_heavy" - lift painkill to 100
  STATUS_DRUNK,     // "drunk" - stat loss, mild painkiller
  STATUS_MAX
};

Status_effect_type lookup_status_effect(std::string name);
std::string status_effect_name(Status_effect_type type);

struct Status_effect
{
  Status_effect();
  Status_effect(Status_effect_type _type, int _duration, int _level = 1);
  ~Status_effect();

  bool load_data(std::istream& data, std::string owner_name);
  std::string get_name();

  void boost(int dur, int lev = 1);
  void boost(const Status_effect& rhs);
// Returns true if timed out
  bool decrement();

// Simple effects - for active use and info screens
  int speed_mod();
  Stats stats_mod();

  Status_effect_type type;
  int duration;
  std::vector<int> step_down;  // The duration(s) at which we lose a level.
  int level;
};

#endif
