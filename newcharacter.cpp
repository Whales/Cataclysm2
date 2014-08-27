#include "entity.h"
#include "player.h"
#include "cuss.h"
#include "trait.h"
#include "files.h"  // For CUSS_DIR
#include "window.h"
#include <string>
#include <sstream>
#include <stdlib.h> // For abs()

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
  stats = Stats(10, 10, 10, 10);
  Window w_newch(0, 0, 80, 24);
  cuss::interface i_newch;
  if (!i_newch.load_from_file(CUSS_DIR + "/i_newchar_stats.cuss")) {
    return false;
  }

  New_char_screen cur_screen = NCS_STATS;

  Stat_selected cur_stat = STATSEL_STR;
  int* stat_value = &(stats.strength);

/* We need to set up a list of traits which does NOT include the placeholder / 
 * marker "traits" like TRAIT_MAX_GOOD and TRAIT_MAX_NEUTRAL etc.
 */
  std::vector<Trait_id> selectable_traits;
  for (int i = 1; i < TRAIT_MAX_BAD; i++) {
    if (i != TRAIT_MAX_GOOD && i != TRAIT_MAX_NEUTRAL) {
      selectable_traits.push_back( Trait_id(i) );
    }
  }

  std::vector<std::string> traits_list     = get_trait_list     (this);
  std::vector<std::string> profession_list = get_profession_list(this);

  name = "";

  int points = 4;
  int num_traits = 0;

  i_newch.ref_data("num_points", &points);

  i_newch.ref_data("num_strength",     &stats.strength);
  i_newch.ref_data("num_dexterity",    &stats.dexterity);
  i_newch.ref_data("num_perception",   &stats.perception);
  i_newch.ref_data("num_intelligence", &stats.intelligence);
  i_newch.set_data("text_description", get_stat_description(cur_stat));

  i_newch.set_data("text_strength",     "<c=ltblue>Strength<c=/>");
  i_newch.set_data("text_dexterity",    "<c=ltgray>Dexterity<c=/>");
  i_newch.set_data("text_perception",   "<c=ltgray>Perception<c=/>");
  i_newch.set_data("text_intelligence", "<c=ltgray>Intelligence<c=/>");

  bool done = false;

  while (!done) {  // We'll exit this function via keypresses, always
// Always set num_points!

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
        std::string reason_for_fail;
        if (points > 0) {
          reason_for_fail += "\nYou have unspent points!";
        }
        if (profession == NULL) {
          reason_for_fail += "\nYou didn't choose a profession!";
        }
        if (name.empty()) {
          reason_for_fail += "\nYour name is blank!";
        }
        if (!reason_for_fail.empty()) {
          popup("Wait, you can't start the game yet!%s",
                reason_for_fail.c_str());
        } else if (query_yn("Complete character and start the game?")) {
          done = true;
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
                (*stat_value)--;
              }
              break;

            case '6':
            case 'l':
            case KEY_RIGHT: {
              int point_req = (*stat_value >= 16 ? 2 : 1);
              if (*stat_value < 20 && points >= point_req) {
                points -= point_req;
                (*stat_value)++;
              }
            } break;
          } // switch (ch)

          if (changed_stat) { // Update stat_value
            i_newch.set_data("text_strength",    "<c=ltgray>Strength<c=/>");
            i_newch.set_data("text_dexterity",   "<c=ltgray>Dexterity<c=/>");
            i_newch.set_data("text_perception",  "<c=ltgray>Perception<c=/>");
            i_newch.set_data("text_intelligence","<c=ltgray>Intelligence<c=/>");

            i_newch.set_data("text_description",
                             get_stat_description(cur_stat));
            switch (cur_stat) {
              case STATSEL_STR:
                stat_value = &(stats.strength);
                i_newch.set_data("text_strength",
                                 "<c=ltblue>Strength<c=/>");
                break;
              case STATSEL_DEX:
                stat_value = &(stats.dexterity);
                i_newch.set_data("text_dexterity",
                                 "<c=ltblue>Dexterity<c=/>");
                break;
              case STATSEL_PER:
                stat_value = &(stats.perception);
                i_newch.set_data("text_perception",
                                 "<c=ltblue>Perception<c=/>");
                break;
              case STATSEL_INT:
                stat_value = &(stats.intelligence);
                i_newch.set_data("text_intelligence",
                                 "<c=ltblue>Intelligence<c=/>");
                break;
            }
          }
        } break;

        case NCS_TRAITS: {
          switch (ch) {
            case '2':
            case 'j':
            case KEY_DOWN:
            {
              i_newch.add_data("list_traits", 1);
              int sel = i_newch.get_int("list_traits");
              Trait_id cur_trait = selectable_traits[sel];
              i_newch.set_data("num_cost", abs(trait_cost(cur_trait)));
              if (trait_cost(cur_trait) >= 0) {
                i_newch.set_data("text_cost", "<c=yellow>Cost:<c=/>");
              } else {
                i_newch.set_data("text_cost", "<c=yellow>Earns:<c=/>");
              }
              if (trait_cost(cur_trait) > points) {
                i_newch.set_data("num_cost", c_red);
              } else {
                i_newch.set_data("num_cost", c_white);
              }
              i_newch.set_data("text_description",
                               trait_description(cur_trait));
            } break;

            case '8':
            case 'k':
            case KEY_UP:
            {
              i_newch.add_data("list_traits", -1);
              int sel = i_newch.get_int("list_traits");
              Trait_id cur_trait = selectable_traits[sel];
              i_newch.set_data("num_cost", abs(trait_cost(cur_trait)));
              if (trait_cost(cur_trait) >= 0) {
                i_newch.set_data("text_cost", "<c=yellow>Cost:<c=/>");
              } else {
                i_newch.set_data("text_cost", "<c=yellow>Earns:<c=/>");
              }
              if (trait_cost(cur_trait) > points) {
                i_newch.set_data("num_cost", c_red);
              } else {
                i_newch.set_data("num_cost", c_white);
              }
              i_newch.set_data("text_description",
                               trait_description(cur_trait));
            } break;

            case '\n':
            case ' ':
            {
              int sel = i_newch.get_int("list_traits");
              Trait_id cur_trait = selectable_traits[sel];
              if (has_trait(cur_trait)) {
                traits[cur_trait] = false;
                points += trait_cost(cur_trait);
                num_traits--;
                traits_list = get_trait_list(this);
              } else if (points >= trait_cost(cur_trait) && num_traits < 5){
                traits[cur_trait] = true;
                points -= trait_cost(cur_trait);
                num_traits++;
                traits_list = get_trait_list(this);
              }
              i_newch.set_data("num_traits_left", 5 - num_traits);
              if (num_traits >= 5) {
                i_newch.set_data("num_traits_left", c_red);
              }
            } break;

          } // switch (ch)
        } break;

        case NCS_PROFESSION: {
          switch (ch) {
            case '2':
            case 'j':
            case KEY_DOWN:
            {
              i_newch.add_data("list_professions", 1);
              std::string prof_name = i_newch.get_str("list_professions");
              prof_name = remove_color_tags(prof_name);
              Profession* cur_prof = PROFESSIONS.lookup_name(prof_name);
              if (!cur_prof) {
                debugmsg("No such profession as '%s'!", prof_name.c_str());
                return false;
              }
              i_newch.set_data("text_description", cur_prof->description);
            } break;

            case '8':
            case 'k':
            case KEY_UP:
            {
              i_newch.add_data("list_professions", -1);
              std::string prof_name = i_newch.get_str("list_professions");
              prof_name = remove_color_tags(prof_name);
              Profession* cur_prof = PROFESSIONS.lookup_name(prof_name);
              if (!cur_prof) {
                debugmsg("No such profession as '%s'!", prof_name.c_str());
                return false;
              }
              i_newch.set_data("text_description", cur_prof->description);
            } break;

            case '\n':
            case ' ':
            {
              std::string prof_name = i_newch.get_str("list_professions");
              prof_name = remove_color_tags(prof_name);
              Profession* cur_prof = PROFESSIONS.lookup_name(prof_name);
              if (!cur_prof) {
                debugmsg("No such profession as '%s'!", prof_name.c_str());
                return false;
              }
              set_profession(cur_prof);
              profession_list = get_profession_list(this);
            } break;

          } // switch (ch)
        } break;

        case NCS_DESCRIPTION: {
          if (ch == '/') {
            male = !male;
            if (male) {
              i_newch.set_data("text_male",   "<c=yellow>Male<c=/>");
              i_newch.set_data("text_female", "<c=dkgray>Female<c=/>");
            } else {
              i_newch.set_data("text_male",   "<c=dkgray>Male<c=/>");
              i_newch.set_data("text_female", "<c=yellow>Female<c=/>");
            }
          } else {
/* Let the interface handle name entry; this includes cursor movement,
 * backspace, etc.  The only downside is that this allows entry of "invalid"
 * name characters like "'&^%$#@ etc.  Bad?
 */
            cuss::element* entry = i_newch.find_by_name("entry_name");
            entry->handle_keypress(ch);
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

      i_newch.ref_data("num_points", &points);

      switch (cur_screen) {

        case NCS_STATS:
          cur_stat = STATSEL_STR;
          i_newch.set_data("text_strength",     "<c=ltblue>Strength<c=/>");
          i_newch.set_data("text_dexterity",    "<c=ltgray>Dexterity<c=/>");
          i_newch.set_data("text_perception",   "<c=ltgray>Perception<c=/>");
          i_newch.set_data("text_intelligence", "<c=ltgray>Intelligence<c=/>");
          i_newch.ref_data("num_strength",     &stats.strength);
          i_newch.ref_data("num_dexterity",    &stats.dexterity);
          i_newch.ref_data("num_perception",   &stats.perception);
          i_newch.ref_data("num_intelligence", &stats.intelligence);
          i_newch.set_data("text_description", get_stat_description(cur_stat));
          break;

        case NCS_TRAITS: {
          i_newch.select("list_traits");
          i_newch.ref_data("list_traits", &traits_list);
          int sel = i_newch.get_int("list_traits");
          Trait_id cur_trait = selectable_traits[sel];
          i_newch.set_data("text_description", trait_description(cur_trait));
          i_newch.set_data("num_cost", abs(trait_cost(cur_trait)));
          if (trait_cost(cur_trait) >= 0) {
            i_newch.set_data("text_cost", "<c=yellow>Cost:<c=/>");
          } else {
            i_newch.set_data("text_cost", "<c=yellow>Earns:<c=/>");
          }
          if (trait_cost(cur_trait) > points) {
            i_newch.set_data("num_cost", c_red);
          } else {
            i_newch.set_data("num_cost", c_white);
          }
          i_newch.set_data("num_traits_left", 5 - num_traits);
          if (num_traits >= 5) {
            i_newch.set_data("num_traits_left", c_red);
          }
        } break;

        case NCS_PROFESSION: {
          i_newch.select("list_professions");
          i_newch.ref_data("list_professions", &profession_list);
          std::string prof_name = i_newch.get_str("list_professions");
          prof_name = remove_color_tags(prof_name);
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
    } // if (changed_screen)
  }

// Now set up our skills and equipment based on our profession
  if (!profession) {
    debugmsg("Character creation finished without a profession!");
    return false;
  }
  std::vector<Item_type_chance> prof_items = profession->items.item_types;
  for (int i = 0; i < prof_items.size(); i++) {
    int count = prof_items[i].number;
    Item tmp_it(prof_items[i].item);
    for (int i = 0; i < count; i++) {
      if (tmp_it.get_item_class() == ITEM_CLASS_CLOTHING) {
        items_worn.push_back(tmp_it);
      } else {
        inventory.push_back(tmp_it);
      }
    }
  }

  skills = profession->skills;

// The "Durable" trait needs to be set up here.
  if (has_trait(TRAIT_DURABLE)) {
    for (int i = 0; i < HP_PART_MAX; i++) {
      current_hp[i] = 115;
      max_hp[i] = 115;
    }
  }

// Set up our max mental skill levels, based on int.
  int max_sk = stats.intelligence / 5;
  for (int i = 0; i < SKILL_MAX; i++) {
    Skill_type sk = Skill_type(i);
// If we start with some skill from our profession, increase the max by that
    int cap = max_sk + skills.get_level(sk);
    if (is_skill_mental(sk)) {
      skills.set_max_level(sk, cap);
    }
  }

// Myopic characters get free glasses
  if (has_trait(TRAIT_MYOPIC)) {
    Item_type* glasses = ITEM_TYPES.lookup_name("glasses");
    if (!glasses) {
      debugmsg("No item 'glasses' exists - required for the Myopic trait!");
      return false;
    }
    Item tmp_it(glasses);
    items_worn.push_back(tmp_it);
  }

// Set max stats to current stats.
  stats.strength_max     = stats.strength;
  stats.dexterity_max    = stats.dexterity;
  stats.perception_max   = stats.perception;
  stats.intelligence_max = stats.intelligence;

// Assign some starting missions!
  assign_personal_missions();

  return true;
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
      name << "<c=green>";
    } else if (i < TRAIT_MAX_NEUTRAL) {
      name << "<c=brown>";
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
