#include "entity.h"
#include "player.h"
#include "cuss.h"

enum New_char_screen
{
  NCS_STATS,
  NCS_TRAITS,
  NCS_PROFESSION,
  NCS_DESCRIPTION
};

enum Stat_selected
{
  STATSEL_STR,
  STATSEL_DEX,
  STATSEL_PER,
  STATSEL_INT
};

bool Player::create_new_character()
{
  Window w_newch(0, 0, 80, 24);
  cuss::interface i_newch;
  if (!i_newch.load_from_file(CUSS_DIR + "/i_newchar_stats.cuss")) {
    return false;
  }

  New_char_screen cur_screen = NCS_STATS;
  Stat_selected curstat = STATSEL_STR;

  while (true) {  // We'll exit this function via keypresses, always
    switch (cur_screen) {
      case NCS_STATS:
        i_newch.set_data("num_strength",     stats.strength);
        i_newch.set_data("num_dexterity",    stats.dexterity);
        i_newch.set_data("num_perception",   stats.perception);
        i_newch.set_data("num_intelligence", stats.intelligence);
