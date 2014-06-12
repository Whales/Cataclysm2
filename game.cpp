#include "game.h"
#include "window.h"
#include "stringfunc.h"
#include "map.h"
#include "rng.h"
#include "pathfind.h"
#include "files.h"      // For CUSS_DIR
#include "help.h"
#include <stdarg.h>
#include <math.h>
#include <sstream>

std::vector<std::string> get_pickup_strings(std::vector<Item> *items,
                                            std::vector<bool> *picking_up);
std::string pickup_string(Item *item, char letter, bool picking_up);

Game::Game()
{
  map       = NULL;
  worldmap  = NULL;
  w_map     = NULL;
  w_hud     = NULL;
  player    = NULL;
  last_target = -1;
  new_messages = 0;
  next_item_uid = 0;
  next_furniture_uid = 0;
  temp_light_level = 0;
  game_over = false;
}

Game::~Game()
{
  if (map) {
    delete map;
  }
  if (worldmap) {
    delete worldmap;
  }
  if (w_map) {
    delete w_map;
  }
}
  
bool Game::setup_ui()
{
  if (!i_hud.load_from_file(CUSS_DIR + "/i_hud.cuss")) {
    return false;
  }
  int xdim, ydim;
  get_screen_dims(xdim, ydim);
  int win_size = ydim;
/* Commenting this out for now - the extra empty space caused issues
  if (win_size % 2 == 0) {
    win_size--; // Only odd numbers allowed!
  }
*/
  w_map = new Window(0, 0, win_size, win_size);
  w_hud = new Window(win_size, 0, xdim - win_size, ydim);
// Attempt to resize the messages box to be as tall as the window allows
  cuss::element* messages = i_hud.select("text_messages");
  if (!messages) {
    debugmsg("Couldn't find element text_messages in i_hud");
    return false;
  }
  messages->sizey = ydim - messages->posy;

// Populate Worldmap_names with all the Worldmaps in SAVE_DIR/worlds
  worldmap_names = files_in(SAVE_DIR + "/worlds", ".map");
// Strip the ".map" from each Worldmap name
  for (int i = 0; i < worldmap_names.size(); i++) {
    size_t suffix = worldmap_names[i].find(".map");
    if (suffix != std::string::npos) {
      worldmap_names[i] = worldmap_names[i].substr(0, suffix);
    }
  }
  return true;
}

bool Game::setup_new_game(int world_index)
{
  worldmap = new Worldmap;
  if (world_index >= 0 && world_index < worldmap_names.size()) {
    std::string world_file = SAVE_DIR + "/worlds/" +
                             worldmap_names[world_index] + ".map";
    if (!worldmap->load_from_file(world_file)) {
      debugmsg("Couldn't load worldmap from '%s'.", world_file.c_str());
      return false;
    }
  } else {
    worldmap->generate();
  }

  map = new Map;
// The second argument of 0 means "on the main island"
  Point start = worldmap->random_tile_with_terrain("beach", 0);
// Set the starting point to a shipwreck beach!
  worldmap->set_terrain(start.x, start.y, "beach_shipwreck");
// And then generate our map.
  map->generate(worldmap, start.x, start.y, 0);
  worldmap->set_terrain(start.x, start.y, "beach");

  player = new Player;
  player->prep_new_character();
// Player::create_new_character() returns false if the user cancels the process.
  if (!player->create_new_character()) {
    return false;
  }
// entities[0] should always be the player!
  entities.add_entity(player);

  time = Time(0, 0, 8, 1, SEASON_SPRING, STARTING_YEAR);
  last_target = -1;
  new_messages = 0;
  next_item_uid = 0;
  next_furniture_uid = 0;
  game_over = false;
  return true;
}

bool Game::starting_menu()
{
  cuss::interface i_menu;
  Window w_menu;
  if (!i_menu.load_from_file(CUSS_DIR + "/i_starting_menu.cuss")) {
    return false;
  }

  std::string motd = slurp_file(DATA_DIR + "/motd.txt");

  i_menu.set_data("text_motd", motd);

  int current_world = -1;

  while (true) {
    i_menu.draw(&w_menu);
    w_menu.refresh();
    long ch = input();

    if (ch == 'n' || ch == 'N') {
      if (setup_new_game(current_world)) {
        return true;
      }

    } else if (ch == 'l' || ch == 'L') {
// TODO: Load character here
      return true;

    } else if (ch == 'w' || ch == 'W') {
      current_world = world_screen();

    } else if (ch == 'h' || ch == 'H') {
      help_screen();

    } else if (ch == 'q' || ch == 'Q') {
      return false;
    }

  }
  return false; // Should never be reached
}

int Game::world_screen()
{
  cuss::interface i_worlds;
  Window w_worlds(0, 0, 80, 24);
  if (!i_worlds.load_from_file(CUSS_DIR + "/i_worlds.cuss")) {
    return -1;
  }
  i_worlds.set_data("list_worlds", worldmap_names);
  i_worlds.select("list_worlds");

  while (true) {  // We'll exit when the player hits enter
    i_worlds.draw(&w_worlds);
    w_worlds.refresh();
    long ch = input();

    if (ch == 'c' || ch == 'C') {
      create_world();
// Repopulate list_worlds with (hopefully) a new world name.
      i_worlds.set_data("list_worlds", worldmap_names);
      i_worlds.select("list_worlds");

    } else if (ch == '\n') {
      return i_worlds.get_int("list_worlds");

    } else {
      i_worlds.handle_action(ch); // Handles any scrolling
    }
  }
  return -1;
}

void Game::create_world()
{
  cuss::interface i_editor;
  Window w_editor(0, 0, 80, 24);
  if (!i_editor.load_from_file(CUSS_DIR + "/i_world_editor.cuss")) {
    return;
  }

  Worldmap tmp_world;
  std::string world_name;
  i_editor.ref_data("entry_name", &world_name);
  i_editor.select("entry_name");

// TODO: Enable climate selection, and generate the world based on the climate.
  while (true) {  // We'll exit this loop via player input.
    i_editor.draw(&w_editor);
    w_editor.refresh();

    long ch = input();

    if (ch == '!') {
      tmp_world.randomize_name();
      world_name = tmp_world.get_name();

    } else if (ch == '\n') {
      if (tmp_world.get_name().empty()) {
        popup("<c=ltred>Error:<c=/> Name is empty!");
      } else {
        popup_nowait("Generating world, please wait...");
        tmp_world.generate();
        if (tmp_world.save_to_name()) {
          worldmap_names.push_back(tmp_world.get_name());
        }
      }
      return;

    } else if (ch == KEY_ESC) {
      return;

    } else if (ch == '@') {
// TODO: Unlock advanced options.

    } else {
      i_editor.handle_keypress(ch);
    }
  }
}


bool Game::main_loop()
{
// Sanity check
  if (!w_map || !w_hud || !player || !worldmap || !map) {
    return false;
  }
  if (game_over) {
    return false;
  }
// Reset all temp values.
  reset_temp_values();
// Process active items; these may set temp values!
  process_active_items();
/* TODO:  It's be nice to move all of this to Player::take_turn().  Then we
 *        won't have to special case it - it'd just be another entity taking
 *        their turn!
 */
// Give the player their action points
  player->gain_action_points();
// Set all values, incur hunger/thirst, etc.
  player->start_turn();
  while (player->action_points > 0) {
// Handle the player's activity (e.g. reloading, crafting, etc)
    handle_player_activity();
// Update the map in case we need to right now
    shift_if_needed();
// Print everything (update_hud() and map->draw())
    draw_all();

// The player doesn't get to give input if they have an active activity.
    if (!player->activity.is_active()) {
      long ch = input();
// Fetch the action bound to whatever key we pressed...
      Interface_action act = KEYBINDINGS.bound_to_key(ch);
// ... and do that action.
      do_action(act);
    }
  }
// Map processes fields after the player
  map->process_fields();
// Shift the map - it's likely that the player moved or something
  shift_if_needed();
// Now all other entities get their turn
  move_entities();
// Maybe a monster killed us
  if (game_over) {
    return false; // This terminates the game
  }
// Advance the turn
  time.increment();
  return true;    // This keeps the game going
}

