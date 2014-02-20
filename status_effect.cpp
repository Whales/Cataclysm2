#include "status_effect.h"
#include "stringfunc.h" // For no_caps and trim
#include "window.h" // For debugmsg

Status_effect::Status_effect()
{
  type = STATUS_NULL;
  duration = 0;
  level = 1;
}

Status_effect::Status_effect(Status_effect_type _type, int _duration, int _level)
{
  type = _type;
  duration = _duration;
  level = _level;
}

Status_effect::~Status_effect()
{
}

bool Status_effect::load_data(std::istream& data, std::string owner_name)
{
  std::string ident, junk;
  while (ident != "done" && !data.eof()) {
    if ( ! (data >> ident) ) {
      return false;
    }
    ident = no_caps(ident);

    if (!ident.empty() && ident[0] == '#') {
// It's a comment
      std::getline(data, junk);

    } else if (ident == "type:" || ident == "type:") {
      std::string typestr;
      std::getline(data, typestr);
      type = lookup_status_effect(typestr);
      if (type == STATUS_NULL) {
        debugmsg("Unknown Status_effect_type '%s' (%s)", typestr.c_str(),
                 owner_name.c_str());
        return false;
      }

    } else if (ident == "duration:") {
      data >> duration;
      std::getline(data, junk);

    } else if (ident == "level:") {
      data >> level;
      std::getline(data, junk);

    } else if (ident != "done") {
      debugmsg("Unknown Status_effect property '%s' (%s)", ident.c_str(),
               owner_name.c_str());
      return false;
    }
  }
  return true;
}
 

std::string Status_effect::get_name()
{
  return status_effect_name(type);
}

void Status_effect::boost(int dur, int lev)
{
  int old_dur = duration;
  duration += dur;
  level += lev;
/* Set many step_down points; first when we hit our current duration, more
 * evenly spaced between the current duration and the new duration.
 */
  if (lev > 0) {
    int gap = dur / lev;
    for (int i = 0; i < lev; i++) {
      step_down.push_back(old_dur);
      old_dur += gap;
    }
  }
}

void Status_effect::boost(const Status_effect& rhs)
{
  boost(rhs.duration, rhs.level);
}

bool Status_effect::decrement()
{
  duration--;
  if (!step_down.empty() && duration <= step_down.back()) {
    step_down.pop_back();
    if (level > 1) {
      level--;
    }
  }
  return (duration <= 0);
}

Status_effect_type lookup_status_effect(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < STATUS_MAX; i++) {
    Status_effect_type ret = Status_effect_type(i);
    if (name == no_caps( status_effect_name(ret) ) ) {
      return ret;
    }
  }
  return STATUS_NULL;
}

std::string status_effect_name(Status_effect_type type)
{
  switch (type) {
    case STATUS_NULL:           return "NULL";
    case STATUS_BLIND:          return "blind";
    case STATUS_CAFFEINE:       return "caffeine";
    case STATUS_PAINKILL_MILD:  return "painkill_mild";
    case STATUS_PAINKILL_MED:   return "painkill_med";
    case STATUS_PAINKILL_HEAVY: return "painkill_heavy";
    case STATUS_MAX:            return "BUG - STATUS_MAX";
    default:                    return "BUG - Unnamed Status_effect_type";
  }
  return "BUG - Escaped status_effect_name() switch";
}
