#include "status_effect.h"
#include "stringfunc.h" // For no_caps and trim
#include "window.h" // For debugmsg
#include "entity.h" // For Stats
#include <sstream>

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

Status_effect& Status_effect::operator=(const Status_effect& rhs)
{
  type = rhs.type;
  duration = rhs.duration;
  level = rhs.level;
  step_down.clear();
  for (int i = 0; i < rhs.step_down.size(); i++) {
    step_down.push_back(rhs.step_down[i]);
  }
  return *this;
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
      typestr = no_caps(typestr);
      typestr = trim(typestr);
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

std::string Status_effect::get_display_name()
{
  return status_effect_display_name(type);
}

std::string Status_effect::get_description()
{
  std::stringstream ret;
  switch (type) {

    case STATUS_NULL:
      ret << "Oops!  STATUS_NULL!  This is a bug.";
      break;

    case STATUS_BLIND:
      ret << "You are blinded and cannot see anything.";
      break;

    case STATUS_CAFFEINE:
      ret << "You're feeling peppy!" << std::endl <<
             "<c=ltgreen>Speed + 3<c=/>" << std::endl <<
             "<c=ltgreen>Intelligence + 1<c=/>" << std::endl <<
             "<c=ltgreen>Perception   + 1<c=/>";
      break;

    case STATUS_NICOTINE:
      ret << "Mmm, sweet nicotine." << std::endl;
      if (level <= 2) {
        ret << "<c=ltgreen>Dexterity    + 1<c=/>" << std::endl;
      } else {
        ret << "Too much nicotine made you jittery..." << std::endl <<
                 "<c=ltred>Dexterity    - 1<c=/>" << std::endl;
      }
      ret << "<c=ltgreen>Intelligence + 1<c=/>" << std::endl <<
             "<c=ltgreen>Perception   + 1<c=/>";
      break;

    case STATUS_STIMULANT:
      ret << "You're feeling speedy!" << std::endl;
      if (level >= 5) {
        ret << "<c=ltgreen>Speed + 15<c=/>";
      } else if (level >= 3) {
        ret << "<c=ltgreen>Speed + 10<c=/>";
      } else {
        ret << "<c=ltgreen>Speed + 5<c=/>";
      }
      ret << "<c=ltgreen>Dexterity    + 1<c=/>" << std::endl <<
             "<c=ltgreen>Intelligence + " << (level > 3 ? 3 : level) <<
             "<c=/>" << std::endl <<
             "<c=ltgreen>Perception   + " << (level > 2 ? 2 : level) <<
             "<c=/>" << std::endl;
      break;

    case STATUS_SLEEP_AID:
      ret << "You are gaining fatigue quickly.  This is useful if you want " <<
             "to fall asleep!";
      break;

    case STATUS_PAINKILL_MILD:
      ret << "Your pain is reduced slightly every " << 24 - level * 4 <<
             " minutes.";
      break;

    case STATUS_PAINKILL_MED:
      ret << "Your pain is reduced every " << 17 - level * 2 << " minutes." <<
             std::endl;
      if (level >= 5) {
        ret << "<c=ltred>Strength     - 2<c=/>" << std::endl;
        ret << "<c=ltred>Dexterity    - 2<c=/>" << std::endl;
      } else if (level >= 2) {
        ret << "<c=ltred>Strength     - 1<c=/>" << std::endl;
        ret << "<c=ltred>Dexterity    - 1<c=/>" << std::endl;
      }
      ret << "<c=ltred>Perception   - 1<c=/>";
      break;

    case STATUS_PAINKILL_LONG:
      ret << "Your pain is reduced every 2 minutes.";
      break;

    case STATUS_PAINKILL_HEAVY:
      ret << "Your pain is greated reduced every " << 11 - level <<
             " minutes." << std::endl;
      if (level >= 3) {
        ret << "<c=ltred>Strength     - " << (level - 1) / 2 << "<c=/>" <<
               std::endl;
      }
      if (level >= 2) {
        ret << "<c=ltred>Dexterity    - " << (level + 1) / 3 << "<c=/>" <<
               std::endl;
      }
      ret << "<c=ltred>Perception   - " << 1 + (level / 4) << "<c=/>";
      break;

    case STATUS_DRUNK:
      if (level >= 10) {
        ret << "The world is spinning and you want to puke." << std::endl;
      } else if (level >= 8) {
        ret << "You're stumbling around and having trouble focusing." <<
               std::endl;
      } else if (level >= 6) {
        ret << "You're slurring your speech and feeling wobbly." << std::endl;
      } else if (level >= 4) {
        ret << "You can't quite stand straight or touch your nose." <<
               std::endl;
      } else if (level >= 2) {
        ret << "You're feeling a bit light-headed." << std::endl;
      } else {
        ret << "You feel tough!" << std::endl;
      }
      if (level == 1) {
        ret << "<c=ltgreen>Strength     + 1<c=/>";
      } else {
        if (level >= 3) {
          ret << "<c=ltred>Strength     - " << level / 3 << "<c=/>" <<
                 std::endl;
        }
        ret << "<c=ltred>Dexterity    - " << 1 + level / 3 << "<c=/>" <<
               std::endl;
        ret << "<c=ltred>Perception   - " << 1 + level / 2 << "<c=/>" <<
               std::endl;
        ret << "<c=ltred>Intelligence - " << level << "<c=/>" << std::endl;
      }
      break;

    case STATUS_MAX:
      ret << "STATUS_MAX!  If you're seeing this, it's a bug!";
      break;

    default:
      ret << "Oh no!  Someone forgot to code a description for " <<
             get_name() << "!";
  } // switch (type)

  return ret.str();
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

int Status_effect::speed_mod()
{
  switch (type) {

    case STATUS_CAFFEINE:
      return 3;

    case STATUS_STIMULANT:
      if (level >= 5) {
        return 15;
      } else if (level >= 3) {
        return 10;
      } else {
        return 5;
      }
      break;
  }
  return 0;
}

Stats Status_effect::stats_mod()
{
  Stats ret(0, 0, 0, 0);

  switch (type) {

    case STATUS_CAFFEINE:
      ret.intelligence++;
      ret.perception++;
      break;

    case STATUS_NICOTINE:
      ret.intelligence++;
      ret.perception++;
      if (level <= 2) {
        ret.dexterity++;
      } else {
        ret.dexterity--;
      }
      break;

    case STATUS_STIMULANT:
      ret.dexterity++;
      ret.intelligence += (level > 3 ? 3 : level);
      ret.perception += (level > 2 ? 2 : level);
      break;

    case STATUS_PAINKILL_MED:
      ret.perception--;
      if (level >= 5) {
        ret.strength -= 2;
        ret.dexterity -= 2;
      } else if (level >= 2) {
        ret.strength--;
        ret.dexterity--;
      }
      break;

    case STATUS_PAINKILL_HEAVY:
      ret.strength   -=    (level - 1) / 2;
      ret.dexterity  -=    (level + 1) / 3;
      ret.perception -= 1 + level      / 4;
      break;

    case STATUS_DRUNK:
      if (level == 1) {
// Is this just silly?  I mean, in my experience it kinda makes sense...
        ret.strength++;
      } else {
        ret.strength   -= level / 3;  // 0, 0, 1, 1, 1, 2, ...
      }
      ret.dexterity    -= 1 + level / 3;  // 1, 1, 2, 2, 2, 3, ...
      ret.perception   -= 1 + level / 2;  // 1, 2, 2, 3, 3, 4, ...
      ret.intelligence -= level;          // 1, 2, 3, 4, 5, 6, ...
      break;
  }

  return ret;
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
    case STATUS_NICOTINE:       return "nicotine";
    case STATUS_STIMULANT:      return "stimulant";
    case STATUS_SLEEP_AID:      return "sleep_aid";
    case STATUS_PAINKILL_MILD:  return "painkill_mild";
    case STATUS_PAINKILL_MED:   return "painkill_med";
    case STATUS_PAINKILL_LONG:  return "painkill_long";
    case STATUS_PAINKILL_HEAVY: return "painkill_heavy";
    case STATUS_DRUNK:          return "drunk";
    case STATUS_MAX:            return "BUG - STATUS_MAX";
    default:                    return "BUG - Unnamed Status_effect_type";
  }
  return "BUG - Escaped status_effect_name() switch";
}

std::string status_effect_display_name(Status_effect_type type)
{
  switch (type) {
    case STATUS_NULL:           return "NULL";
    case STATUS_BLIND:          return "Blind";
    case STATUS_CAFFEINE:       return "Caffeine";
    case STATUS_NICOTINE:       return "Nicotine";
    case STATUS_STIMULANT:      return "Stimulant";
    case STATUS_SLEEP_AID:      return "Sleep Aid";
    case STATUS_PAINKILL_MILD:  return "Mild Painkiller";
    case STATUS_PAINKILL_MED:   return "Moderate Painkiller";
    case STATUS_PAINKILL_LONG:  return "Long-acting Painkiller";
    case STATUS_PAINKILL_HEAVY: return "Heavy Painkiller";
    case STATUS_DRUNK:          return "Drunk";
    case STATUS_MAX:            return "BUG - STATUS_MAX";
    default:                    return "BUG - Unnamed Status_effect_type";
  }
  return "BUG - Escaped status_effect_display_name() switch";
}