void Game::reset_temp_values()
{
  temp_light_level = 0;
}

void Game::do_action(Interface_action act)
{
  switch (act) {
    case IACTION_MOVE_N:  player_move( 0, -1);  break;
    case IACTION_MOVE_S:  player_move( 0,  1);  break;
    case IACTION_MOVE_W:  player_move(-1,  0);  break;
    case IACTION_MOVE_E:  player_move( 1,  0);  break;
    case IACTION_MOVE_NW: player_move(-1, -1);  break;
    case IACTION_MOVE_NE: player_move( 1, -1);  break;
    case IACTION_MOVE_SW: player_move(-1,  1);  break;
    case IACTION_MOVE_SE: player_move( 1,  1);  break;
    case IACTION_PAUSE:   player->pause();      break;

    case IACTION_MOVE_UP:
      if (!map->has_flag(TF_STAIRS_UP, player->pos)) {
        add_msg("You cannot go up here.");
        player_move_vertical(1);  // Snuck this in for debugging purposes
      } else {
        player_move_vertical(1);
      }
      break;

    case IACTION_MOVE_DOWN:
      if (!map->has_flag(TF_STAIRS_DOWN, player->pos)) {
        add_msg("You cannot go down here.");
        player_move_vertical(-1); // Snuck this in for debugging purposes
      } else {
        player_move_vertical(-1);
      }
      break;

    case IACTION_PICK_UP:
// Make sure we're not sealed...
      if (map->has_flag(TF_SEALED, player->pos)) {
        add_msg("<c=dkgray>That %s is sealed; you cannot retrieve items \
there.<c=/>", map->get_name(player->pos).c_str());

      } else if (map->item_count(player->pos) == 0) {
        add_msg("No items here.");

      } else if (map->item_count(player->pos) == 1) {
// Only one item - no need for the interface
        std::vector<Item> *items = map->items_at(player->pos);
        if (player->add_item( (*items)[0] )) {
          items->clear();
        }
      } else {
        pickup_items(player->pos);
      }
      break;

    case IACTION_OPEN: {
      add_msg("<c=ltgreen>Open where? (Press direction key)<c=/>");
      draw_all();
      Point dir = input_direction(input());
      if (dir.x == -2) { // Error
        add_msg("Invalid direction.");
      } else {
        Tripoint open = player->pos + dir;
        if (map->apply_signal("open", open, player)) {
          player->use_ap(100);
        }
      }
    } break;

    case IACTION_CLOSE: {
      add_msg("<c=ltgreen>Close where? (Press direction key)<c=/>");
      draw_all();
      Point dir = input_direction(input());
      if (dir.x == -2) { // Error
        add_msg("Invalid direction.");
      } else {
        Tripoint close = player->pos + dir;
        Entity* ent = entities.entity_at(close);
        if (ent == player) {
          add_msg("Maybe you should move out of the doorway first.");
        } else if (ent) {
          add_msg("There's a %s in the way.", ent->get_name().c_str());
        } else if (map->furniture_at(close)) {
          add_msg("There's some furniture in the way.");
        } else if (map->apply_signal("close", close, player)) {
          player->use_ap(100);
        }
      }
    } break;

    case IACTION_SMASH: {
      add_msg("<c=ltgreen>Smash where? (Press direction key)<c=/>");
      draw_all();
      Point dir = input_direction(input());
      if (dir.x == -2) { // Error
        add_msg("Invalid direction.");
      } else {
        Tripoint sm = player->pos + dir;
        if (!map->is_smashable(sm)) {
          add_msg("Nothing to smash there.");
        } else {
          add_msg("You smash the %s.", map->get_name(sm).c_str());
          map->smash(sm, player->std_attack().roll_damage());
          player->use_ap(100);
        }
      }
    } break;

    case IACTION_EXAMINE: {
      add_msg("<c=ltgreen>Examine where? (Press direction key)<c=/>");
      draw_all();
      Point dir = input_direction(input());
      if (dir.x == -2) { // Error
        add_msg("Invalid direction.");
      } else {
        Tripoint examine = player->pos + dir;
// Can't pick up items if we're sealed
        if (map->has_flag(TF_SEALED, examine)) {
          add_msg("<c=dkgray>That %s is sealed; you cannot retrieve items \
there.<c=/>", map->get_name(examine).c_str());

        } else if (map->item_count(examine) > 0) {
          pickup_items(examine);
        }

        std::stringstream description;
        description << "That is " << map->get_name_indefinite(examine) << ".";
        if (map->furniture_at(examine)) {
          description << "  You can drag it using the <c=yellow>grab<c=/> " <<
                         "command (<c=yellow>" <<
                         KEYBINDINGS.describe_bindings_for(IACTION_GRAB) <<
                         "<c=/>).";
          if (TESTING_MODE) {
            add_msg("<c=pink>Furniture uid %d.<c=/>",
                    map->furniture_at(examine)->get_uid());
          }
        }

        add_msg(description.str());
      }
    } break;

    case IACTION_GRAB: {
      if (player->dragged.empty()) {
        add_msg("<c=ltgreen>Grab where? (Press direction key)<c=/>");
        draw_all();
        Point dir = input_direction(input());
        if (dir.x == -2) { // Error
          add_msg("Invalid direction.");
        } else {
          Tripoint target = player->pos + dir;
          player->dragged = map->grab_furniture(player->pos, target);
          if (player->dragged.empty()) {
            add_msg("Nothing to grab there.");
          } else {
            add_msg("You grab the %s.", player->get_dragged_name().c_str());
            if (TESTING_MODE) {
              for (int i = 0; i < player->dragged.size(); i++) {
                add_msg("%s", player->dragged[i].pos.str().c_str());
              }
            }
          }
        }
      } else {  // We're already dragging something; so let go!
        add_msg("You let go of the %s.", player->get_dragged_name().c_str());
        player->dragged.clear();
      }
    } break;

    case IACTION_INVENTORY: {
      Item it = player->inventory_single();
      Item_action act = it.show_info();

      switch (act) {

        case IACT_WIELD:
          add_msg( player->wield_item_message(it) );
          player->wield_item_uid(it.get_uid());
          break;

        case IACT_WEAR:
          add_msg( player->wear_item_message(it) );
          player->wear_item_uid(it.get_uid());
          break;

        case IACT_DROP:
          add_msg( player->drop_item_message(it) );
          player->remove_item_uid(it.get_uid());
// TODO: Dropping may fail sometimes(?), so don't automatically add the item
          map->add_item(it, player->pos);
          break;

        case IACT_EAT:
          add_msg( player->eat_item_message(it) );
          player->eat_item_uid(it.get_uid());
          break;

        case IACT_APPLY:
          add_msg( player->apply_item_message(it) );
          player->apply_item_uid(it.get_uid());
          break;

        case IACT_UNLOAD:
// TODO: Put unload code here
          break;

        case IACT_RELOAD:
          player->reload_prep(it.get_uid());
          break;

        case IACT_BUTCHER:
// TODO: Put butcher code here
          break;
      } // switch (act)

    } break;

    case IACTION_DROP: {
      std::vector<Item> dropped = player->drop_items();
      std::stringstream message;
      message << "You drop " << list_items(&dropped);
      for (int i = 0; i < dropped.size(); i++) {
        map->add_item(dropped[i], player->pos);
      }
      add_msg( message.str() );
    } break;

    case IACTION_WIELD: {
      Item it = player->inventory_single();
      add_msg( player->sheath_weapon_message() );
      player->sheath_weapon();
      add_msg( player->wield_item_message(it) );
      player->wield_item_uid(it.get_uid());
    } break;

    case IACTION_WEAR: {
      Item it = player->inventory_single();
      add_msg( player->wear_item_message(it) );
      player->wear_item_uid(it.get_uid());
    } break;

    case IACTION_TAKE_OFF: {
      Item it = player->inventory_single();
      add_msg( player->take_off_item_message(it) );
      player->take_off_item_uid(it.get_uid());
    } break;

    case IACTION_APPLY: {
      Item it = player->inventory_single();
      Item_action act = it.default_action();
      switch (act) {
        case IACT_NULL:
          add_msg("Can't do anything with that item.");
          break;
        case IACT_WIELD:
          add_msg( player->wield_item_message(it) );
          player->wield_item_uid(it.get_uid());
          break;
        case IACT_WEAR:
          add_msg( player->wear_item_message(it) );
          player->wear_item_uid(it.get_uid());
          break;
        case IACT_EAT:
          add_msg( player->eat_item_message(it) );
          player->eat_item_uid(it.get_uid());
        case IACT_APPLY:
// Need to redraw the map
          add_msg( player->apply_item_message(it) );
          player->apply_item_uid(it.get_uid());
          break;
      }
    } break;

    case IACTION_RELOAD: {
      Item it = player->inventory_single();
      player->reload_prep(it.get_uid());
    } break;

    case IACTION_RELOAD_EQUIPPED:
      player->reload_prep(player->weapon.get_uid());
      break;

    case IACTION_THROW: {
      Item it = player->inventory_single();
      if (!it.is_real()) {
        add_msg("Never mind.");
      } else {
        int x = player->pos.x, y = player->pos.y,
            range = player->weapon.get_fired_attack().range;
        Tripoint target = target_selector(x, y, range, true, true);
        if (target.x == -1) { // We canceled
          add_msg("Never mind.");
        } else {
// If we actually targeted an entity, set that to our last target.
          Entity* targeted_entity = entities.entity_at(target);
          if (targeted_entity) {
            last_target = targeted_entity->uid;
          }
          player->remove_item_uid(it.get_uid(), 1);
          Ranged_attack att = player->throw_item(it);
          launch_projectile(player, it, att, player->pos, target);
        }
      }
    } break;

    case IACTION_FIRE:
      if (player->can_fire_weapon()) {
        int x = player->pos.x, y = player->pos.y,
            range = player->weapon.get_fired_attack().range;
        Tripoint target = target_selector(x, y, range, true, true);
        if (target.x == -1) { // We canceled
          add_msg("Never mind.");
// If we target ourself, confirm that we want to shoot ourselves...
        } else if (target != player->pos ||
                   msg_query_yn("Really target yourself?")) {
// If we actually targeted an entity, set that to our last target.
          Entity* targeted_entity = entities.entity_at(target);
          if (targeted_entity) {
            last_target = targeted_entity->uid;
          }
// And do the actual attack!
          Ranged_attack att = player->fire_weapon();
          launch_projectile(player, player->weapon, att, player->pos, target);
        }
      }
      break;

    case IACTION_ADVANCE_FIRE_MODE:
      player->weapon.advance_fire_mode();
      add_msg( player->advance_fire_mode_message() );
      break;

    case IACTION_EAT: {
      Item it = player->inventory_single();
      add_msg( player->eat_item_message(it) );
      player->eat_item_uid(it.get_uid());
    } break;

    case IACTION_MESSAGES_SCROLL_BACK:
      i_hud.add_data("text_messages", -1);
      break;

    case IACTION_MESSAGES_SCROLL_FORWARD:
      i_hud.add_data("text_messages",  1);
      break;

    case IACTION_VIEW_WORLDMAP: {
      Point p = map->get_center_point();
      Point got = worldmap->get_point(p.x, p.y);
// Adjust to match the upper-left corner
      got.x -= MAP_SIZE / 2;
      got.y -= MAP_SIZE / 2;
      if (TESTING_MODE) {
        map->generate(worldmap, got.x, got.y);
      }
    }  break;

    case IACTION_DEBUG:
      if (!TESTING_MODE) {
        add_msg("<c=red>To access debug commands, run the program with the \
--test flag.");
      } else {
        debug_command();
      }
      break;

    case IACTION_QUIT:
      if (query_yn("Commit suicide?")) {
        game_over = true;
        player->action_points = 0;
      }
      break;
  }
}

