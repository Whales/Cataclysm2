#include "status_effect.h"
#include "stringfunc.h" // For no_caps and trim
#include "window.h" // For debugmsg

Status_effect::Status_effect()
{
  id = STATUS_NULL;
  duration = 0;
  level = 0;
}

Status_effect::Status_effect(Status_effect_id _id, int _duration, int _level)
{
  id = _id;
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

    } else if (ident == "type:" || ident == "id:") {
      std::string type;
      std::getline(data, type);
      id = lookup_status_effect(type);
      if (id == STATUS_NULL) {
        debugmsg("Unknown Status_effect_id '%s' (%s)", type.c_str(),
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
  return status_effect_name(id);
}

Status_effect_id lookup_status_effect(std::string name)
{
  name = no_caps(name);
  name = trim(name);
  for (int i = 0; i < STATUS_MAX; i++) {
    Status_effect_id ret = Status_effect_id(i);
    if (name == no_caps( status_effect_name(ret) ) ) {
      return ret;
    }
  }
  return STATUS_NULL;
}

std::string status_effect_name(Status_effect_id id)
{
  switch (id) {
    case STATUS_NULL:           return "NULL";
    case STATUS_BLIND:          return "blind";
    case STATUS_CAFFEINE:       return "caffeine";
    case STATUS_PAINKILL_MILD:  return "painkill_mild";
    case STATUS_PAINKILL_MED:   return "painkill_med";
    case STATUS_PAINKILL_HEAVY: return "painkill_heavy";
    case STATUS_MAX:            return "BUG - STATUS_MAX";
    default:                    return "BUG - Unnamed Status_effect_id";
  }
  return "BUG - Escaped status_effect_name() switch";
}
