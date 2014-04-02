#include "entity.h"
#include "player.h"
#include "cuss.h"
#include "trait.h"
#include "files.h"  // For CUSS_DIR
#include "window.h"
#include <string>
#include <sstream>

enum New_char_screen
{
  NCS_CANCEL,
  NCS_STATS,
  NCS_TRAITS,
  NCS_PROFESSION,
  NCS_DESCRIPTION,
  NCS_DONE
};

enum Stat_selected
{
  STATSEL_STR,
  STATSEL_DEX,
  STATSEL_PER,
  STATSEL_INT
};

std::string                 get_stat_description(Stat_selected stat);

std::vector< std::string >  get_trait_list(Player* pl);

std::vector< std::string >  get_profession_list(Player* pl);

bool Player::create_new_character()
{
  Window w_newch(0, 0, 80, 24);
  cuss::interface i_newch;
  if (!i_newch.load_from_file(CUSS_DIR + "/i_newchar_stats.cuss")) {
    return false;
  }

  New_char_screen cur_screen = NCS_STATS;

  Stat_selected cur_stat = STATSEL_STR;
  int* stat_value = &(stats.strength);

  std::vector<std::string> traits_list = get_trait_list(this);
  std::vector<std::string> profession_list = get_profession_list(this);

  name = "";

  int points = 4;
  int num_traits = 0;

  while (true) {  // We'll exit this function via keypresses, always
// Always set num_points!
    i_newch.set_data("num_points", points);
    switch (cur_screen) {

      case NCS_STATS:
        i_newch.set_data("num_strength",     stats.strength);
        i_newch.set_data("num_dexterity",    stats.dexterity);
        i_newch.set_data("num_perception",   stats.perception);
        i_newch.set_data("num_intelligence", stats.intelligence);
        i_newch.set_data("text_description", get_stat_description(cur_stat));
        break;

      case NCS_TRAITS: {
        i_newch.set_data("list_traits", traits_list);
        Trait_id cur_trait = Trait_id( i_newch.get_int("list_traits") );
        i_newch.set_data("text_description", trait_description(cur_trait));
        i_newch.set_data("num_cost", trait_cost(cur_trait));
      } break;

      case NCS_PROFESSION: {
        i_newch.set_data("list_professions", profession_list);
        std::string prof_name = i_newch.get_str("list_professions");
        Profession* cur_prof = PROFESSIONS.lookup_name(prof_name);
        if (!cur_prof) {
          debugmsg("No such profession as '%s'!", prof_name.c_str());
          return false;
        }
        i_newch.set_data("text_description", cur_prof->description);
      } break;

      case NCS_DESCRIPTION:
        i_newch.ref_data("entry_name", &name);
        if (male) {
          i_newch.set_data("text_male",   "<c=yellow>Male<c=/>");
          i_newch.set_data("text_female", "<c=dkgray>Female<c=/>");
        } else {
          i_newch.set_data("text_male",   "<c=dkgray>Male<c=/>");
          i_newch.set_data("text_female", "<c=yellow>Female<c=/>");
        }
        break;
    } // switch (cur_screen)

    i_newch.draw(&w_newch);
    w_newch.refresh();

    long ch = getch();
    bool changed_screen = false;

    if (ch == '<') {
      cur_screen = New_char_screen( cur_screen - 1 );
      if (cur_screen == NCS_CANCEL) {
        if (query_yn("Cancel character creation?")) {
          return false;
        }
        cur_screen = NCS_STATS;
      } else {
        changed_screen = true;
      }
    } else if (ch == '>') {
      cur_screen = New_char_screen( cur_screen + 1 );
      if (cur_screen == NCS_DONE) {
// TODO: Insert tests for completed reqs (all points spent, name entered, etc)
        if (query_yn("Complete character and start the game?")) {
          return true;
        }
        cur_screen = NCS_DESCRIPTION;
      } else {
        changed_screen = true;
      }
    } else {
// We should be doing this with cuss keybindings, but... that gets complex.
// Maybe one day I'll update cuss to be more friendly.
      switch (cur_screen) {

        case NCS_STATS: {
          bool changed_stat = false;
          switch (ch) {
            case '2':
            case 'j':
            case KEY_DOWN:
              if (cur_stat == STATSEL_INT) {
                cur_stat = STATSEL_STR;
              } else {
                cur_stat = Stat_selected( cur_stat + 1 );
              }
              changed_stat = true;
              break;

            case '8':
            case 'k':
            case KEY_UP:
              if (cur_stat == STATSEL_STR) {
                cur_stat = STATSEL_INT;
              } else {
                cur_stat = Stat_selected( cur_stat - 1 );
              }
              changed_stat = true;
              break;

            case '4':
            case 'h':
            case KEY_LEFT:
              if (*stat_value > 4) {
                if (*stat_value > 16) {
                  points++; // Stats above 16 cost 2 points, so get extra back
                }
                points++;
                stat_value--;
              }
              break;

            case '6':
            case 'l':
            case KEY_RIGHT: {
              int point_req = (*stat_value >= 16 ? 2 : 1);
              if (points >= point_req) {
                points -= point_req;
                stat_value++;
              }
            } break;
          } // switch (ch)

          if (changed_stat) { // Update stat_value
            switch (cur_stat) {
              case STATSEL_STR: stat_value = &(stats.strength);     break;
              case STATSEL_DEX: stat_value = &(stats.dexterity);    break;
              case STATSEL_PER: stat_value = &(stats.perception);   break;
              case STATSEL_INT: stat_value = &(stats.intelligence); break;
            }
          }
        } break;

        case NCS_TRAITS: {
          switch (ch) {
            case '2':
            case 'j':
            case KEY_DOWN:
              i_newch.add_data("list_traits", 1);
              break;

            case '8':
            case 'k':
            case KEY_UP:
              i_newch.add_data("list_traits", -1);
              break;

            case '\n':
            case ' ':
            {
              Trait_id cur_trait = Trait_id( i_newch.get_int("list_traits") );
              if (has_trait(cur_trait)) {
                traits[cur_trait] = false;
                points += trait_cost(cur_trait);
                num_traits--;
              } else if (points >= trait_cost(cur_trait) && num_traits < 5){
                traits[cur_trait] = true;
                points -= trait_cost(cur_trait);
                num_traits++;
              }
            } break;

          } // switch (ch)
        } break;

        case NCS_PROFESSION: {
          switch (ch) {
            case '2':
            case 'j':
            case KEY_DOWN:
              i_newch.add_data("list_professions", 1);
              break;

            case '8':
            case 'k':
            case KEY_UP:
              i_newch.add_data("list_professions", -1);
              break;

            case '\n':
            case ' ':
            {
              std::string prof_name = i_newch.get_str("list_professions");
              Profession* cur_prof = PROFESSIONS.lookup_name(prof_name);
              if (!cur_prof) {
                debugmsg("No such profession as '%s'!", prof_name.c_str());
                return false;
              }
              set_profession(cur_prof);
            } break;

          } // switch (ch)
        } break;

        case NCS_DESCRIPTION: {
          i_newch.select("entry_name"); // Always make sure this is selected
          if (ch == '/') {
            male = !male;
          } else {
/* Let the interface handle name entry; this includes cursor movement,
 * backspace, etc.  The only downside is that this allows entry of "invalid"
 * name characters like "'&^%$#@ etc.  Bad?
 */
            i_newch.handle_keypress(ch);
          }
            
        } break;

      } // switch (cur_screen)
    } // key pressed isn't '<' or '>'

    if (changed_screen) {
      std::string filename = CUSS_DIR + "/i_newchar_";
      switch (cur_screen) {
        case NCS_STATS:       filename += "stats.cuss";       break;
        case NCS_TRAITS:      filename += "traits.cuss";      break;
        case NCS_PROFESSION:  filename += "profession.cuss";  break;
        case NCS_DESCRIPTION: filename += "description.cuss"; break;
      }
      if (!i_newch.load_from_file(filename)) {
        return false;
      }
    }
  }

  debugmsg("Player::create_new_character() escaped loop somehow!");
  return false;
}