void Game::move_entities()
{
  clean_up_dead_entities();
  //scent_map = map->get_dijkstra_map(player->pos, 15);
// First, give all entities action points
  for (std::list<Entity*>::iterator it = entities.instances.begin();
       it != entities.instances.end();
       it++) {
    if (!(*it)->is_player()) {
      (*it)->gain_action_points();
    }
  }
/* Loop through the entities, giving each one a single turn at a time.
 * Stop when we go through a loop without finding any entities that can
 * take a turn.
 */
  bool all_done = true;
  do {
    all_done = true;
    for (std::list<Entity*>::iterator it = entities.instances.begin();
         it != entities.instances.end();
         it++) {
      Entity* ent = *it;
      if (!ent->is_player() && ent->action_points > 0 && !ent->dead) {
        ent->take_turn();
        all_done = false;
      }
    }
  } while (!all_done);

  clean_up_dead_entities(); // Just in case an entity killed itself

}

void Game::clean_up_dead_entities()
{
  std::list<Entity*>::iterator it = entities.instances.begin();
  while (it != entities.instances.end()) {
    Entity *ent = (*it);
    if ( ent->dead ) {
      if (player->can_see(map, ent->pos)) {
        if (ent->killed_by_player) {
          add_msg("You kill %s!", ent->get_name_to_player().c_str());
        } else {
          add_msg("%s dies!", ent->get_name_to_player().c_str());
        }
      }
      ent->die();
      delete ent;
      it = entities.erase(it);
    } else {
      it++;
    }
  }
}

void Game::handle_player_activity()
{
  Player_activity *act = &(player->activity);
  if (act->is_active()) {
    if (act->duration <= player->action_points) {
      player->action_points -= act->duration;
      complete_player_activity();
    } else {
      act->duration -= player->action_points;
      player->action_points = 0;
    }
  }
}

void Game::complete_player_activity()
{
  Player_activity *act = &(player->activity);
  switch (act->type) {

    case PLAYER_ACTIVITY_NULL:
    case PLAYER_ACTIVITY_WAIT:
      break; // Nothing happens

    case PLAYER_ACTIVITY_RELOAD: {
      Item *reloaded = player->ref_item_uid(act->primary_item_uid);
      if (!reloaded) {
        debugmsg("Completed reload, but the item wasn't there!");
        return;
      }
      add_msg("You reload your %s.", reloaded->get_name().c_str());
      reloaded->reload(player, act->secondary_item_uid);
    } break;

    default:
      debugmsg("Our switch doesn't know how to handle action %d, '%s'",
               act->type, act->get_name().c_str());
  }
  player->activity = Player_activity();
}

void Game::process_active_items()
{
  for (int i = 0; i < active_items.size(); i++) {
    active_items[i]->process_active();
  }
}

void Game::shift_if_needed()
{
  int min = SUBMAP_SIZE * (MAP_SIZE / 2), max = min + SUBMAP_SIZE - 1;
  int shiftx = 0, shifty = 0;
  if (player->pos.x < min) {
    shiftx = -1 + (player->pos.x - min) / SUBMAP_SIZE;
  } else if (player->pos.x > max) {
    shiftx =  1 + (player->pos.x - max) / SUBMAP_SIZE;
  }
  if (player->pos.y < min) {
    shifty = -1 + (player->pos.y - min) / SUBMAP_SIZE;
  } else if (player->pos.y > max) {
    shifty =  1 + (player->pos.y - max) / SUBMAP_SIZE;
  }
  map->shift(worldmap, shiftx, shifty);
  for (std::list<Entity*>::iterator it = entities.instances.begin();
       it != entities.instances.end();
       it++) {
    (*it)->shift(shiftx, shifty);
  }
}

void Game::make_sound(std::string desc, Tripoint pos)
{
  make_sound(desc, pos.x, pos.y);
}

void Game::make_sound(std::string desc, Point pos)
{
  make_sound(desc, pos.x, pos.y);
}

void Game::make_sound(std::string desc, int x, int y)
{
  if (desc.empty()) {
    return;
  }
// TODO: Alert monsters
  Direction_full dir = get_general_direction(player->pos,
                                             Point(x, y));
// TODO: Don't hardcode color
  if (dir == DIRFULL_NULL) { // On top of the player!
    add_msg("<c=ltblue>You hear %s<c=/>", desc.c_str());
  } else {
    add_msg("<c=ltblue>To the <c=ltred>%s<c=ltblue>, you hear %s<c=/>",
            Direction_name(dir).c_str(), desc.c_str());
  }
}

void Game::launch_projectile(Ranged_attack attack,
                             Tripoint origin, Tripoint target)
{
  launch_projectile(NULL, attack, origin, target);
}

void Game::launch_projectile(Item it, Ranged_attack attack, Tripoint origin,
                             Tripoint target)
{
  launch_projectile(NULL, it, attack, origin, target);
}

void Game::launch_projectile(Entity* shooter, Ranged_attack attack,
                             Tripoint origin, Tripoint target)
{
  launch_projectile(shooter, Item(), attack, origin, target);
}

void Game::launch_projectile(Entity* shooter, Item it, Ranged_attack attack,
                             Tripoint origin, Tripoint target)
{
// Set up some nouns and verbs for messages (far below)
  std::string shooter_name, verb = "shoot", miss_verb = "miss",
              graze_verb = "graze";
  if (shooter) {
    shooter_name = shooter->get_name_to_player();
    verb = shooter->conjugate(verb);
    miss_verb = shooter->conjugate(miss_verb);
    graze_verb = shooter->conjugate(graze_verb);
  } else {
/* If there's no shooter, that implies that natural forces launched the
 * projectile, e.g. rubble from an explosion.  In that case, we want our hit
 * message to be "A piece of rubble hits you!"
 */
    if (it.is_real()) {
      shooter_name = it.get_name_indefinite();
    }
    verb = "hits";
    miss_verb = "misses";
    graze_verb = "grazes";
  }

// We want to loop through once for each round.  Also, (target) will probably
// change between rounds, so we recalculate range etc.
  int retarget_range = 0;
  if (attack.rounds < 1) {
    attack.rounds = 1;
  } else if (it.is_real()) {
/* If we're firing a multiple-round weapon, and we kill our original target or
 * wind up targeting a space without a monster in it, we can re-target to a
 * nearby monster between rounds.  The range of this retargeting is dependent
 * upon our skills.
 */
    Item_type_launcher* launcher_type =
      static_cast<Item_type_launcher*>(it.type);
    Skill_type launcher_skill = launcher_type->skill_used;
    if (shooter) {
      retarget_range = shooter->skills.get_level(launcher_skill) +
                       shooter->skills.get_level(SKILL_LAUNCHERS) / 2;
      retarget_range = sqrt(retarget_range);
    }
  }

/* If we're NOT targeting an entity, then we are probably shooting at scenery
 * and we should not use our retargeting ability!
 */
  bool targeting_entity = entities.entity_at(target);

  for (int round = 0; round < attack.rounds; round++) {
// Figure out the range to the target
    int range = rl_dist(origin, target);

// We calculate angle individually for every pellet...
    int num_pellets = attack.pellets;
    if (num_pellets < 1) {
      num_pellets = 1;
    }
/* We summarize all pellets in a single message to avoid spamming the player
 * with 20 messages for a single round of 00 shot.  We do this with a few
 * vectors; since we may hit more than one enemy and we want messages for all.
 */

    std::vector<Entity*> entities_hit;
    std::vector<int> total_damage;
    std::vector<Ranged_hit_type> best_hit;
/* We track all the tiles_hit by all pellets.  After we finish processing all
 * pellets, we move (target) to the average of all pellets.
 */
    std::vector<Tripoint> tiles_hit;

    for (int pellet = 0; pellet < num_pellets; pellet++) {
      int angle_missed_by = attack.roll_variance();
/*
      if (TESTING_MODE) {
        debugmsg("angle %d (%s)", angle_missed_by, attack.variance.str().c_str());
      }
*/
// Use 1800 since attack.variance is measured in 10ths of a degree
      double distance_missed_by = range * tan(angle_missed_by * PI / 1800);
      int tiles_off = int(distance_missed_by);
      Tripoint curtarget = target;
      if (tiles_off >= 1) {
        curtarget.x += rng(0 - tiles_off, tiles_off);
        curtarget.y += rng(0 - tiles_off, tiles_off);
      }
      tiles_hit.push_back(curtarget);
// fine_distance is used later to see if we hit the target or "barely missed"
      int fine_distance = 100 * (distance_missed_by - tiles_off);

      std::vector<Tripoint> path = map->line_of_sight(origin, curtarget);
      if (path.empty()) { // Lost line of sight at some point
        path = line_to(origin, curtarget);
      }

// We track i outside of the function, because we need it to know where the
// projectile stopped.
      int i = 0;
      bool stopped = false;
      while (!stopped && i < path.size()) {
        if (map->move_cost(path[i].x, path[i].y) == 0) {
// It's a solid tile, so let's try to smash through it!
          map->smash(path[i].x, path[i].y, attack.roll_damage(), false);
          if (map->move_cost(path[i].x, path[i].y) == 0) {
            stopped = true; // Couldn't get through the terrain!
            i--; // Stop at the terrain before the solid one
          }
        } else {
// Drop a field in our wake?
          if (attack.wake_field.exists()) {
            attack.wake_field.drop(Tripoint(path[i].x, path[i].y, path[i].z),
                                   shooter_name);
          }
// Did we hit an entity?
          Entity* entity_hit = entities.entity_at(path[i].x, path[i].y);
          if (entity_hit) {
            bool hit;
            Ranged_hit_type hit_type;
            if (i == path.size() - 1) {
              hit = rng(0, 100) >= fine_distance;
              if (fine_distance <= 10) {
                hit_type = RANGED_HIT_HEADSHOT;
              } else if (fine_distance <= 35) {
                hit_type = RANGED_HIT_CRITICAL;
              } else if (fine_distance <= 75) {
                hit_type = RANGED_HIT_NORMAL;
              } else {
                hit_type = RANGED_HIT_GRAZE;
              }
            } else {
              hit = one_in(3);// TODO: Incorporate the size of the monster
              if (hit) {
                int hit_roll = rng(1, 100);
                if (hit_roll <= 10) {
                  hit_type = RANGED_HIT_HEADSHOT;
                } else if (hit_roll <= 35) {
                  hit_type = RANGED_HIT_CRITICAL;
                } else if (hit_roll <= 75) {
                  hit_type = RANGED_HIT_NORMAL;
                } else {
                  hit_type = RANGED_HIT_GRAZE;
                }
              }
            }

            if (hit) {
// Check if we've already hit this entity once before.
              bool found_entity = false;
              int entity_index;
              for (entity_index = 0;
                   !found_entity && entity_index < entities_hit.size();
                   entity_index++) {
                if (entities_hit[entity_index] == entity_hit) {
                  found_entity = true;
                }
              }
              if (!found_entity) {
                entities_hit.push_back( entity_hit );
                total_damage.push_back(0);
                best_hit.push_back(RANGED_HIT_NULL);
                entity_index = entities_hit.size() - 1;
              }

              Damage_set dam = attack.roll_damage(hit_type);

              total_damage[entity_index] += dam.get_damage(DAMAGE_PIERCE);
              if (hit_type > best_hit[entity_index]) {
                best_hit[entity_index] = hit_type;
              }
// Outstanding bug!  Sometimes ranged attacks inexplicably do 0 damage.  I'm
// leaving this in to help track it down, maybe.
              if (dam.get_damage(DAMAGE_PIERCE) == 0 && TESTING_MODE) {
                debugmsg("0 ranged damage!");
                debugmsg("Attack damage: %d %d %d",
                         attack.damage[DAMAGE_BASH],
                         attack.damage[DAMAGE_CUT],
                         attack.damage[DAMAGE_PIERCE]);
              }
              stopped = true;
            } else if (i == path.size() - 1 && shooter == player) {
              add_msg("<c=dkgray>%s barely %s %s.<c=/>",
                      shooter_name.c_str(), miss_verb.c_str(),
                      entity_hit->get_name_to_player().c_str());
            }
          } // if (entity hit)
        } // End of <Didn't hit solid terrain>
        i++;  // Increment which tile in the trajectory we're examining
      } // while (!stopped && i < path.size())

      Tripoint end_point;
      if (i == path.size()) {
        end_point = Tripoint(path.back().x, path.back().y, 0);
      } else {
        end_point = Tripoint(path[i].x, path[i].y, 0);
      }
// Drop the projectile we threw, if it's "real" and we're throwing it
      if (it.is_real() && attack.type == RANGED_ATT_THROW) {
        map->add_item(it, end_point);
      }
// Create the target_field from our attack, if it's "real"
      if (attack.target_field.exists()) {
        attack.target_field.drop(end_point, shooter_name);
      }
// Now, display messages for all entities hit.
// Sanity check for vector sizes!
      if (entities_hit.size() != total_damage.size() ||
          entities_hit.size() != best_hit.size()) {
        debugmsg("entities_hit %d, total_damage %d, best_hit %d",
                 entities_hit.size(), total_damage.size(), best_hit.size());
      } else {
        for (int i = 0; i < entities_hit.size(); i++) {
          Entity* entity_hit = entities_hit[i];
          Ranged_hit_type hit_type = best_hit[i];
          int dam = total_damage[i];
          if (hit_type == RANGED_HIT_HEADSHOT) {
            shooter_name = capitalize(shooter_name);
            add_msg("<c=ltred>Headshot!  %s %s %s for %d damage!<c=/>",
                    shooter_name.c_str(), verb.c_str(),
                    entity_hit->get_name_to_player().c_str(),
                    dam);
          } else if (hit_type == RANGED_HIT_CRITICAL) {
            shooter_name = capitalize(shooter_name);
            add_msg("<c=ltred>Critical!  %s %s %s for %d damage!<c=/>",
                    shooter_name.c_str(), verb.c_str(),
                    entity_hit->get_name_to_player().c_str(),
                    dam);
          } else if (hit_type == RANGED_HIT_GRAZE) {
            add_msg("<c=ltred>%s %s %s for %d damage!<c=/>",
                    shooter_name.c_str(), graze_verb.c_str(),
                    entity_hit->get_name_to_player().c_str(),
                    dam);
          } else {
            add_msg("<c=ltred>%s %s %s for %d damage!<c=/>",
                    shooter_name.c_str(), verb.c_str(),
                    entity_hit->get_name_to_player().c_str(),
                    dam);
          }
// Figure out the part hit, in case it's the player or NPC
          Body_part part_hit;
          if (hit_type == RANGED_HIT_HEADSHOT) {
            part_hit = random_head_part();
          } else if (hit_type == RANGED_HIT_CRITICAL) {
            part_hit = BODY_PART_TORSO;
          } else if (hit_type == RANGED_HIT_GRAZE || one_in(3)) {
            part_hit = random_extremity();
          } else {
            part_hit = BODY_PART_TORSO;
          }
            
          entity_hit->take_damage(DAMAGE_PIERCE, dam, shooter_name, part_hit);
        } //for (int i = 0; i < entities_hit.size(); i++)
      } // Sanity check passed
    } // for (int pellet = 0; pellet < attack.pellets; pellet++)
// Now we have to move target to the average of all the tiles hit by pellets
    int new_x = 0, new_y = 0;
    for (int i = 0; i < tiles_hit.size(); i++) {
      new_x += tiles_hit[i].x;
      new_y += tiles_hit[i].y;
    }
// We want to round randomly - not always round down.
    int remainder_x = new_x % tiles_hit.size(),
        remainder_y = new_y % tiles_hit.size();
    new_x /= tiles_hit.size();
    new_y /= tiles_hit.size();
    if (rng(1, tiles_hit.size()) <= remainder_x) {
      new_x--;
    }
    if (rng(1, tiles_hit.size()) <= remainder_y) {
      new_y--;
    }
/* Finally, if we were originally targeting an entity, and we are no longer
 * pointed at an entity, we may have a chance to retarget.
 */
    if (targeting_entity && retarget_range > 0 &&
        entities.entity_at(new_x, new_y, target.z)) {
      std::vector<Tripoint> new_targets;
      for (int ntx = new_x - retarget_range; ntx <= new_x + retarget_range;
           ntx++) {
        for (int nty = new_y - retarget_range; nty <= new_y + retarget_range;
             nty++) {
          Entity* new_target = entities.entity_at(ntx, nty, target.z);
// TODO: Ensure that the new target isn't friendly!
          if (new_target) {
            new_targets.push_back( Tripoint(ntx, nty, target.z) );
          }
        }
      }
      if (!new_targets.empty()) {
        int nt_index = rng(0, new_targets.size() - 1);
        new_x = new_targets[nt_index].x;
        new_y = new_targets[nt_index].y;
      }
    }
    target.x = new_x;
    target.y = new_y;
        
  } // for (int round = 0; round < attack.rounds; round++)
}