std::string get_stat_description(Stat_selected stat)
{
  switch (stat) {

    case STATSEL_STR: return "\
Strength affects the amount of weight you can carry or drag, and your base \
melee damage.  It's especially important for increasing the damage when using \
a blunt weapon.  It also helps you hit faster when using a heavy weapon. Many \
actions which require brute force, like smashing through doors, rely on \
strength.";

    case STATSEL_DEX: return "\
Dexterity improves your chance of hitting a target in melee combat, and to a \
lesser degree, ranged combat.  It improves damage when using a piercing \
weapon, and improves speed when attacking with a large object.  It's also \
useful for a variety of smaller purposes, like falling gracefully or avoiding \
traps.";

    case STATSEL_PER: return "\
Perception is vital for accurate ranged attacks.  It slightly improves your \
damage with cutting or piercing melee weapons.  Finally, it's useful for a \
variety of minor uses, like detecting traps or stealthy monsters, noticing \
when an NPC is lying, or smuggling items.";

    case STATSEL_INT: return "\
Intelligence is the most subtle stat, but often the most important.  It \
affects how quickly you can read books, and how well you'll absorb their \
contents.  It's also important for NPC interaction and the use of bionics.";
  }

  return "Unknown stat???";
}


std::vector<std::string> get_trait_list(Player* pl)
{
  std::vector<std::string> ret;
  for (int i = 1; i < TRAIT_MAX_BAD; i++) {
// Skip over "marker" traits
    if (i == TRAIT_MAX_GOOD || i == TRAIT_MAX_NEUTRAL) {
      i++;
    }
    std::stringstream name;
    if (pl->has_trait( Trait_id(i) )) {
      name << "<c=white>";
    } else if (i < TRAIT_MAX_GOOD) {
      name << "<c=ltgreen>";
    } else if (i < TRAIT_MAX_NEUTRAL) {
      name << "<c=yellow>";
    } else {
      name << "<c=red>";
    }
    name << trait_id_name( Trait_id(i) ) << "<c=/>";
    ret.push_back(name.str());
  }
  return ret;
}

std::vector< std::string >  get_profession_list(Player* pl)
{
  std::vector< std::string > ret;
  for (std::list<Profession*>::iterator it = PROFESSIONS.instances.begin();
       it != PROFESSIONS.instances.end();
       it++) {
    std::stringstream text;
    if ( (*it) == pl->get_profession()) {
      text << "<c=white>";
    } else {
      text << "<c=ltblue>";
    }
    text << (*it)->name << "<c=/>";
    ret.push_back( text.str() );
  }
  return ret;
}