void Game::player_move(int xdif, int ydif)
{
// TODO: Remove this?
  if (xdif < -1 || xdif > 1 || ydif < -1 || ydif > 1) {
    debugmsg("Game::player_move called with [%d, %d]", xdif, ydif);
    return;
  }

  int newx = player->pos.x + xdif, newy = player->pos.y + ydif;
  Entity* ent = entities.entity_at(newx, newy, player->pos.z);
  std::string tername = map->get_name(newx, newy, player->pos.z);

// If we bump an entity, attack it.
// TODO: If that entity is a friendly NPC, talk instead!
  if (ent) {
    player->attack(ent);

// Check that we can drag our furniture (if any)
  } else if (!player->can_drag_furniture_to(map, newx, newy)) {
    add_msg("The %s you're dragging prevents you from moving there.",
            player->get_dragged_name().c_str());
    add_msg("Press (<c=yellow>%s<c=/>) to drop it.",
            KEYBINDINGS.describe_bindings_for(IACTION_GRAB).c_str());
    return;

// If it's open space, we'll fall!
  } else if (map->has_flag(TF_OPEN_SPACE, newx, newy)) {
    int levels_to_fall = 0;
    int fall_z = player->pos.z;
    while (map->has_flag(TF_OPEN_SPACE, newx, newy, fall_z)) {
      levels_to_fall++;
      fall_z--;
    }
    if (!msg_query_yn("Step out into open space and fall %d floors?",
                      levels_to_fall)) {
      return;
    }
    debugmsg("Fall");
    player->move_to(map, newx, newy);
    player_move_vertical(0 - levels_to_fall);
    player->fall(levels_to_fall); // This handles damage, etc.

// When we fall hit land hard; getting ready for our next turn takes time
    int move_penalty = (120 - 2 * player->stats.dexterity);
// Low dexterity means we're really bad at recovering from a fall
    if (player->stats.dexterity < 10) {
      move_penalty += 5 * (10 - player->stats.dexterity);
    }
    move_penalty -= 3 * player->skills.get_level(SKILL_DODGE);
    player->use_ap(100 + levels_to_fall * move_penalty);
    
// If we can move there... move there!
  } else if (player->can_move_to(map, newx, newy)) {
    player->move_to(map, newx, newy);

// Otherwise, try to open it?
  } else if (map->apply_signal("open", newx, newy, player->pos.z, player)) {
    player->use_ap(100);
    return; // Don't list items
  }
// List items here, unless it's sealed.
  if (map->has_flag(TF_SEALED, player->pos)) {
    add_msg("<c=dkgray>This %s is sealed; there may be items inside, but you \
cannot see or access them.<c=/>", map->get_name(player->pos).c_str());
  } else {
    std::vector<Item> *items = map->items_at(player->pos);
// TODO: Ensure the player has the sense of sight
    if (!items->empty()) {
      std::string item_message = "You see here " + list_items(items);
      add_msg( item_message );
    }
  }
}

void Game::player_move_vertical(int zdif)
{
// FRODO: Move entities into a stairs-following queue
//        (except not, since we're on a 3D map nowadays)
  map->shift(worldmap, 0, 0, zdif);
  player->pos.z += zdif;
}

void Game::add_msg(std::string msg, ...)
{
  if (msg.empty()) {
    return;
  }
  char buff[2048];
  va_list ap;
  va_start(ap, msg);
  vsprintf(buff, msg.c_str(), ap);
  va_end(ap);
  std::string text(buff);
  text = capitalize(text);
  if (!messages.empty() && messages.back().text == text &&
      time.get_turn() - messages.back().turn <= MESSAGE_GAP) {
    messages.back().count++;
    messages.back().turn = time.get_turn();
    return;
  }
  messages.push_back( Game_message(text, time.get_turn()) );
  new_messages++;
}

bool Game::msg_query_yn(std::string msg, ...)
{
// This duplicates all the code of add_msg(), but there's no other option!
  if (msg.empty()) {
    return true;
  }
  char buff[2048];
  va_list ap;
  va_start(ap, msg);
  vsprintf(buff, msg.c_str(), ap);
  va_end(ap);
  std::string text(buff);
  text = capitalize(text);
// Add the color highlighting that lets the player know it's a prompt
  std::stringstream colorized;
  colorized << "<c=ltgreen>" << text << "<c=/>";
  text = colorized.str();
  if (!messages.empty() && messages.back().text == text &&
      time.get_turn() - messages.back().turn <= MESSAGE_GAP) {
    messages.back().count++;
    messages.back().turn = time.get_turn();
  } else {
    messages.push_back( Game_message(text, time.get_turn()) );
    new_messages++;
  }
  draw_all();
  long ch = input();
  if (ch != 'Y' && ch != 'N') {
    add_msg("<c=ltred>Y<c=ltgreen> or <c=ltred>N<c=ltgreen> only, please. \
Case-sensitive.");
  }
  while (ch != 'Y' && ch != 'N') {
    ch = input();
  }
  return (ch == 'Y');
}

void Game::add_active_item(Item* it)
{
  if (!it) {
    return;
  }
  active_items.push_back(it);
}

void Game::remove_active_item(Item* it)
{
  if (!it) {
    return;
  }
  for (int i = 0; i < active_items.size(); i++) {
    if (active_items[i] == it) {
      active_items.erase( active_items.begin() + i );
      return;
    }
  }
}

void Game::remove_active_item_uid(int uid)
{
  for (int i = 0; i < active_items.size(); i++) {
    if (active_items[i]->get_uid() == uid) {
      active_items.erase( active_items.begin() + i );
      return;
    }
  }
}

// Bit of a duplication of code from find_item(), but what can ya do
bool Game::destroy_item(Item* it, int uid)
{
// Sanity check
  if (it == NULL && (uid < 0 || uid >= next_item_uid)) {
    return false;
  }
// Check entities first - almost certainly faster than the map
  for (std::list<Entity*>::iterator iter = entities.instances.begin();
       iter != entities.instances.end();
       iter++) {
    Item check = (*iter)->remove_item(it, uid);
    if (check.is_real()) {
      return true;
    }
  }
  return map->remove_item(it, uid);
}

bool Game::destroy_item_uid(int uid)
{
  return destroy_item(NULL, uid);
}

void Game::set_temp_light_level(int level)
{
  if (level > temp_light_level) {
    temp_light_level = level;
  }
}

void Game::draw_all()
{
  update_hud();
  int range = player->sight_range( get_light_level() );
  map->draw(w_map, &entities, player->pos, range);
  w_map->refresh();
}
  
void Game::update_hud()
{
  print_messages();
// Update date
  i_hud.set_data("text_date", time.get_text());
// Update location description
  Submap* sm = map->get_center_submap();
  if (sm) {
    i_hud.set_data("text_location", sm->get_world_ter_name());
  }
// Update speed
  i_hud.set_data("num_speed", player->get_speed());
// Colorize speed
  i_hud.set_data("num_speed", player->get_speed_color());
// Add any player ailments
  std::string status = player->get_all_status_text();
  //std::string status = player->get_hunger_text() + " " + player->get_thirst_text();
  i_hud.set_data("text_status", status);
// Draw minimap
  cuss::element* minimap = i_hud.find_by_name("draw_minimap");
  if (minimap) {
    int cornerx = map->posx - minimap->sizex / 2 + MAP_SIZE / 2;
    int cornery = map->posy - minimap->sizey / 2 + MAP_SIZE / 2;
    worldmap->draw_minimap(minimap, cornerx, cornery);
  }
  if (player->weapon.is_real()) {
    std::stringstream weapon_ss;
    weapon_ss << "<c=white>" << player->weapon.get_name_full() << "<c=/>";
    i_hud.set_data("text_weapon", weapon_ss.str());
  } else {
    i_hud.set_data("text_weapon", "None");
  }
  i_hud.set_data("hp_head",  player->hp_text(HP_PART_HEAD     ) );
  i_hud.set_data("hp_torso", player->hp_text(HP_PART_TORSO    ) );
  i_hud.set_data("hp_l_arm", player->hp_text(HP_PART_LEFT_ARM ) );
  i_hud.set_data("hp_r_arm", player->hp_text(HP_PART_RIGHT_ARM) );
  i_hud.set_data("hp_l_leg", player->hp_text(HP_PART_LEFT_LEG ) );
  i_hud.set_data("hp_r_leg", player->hp_text(HP_PART_RIGHT_LEG) );
  
  i_hud.draw(w_hud);
  w_hud->refresh();
}

void Game::print_messages()
{
  i_hud.clear_data("text_messages");
  int sizey;
  cuss::element *message_box = i_hud.find_by_name("text_messages");
  if (!message_box) {
    debugmsg("Couldn't find text_messages in i_hud!");
    return;
  }
  sizey = message_box->sizey;
  int start = messages.size() - sizey;
  if (start < 0) {
    start = 0;
  }
  for (int i = start; i < messages.size(); i++) {
    std::stringstream text;
    int index = messages.size() - new_messages + i;
    text << messages[index].text;
    if (messages[index].count > 1) {
      text << " x" << messages[index].count;
    }
    text << '\n';
    //debugmsg("Adding %s", text.str().c_str());
    i_hud.add_data("text_messages", text.str());
  }
}

void Game::debug_command()
{
  add_msg("<c=yellow>Press debug key.<c=/>");
  draw_all();
  long ch = input();
  Debug_action act = KEYBINDINGS.bound_to_debug_key(ch);

  switch (act) {
    case DEBUG_ACTION_NULL:
      add_msg("<c=dkgray>Never mind.<c=/>");
      break;

    case DEBUG_ACTION_CREATE_ITEM: {
      std::string name = string_input_popup("Item name (partial names OK):");
      Item_type* type = ITEM_TYPES.lookup_partial_name(name);
      if (!type) {
        add_msg("<c=dkgray>'%s' did not match any items.<c=/>", name.c_str());
      } else {
        Item spawned(type);
        map->add_item(spawned, player->pos);
        add_msg("Spawned %s.", spawned.get_name_indefinite().c_str());
      }
    } break;

    case DEBUG_ACTION_BUILD_MAP: {
      std::string name = string_input_popup("Map layout name:");
      Mapgen_spec* spec = MAPGEN_SPECS.lookup_name(name);
      if (spec) {
        Submap* sm = map->get_testing_submap();
        if (sm) {
          spec->prepare();
          sm->clear_items();
          sm->generate(spec);
        } else {
          add_msg("Map::get_testing_submap() return NULL???");
        }
      } else {
        add_msg("<c=dkgray>Layout '%s' does not exist.", name.c_str());
      }
    } break;

    case DEBUG_ACTION_MAP_INFO:
      add_msg(map->get_center_submap()->get_spec_name().c_str());
      break;

    case DEBUG_ACTION_PLACE_FIELD: {
      std::string name = string_input_popup("Field type (partial name OK):");
      Field_type* type = FIELDS.lookup_partial_name(name);
      if (!type) {
        add_msg("<c=dkgray>'%s' did not match any fields.<c=/>", name.c_str());
      } else {
        Tripoint pos = target_selector();
        map->add_field(type, pos, "Magic");
      }
    } break;

    case DEBUG_ACTION_CLEAR_ITEMS:
      map->clear_items();
      add_msg("Items cleared.  Note; this may cause a crash if there were \
active items!");
      break;

    case DEBUG_ACTION_SPAWN_MONSTER: {
      std::string name = string_input_popup("Monster name (partial names OK):");
      Monster_type* type = MONSTER_TYPES.lookup_partial_name(name);
      if (!type) {
        add_msg("<c=dkgray>'%s' did not match any monsters.<c=/>",
                name.c_str());
      } else {
        Monster* mon = new Monster(type);
        mon->pos = target_selector();
        entities.add_entity(mon);
      }
    } break;

    case DEBUG_ACTION_MONSTER_PATH:
      if (!entities.empty()) {
        Entity* ent = *(entities.instances.begin());
        Monster* mon = static_cast<Monster*>(ent);
        mon->make_plans();
        std::stringstream path_info;
        path_info << "Monster: " << mon->pos.str() << std::endl;
        path_info << "You: " << player->pos.str() << std::endl;
        std::vector<Tripoint> path = mon->plan.path.get_points();
        path_info << "Path: (" << mon->plan.path.get_points().size() << ") " <<
                     std::endl;
        for (int i = 0; i < path.size(); i++) {
          path_info << path[i].str() << " => ";
        }
        popup(path_info.str().c_str());
      }
      break;

    case DEBUG_ACTION_MEMORY_INFO:
      popup("Submaps: %d\nSizeof(Submap): %d", SUBMAP_POOL.size(),
            sizeof(Submap));
      break;

    default:
      add_msg("Nothing coded for %s yet.", debug_action_name(act).c_str());
  }
}

void Game::pickup_items(Tripoint pos)
{
  pickup_items(pos.x, pos.y);
}

void Game::pickup_items(Point pos)
{
  pickup_items(pos.x, pos.y);
}

void Game::pickup_items(int posx, int posy)
{
  if (map->has_flag(TF_SEALED, posx, posy)) {
    add_msg("<c=ltred>That %s is sealed; you cannot retrieve items there.<c=/>",
            map->get_name(posx, posy).c_str());
    return;
  }
  if (!w_hud) {
    debugmsg("pickup_items() - w_hud is NULL!");
    return;
  }

  cuss::interface i_pickup;
  if (!i_pickup.load_from_file(CUSS_DIR + "/i_pickup.cuss")) {
    return;
  }

  int weight_after = player->current_weight(),
      volume_after = player->current_volume();
  std::vector<Item> *available = map->items_at(posx, posy);
  std::vector<bool> pick_up;
  std::vector<std::string> pickup_strings, weight_strings, volume_strings;
  for (int i = 0; i < available->size(); i++) {
    pick_up.push_back(false);
    weight_strings.push_back( itos( (*available)[i].get_weight() ) );
    volume_strings.push_back( itos( (*available)[i].get_volume() ) );
  }

  int offset = 0;
  cuss::element *ele_list_items = i_pickup.find_by_name("list_items");
  if (!ele_list_items) {
    debugmsg("No element 'list_items' in i_pickup.cuss");
    return;
  }
  int offset_size = ele_list_items->sizey;
  bool done = false;

  pickup_strings =  get_pickup_strings(available, &pick_up);
// Set interface data
  i_pickup.set_data("weight_current", player->current_weight());
  i_pickup.set_data("weight_maximum", player->maximum_weight());
  i_pickup.set_data("volume_current", player->current_volume());
  i_pickup.set_data("volume_maximum", player->maximum_volume());
  for (int i = 0; i < offset_size && i < pickup_strings.size(); i++) {
    i_pickup.add_data("list_items",  pickup_strings[i]);
    i_pickup.add_data("list_weight", weight_strings[i]);
    i_pickup.add_data("list_volume", volume_strings[i]);
  }

  while (!done) {
    i_pickup.set_data("weight_after",   weight_after);
    i_pickup.set_data("volume_after",   volume_after);
    i_pickup.draw(w_hud);
    long ch = getch();
    if (ch >= 'A' && ch <= 'Z') {
      ch = ch - 'A' + 'a'; // Convert uppercase letters to lowercase
    }
    if (ch >= 'a' && ch - 'a' < available->size()) {
      int index = ch - 'a';
      pick_up[index] = !pick_up[index];
      bool pu = pick_up[index];
      Item *it = &( (*available)[index] );
      pickup_strings[index] = pickup_string(it, ch, pu);
      i_pickup.set_data("list_items", pickup_strings);
      if (pu) {
        weight_after += it->get_weight();
        volume_after += it->get_volume();
      } else {
        weight_after -= it->get_weight();
        volume_after -= it->get_volume();
      }
      i_pickup.clear_data("list_items" );
      i_pickup.clear_data("list_weight");
      i_pickup.clear_data("list_volume");
      std::stringstream weight_ss, volume_ss;
      weight_ss << (pu ? "<c=green>" : "<c=ltgray>") << it->get_weight() <<
                   "<c=/>";
      volume_ss << (pu ? "<c=green>" : "<c=ltgray>") << it->get_volume() <<
                   "<c=/>";
      weight_strings[index] = weight_ss.str();
      volume_strings[index] = volume_ss.str();
      for (int i = offset * offset_size;
           i < (offset + 1) * offset_size && i < pickup_strings.size();
           i++) {
        i_pickup.add_data("list_items",  pickup_strings[i]);
        i_pickup.add_data("list_weight", weight_strings[i]);
        i_pickup.add_data("list_volume", volume_strings[i]);
      }
    } else if (ch == '<' && offset > 0) {
      offset--;
      i_pickup.clear_data("list_items" );
      i_pickup.clear_data("list_weight");
      i_pickup.clear_data("list_volume");
      for (int i = offset * offset_size;
           i < (offset + 1) * offset_size && i < pickup_strings.size();
           i++) {
        i_pickup.add_data("list_items",  pickup_strings[i]);
        i_pickup.add_data("list_weight", weight_strings[i]);
        i_pickup.add_data("list_volume", volume_strings[i]);
      }
    } else if (ch == '>' && available->size() > (offset + 1) * offset_size) {
      offset++;
      i_pickup.clear_data("list_items" );
      i_pickup.clear_data("list_weight");
      i_pickup.clear_data("list_volume");
      for (int i = offset * offset_size;
           i < (offset + 1) * offset_size && i < pickup_strings.size();
           i++) {
        i_pickup.add_data("list_items",  pickup_strings[i]);
        i_pickup.add_data("list_weight", weight_strings[i]);
        i_pickup.add_data("list_volume", volume_strings[i]);
      }
    } else if (ch == KEY_ESC) {
      return;
    } else if (ch == '\n') {
      done = true;
    }
  }
// TODO: Code for multi-turn pickup
  std::vector<Item> items_gotten;
  for (int i = 0; i < available->size(); i++) {
    if (pick_up[i]) {
      items_gotten.push_back( (*available)[i] );
      if (player->add_item( (*available)[i] )) {
        available->erase(available->begin() + i);
        pick_up.erase(pick_up.begin() + i);
      }
    }
  }

  std::string message = "You pick up " + list_items(&items_gotten);
  add_msg(message);
  
}

Tripoint Game::target_selector(int startx, int starty, int range,
                               bool target_entities, bool show_path)
{
  std::vector<Tripoint> path = path_selector(startx, starty, range,
                                             target_entities, show_path);
  if (path.empty()) {
    return Tripoint(-1, -1, -1);
  }
  return path.back();
}

std::vector<Tripoint> Game::path_selector(int startx, int starty, int range,
                                          bool target_entities, bool show_path)
{
  std::vector<Tripoint> ret;
  if (!player) {
    return ret;
  }
  if (startx == -1 || starty == -1) {
    startx = player->pos.x;
    starty = player->pos.y;
  }

  int minx, miny, maxx, maxy;
  if (range == -1) {  // Range defaults to -1, "no limit"
    range = get_light_level();
  }
  if (range > get_light_level()) {
    range = get_light_level();
  }
  minx = startx - range;
  miny = starty - range;
  maxx = startx + range;
  maxy = starty + range;

  Tripoint target(startx, starty, player->pos.z);;
  if (target_entities) {
    if (last_target == -1) {  // No previous target to snap to, pick the closest
      Entity* new_target = entities.closest_seen_by(player, range);
      if (new_target) { // It'll be NULL if no one is in range
        target = new_target->pos;
      }
    } else {
      Entity* old_target = entities.lookup_uid(last_target);
// It'll be NULL if the old target's dead, etc.
      if (old_target &&
          map->senses(player->pos, old_target->pos, range, SENSE_SIGHT)) {
        target = old_target->pos;
      } else {
// Reset last_target
        last_target = -1;
        Entity* new_target = entities.closest_seen_by(player, range);
        if (new_target) { // It'll be NULL if no one is in range
          target = new_target->pos;
        }
      }
    }
  }

// First draw; we need to draw the path since we might auto-target
  ret = map->line_of_sight(player->pos, target);
  map->draw_area(w_map, &entities, player->pos, minx, miny, maxx, maxy);
  if (show_path) {
    for (int i = 0; i < ret.size(); i++) {
      map->draw_tile(w_map, &entities, ret[i].x, ret[i].y,
                     player->pos.x, player->pos.y, true); // true==inverted
    }
  } else {
// Just do the last tile
    map->draw_tile(w_map, &entities, ret.back().x, ret.back().y,
                   player->pos.x, player->pos.y, true);
  }

// TODO: No no no remove this!  Won't work for tiles!
  Entity* ent_targeted = entities.entity_at(target);
  if (ent_targeted) {
    w_map->putglyph(w_map->sizex() / 2 - player->pos.x + target.x,
                    w_map->sizey() / 2 - player->pos.y + target.y,
                    ent_targeted->get_glyph().invert());
  } else {
    w_map->putglyph(w_map->sizex() / 2 - player->pos.x + target.x,
                    w_map->sizey() / 2 - player->pos.y + target.y,
                    glyph('*', c_red, c_black));
  }
  w_map->refresh();

  while (true) {
    long ch = input();
    if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
      return std::vector<Tripoint>();
    } else if (ch == '\n') {
      return ret;
    } else {
      Point p = input_direction(ch);
      if (p.x == 0 && p.y == 0) {
        return ret; // Return our path on hitting "pause"
      } else if (p.x != -2 && p.y != -2) {
        target += p;
// Ensure we're still in-range
        if (target.x < minx) {
          target.x = minx;
        }
        if (target.y < miny) {
          target.y = miny;
        }
        if (target.x > maxx) {
          target.x = maxx;
        }
        if (target.y > maxy) {
          target.y = maxy;
        }
        ret = map->line_of_sight(player->pos, target);
        map->draw_area(w_map, &entities, player->pos, minx, miny, maxx, maxy);
        if (show_path) {
          for (int i = 0; i < ret.size(); i++) {
            map->draw_tile(w_map, &entities, ret[i].x, ret[i].y,
                           player->pos.x, player->pos.y, true);// true==inverted
          }
        }
// TODO: No no no remove this!  Won't work for tiles!
        ent_targeted = entities.entity_at(target);
        if (ent_targeted) {
          w_map->putglyph(w_map->sizex() / 2 - player->pos.x + target.x,
                          w_map->sizey() / 2 - player->pos.y + target.y,
                          ent_targeted->get_glyph().invert());
        } else {
          w_map->putglyph(w_map->sizex() / 2 - player->pos.x + target.x,
                          w_map->sizey() / 2 - player->pos.y + target.y,
                          glyph('*', c_red, c_black));
        }
        w_map->refresh();
      }
    }
  }
}

int Game::get_item_uid()
{
  return next_item_uid++;
}

int Game::get_furniture_uid()
{
  return next_furniture_uid++;
}

bool Game::minute_timer(int minutes)
{
  if (minutes <= 0) {
    return true;
  }
  int turns = (minutes * 60) / SECONDS_IN_TURN;
  return turn_timer(turns);
}

bool Game::turn_timer(int turns)
{
  if (turns <= 0) {
    return true;
  }
  return (time.get_turn() % turns == 0);
}

int Game::get_light_level()
{
  int ret = time.get_light_level();
  if (temp_light_level > ret) {
    return temp_light_level;
  }
  return ret;
}

// UID defaults to -1
Tripoint Game::find_item(Item* it, int uid)
{
// Sanity check
  if (it == NULL && (uid < 0 || uid >= next_item_uid)) {
    return Tripoint(-1, -1, -1);
  }
// Check entities first - almost certainly faster than the map
  for (std::list<Entity*>::iterator iter = entities.instances.begin();
       iter != entities.instances.end();
       iter++) {
    if ( (*iter)->has_item(it, uid)) {
      return (*iter)->pos;
    }
  }
  Tripoint ret = map->find_item(it, uid);
  return ret;
}

Tripoint Game::find_item_uid(int uid)
{
  return find_item(NULL, uid);
}

std::vector<std::string>
get_pickup_strings(std::vector<Item> *items, std::vector<bool> *picking_up)
{
  std::vector<std::string> ret;
  if (items->size() != picking_up->size()) {
    debugmsg("get_pickup_strings() - vectors aren't same size!");
    return ret;
  }

  for (int i = 0; i < items->size(); i++) {
    bool pickup = (*picking_up)[i];
    ret.push_back( pickup_string( &( (*items)[i] ), char('a' + i), pickup ) );
  }
  return ret;
}

std::string pickup_string(Item *item, char letter, bool picking_up)
{
  if (!item) {
    return "<c=red>NO ITEM<c=/>";
  }
  std::stringstream ss;
  ss << "<c=" << (picking_up ? "green" : "ltgray") << ">" << letter <<
        (picking_up ? " +" : " -") << " " << item->get_name_full();
  return ss.str();
}
