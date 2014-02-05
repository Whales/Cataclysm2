#include "game.h"
#include "window.h"
#include "stringfunc.h"
#include "map.h"
#include "rng.h"
#include "pathfind.h"
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
  new_messages = 0;
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
  
bool Game::setup()
{
  if (!i_hud.load_from_file("cuss/i_hud.cuss")) {
    debugmsg("Can't load cuss/i_hud.cuss!");
    return false;
  }
  int xdim, ydim;
  get_screen_dims(xdim, ydim);
  int win_size = ydim;
  if (win_size % 2 == 0) {
    win_size--; // Only odd numbers allowed!
  }
  w_map = new Window(0, 0, win_size, win_size);
  w_hud = new Window(win_size, 0, 55, ydim);
// Attempt to resize the messages box to be as tall as the window allows
  cuss::element* messages = i_hud.select("text_messages");
  if (!messages) {
    debugmsg("Couldn't find element text_messages in i_hud");
    return false;
  }
  messages->sizey = ydim - messages->posy;

  worldmap = new Worldmap;
  worldmap->generate();

  map = new Map;
  Point start = worldmap->random_tile_with_terrain("beach");
  map->generate(worldmap, start.x, start.y, 0);


  player = new Player;
  entities.add_entity(player);

  game_over = false;
  new_messages = 0;
  return true;
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
  player->gain_action_points();
  while (player->action_points > 0) {
    handle_player_activity();
    shift_if_needed();
    update_hud();
    map->draw(w_map, &entities, player->pos);
    w_map->refresh();

    if (!player->activity.is_active()) {
      long ch = input();
      if (ch == '!') {
/*
        Monster* mon = new Monster;
        mon->set_type("zombie");
        mon->pos.x = player->pos.x - 3;
        mon->pos.y = player->pos.y - 3;
        entities.add_entity(mon);
*/
        map->add_field( FIELDS.lookup_name("fire"), player->pos.x - 3, player->pos.y - 3, player->pos.z, "magic" );
      }
      Interface_action act = KEYBINDINGS.bound_to_key(ch);
      do_action(act);
    }
  }
  map->process_fields();
  shift_if_needed();
  move_entities();
  if (game_over) {
    return false;
  }
  return true;
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
        player_move_vertical(1);
      } else {
        player_move_vertical(1);
      }
      break;

    case IACTION_MOVE_DOWN:
      if (!map->has_flag(TF_STAIRS_DOWN, player->pos)) {
        add_msg("You cannot go down here.");
        player_move_vertical(-1);
      } else {
        player_move_vertical(-1);
      }
      break;

    case IACTION_PICK_UP:
// TODO: Interface for picking up >1 item
      if (map->item_count(player->pos) == 0) {
        add_msg("No items here.");
      } else if (map->item_count(player->pos) == 1) {
        std::vector<Item> *items = map->items_at(player->pos);
        std::string message = "You pick up " + list_items(items);
        player->add_item( (*items)[0] );
        items->clear();
        add_msg(message);
      } else {
        pickup_items(player->pos);
      }
      break;

    case IACTION_OPEN: {
      Point dir = input_direction(input());
      if (dir.x == -2) { // Error
        add_msg("Invalid direction.");
      } else {
        Tripoint open = player->pos + dir;
        std::string tername = map->get_name(open);
        if (map->open(open)) {
          add_msg("You open the %s.", tername.c_str());
          player->use_ap(100);
        } else {
          add_msg("You cannot open a %s.", tername.c_str());
        }
      }
    } break;

    case IACTION_CLOSE: {
      Point dir = input_direction(input());
      if (dir.x == -2) { // Error
        add_msg("Invalid direction.");
      } else {
        Tripoint close = player->pos + dir;
        std::string tername = map->get_name(close);
        if (map->close(close)) {
          add_msg("You close the %s.", tername.c_str());
          player->use_ap(100);
        } else {
          add_msg("You cannot close a %s.", tername.c_str());
        }
      }
    } break;

    case IACTION_SMASH: {
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

    case IACTION_INVENTORY: {
      Item it = player->inventory_single();
      Item_action act = it.show_info();
      if (act == IACT_DROP) {
        add_msg( player->drop_item_message(it) );
        player->remove_item_uid(it.get_uid());
        map->add_item(it, player->pos);
      } else if (act == IACT_WIELD) {
        add_msg( player->wield_item_message(it) );
        player->wield_item_uid(it.get_uid());
      } else if (act == IACT_WEAR) {
        add_msg( player->wear_item_message(it) );
        player->wear_item_uid(it.get_uid());
      }
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
        Point target = target_selector();
        if (target.x == -1) { // We canceled
          add_msg("Never mind.");
        } else {
          player->remove_item_uid(it.get_uid(), 1);
          Ranged_attack att = player->throw_item(it);
          launch_projectile(it, att, player->pos, target);
        }
      }
    } break;

    case IACTION_FIRE:
      if (!player->weapon.is_real()) {
        add_msg("You are not wielding anything.");
      } else if (player->weapon.get_item_class() != ITEM_CLASS_LAUNCHER) {
        add_msg("You cannot fire %s.",
                player->weapon.get_name_indefinite().c_str());
      } else if (player->weapon.charges == 0 || !player->weapon.ammo) {
        add_msg("You need to reload %s.",
                player->weapon.get_name_definite().c_str());
      } else {
        Point target = target_selector();
        if (target.x == -1) { // We canceled
          add_msg("Never mind.");
        } else {
          Ranged_attack att = player->fire_weapon();
          launch_projectile(Item(), att, player->pos, target);
        }
      }
      break;

    case IACTION_MESSAGES_SCROLL_BACK:
      i_hud.add_data("text_messages", -1);
      break;
    case IACTION_MESSAGES_SCROLL_FORWARD:
      i_hud.add_data("text_messages",  1);
      break;

    case IACTION_VIEW_WORLDMAP: {
      Point p = map->get_center_point();
      worldmap->draw(p.x, p.y);
    }  break;

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
// TODO: Only update scent_map when the player moves (i.e., move this call to
//       player_move()?)
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

void Game::launch_projectile(Item it, Ranged_attack attack,
                             Point origin, Point target)
{
  int range = rl_dist(origin, target);
  int angle_missed_by = attack.roll_variance();
// Use 1800 since attack.variance is measured in 10ths of a degree
  double distance_missed_by = range * tan(angle_missed_by * PI / 1800);
  int tiles_off = int(distance_missed_by);
  if (tiles_off >= 1) {
    target.x += rng(0 - tiles_off, tiles_off);
    target.y += rng(0 - tiles_off, tiles_off);
  }
// fine_distance is used later to see if we hit the target or "barely missed"
  int fine_distance = 100 * (distance_missed_by - tiles_off);
  debugmsg("angle %d, missed %f, tiles %d, fine %d", angle_missed_by, distance_missed_by, tiles_off, fine_distance);

  std::vector<Point> path = map->line_of_sight(origin, target);
  if (path.empty()) { // Lost line of sight at some point
    path = line_to(origin, target);
  }

  for (int i = 0; i < path.size(); i++) {
    if (map->move_cost(path[i].x, path[i].y) == 0) {
// It's a solid tile, so let's try to smash through it!
      map->smash(path[i].x, path[i].y, attack.roll_damage(), false);
      if (map->move_cost(path[i].x, path[i].y) == 0) {
        return; // We didn't make it!
      }
    } else {
      Entity* entity_hit = entities.entity_at(path[i].x, path[i].y);
      if (entity_hit) {
        bool hit;
// TODO: Incorporate the size of the monster
        if (i == path.size() - 1) {
          hit = rng(0, 100) >= fine_distance;
        } else {
          hit = one_in(3);
        }
        if (hit) {
          add_msg("You shoot %s!", entity_hit->get_name_to_player().c_str());
          Damage_set dam = attack.roll_damage();
          entity_hit->take_damage(DAMAGE_PIERCE, dam.get_damage(DAMAGE_PIERCE),
                                  "you");
          return;
        } else if (i == path.size() - 1) {
          add_msg("You barely miss %s.",
                  entity_hit->get_name_to_player().c_str());
        }
      }
    }
  }
}

void Game::player_move(int xdif, int ydif)
{
// TODO: Remove this?
  if (xdif < -1 || xdif > 1 || ydif < -1 || ydif > 1) {
    debugmsg("Game::player_move called with [%d, %d]", xdif, ydif);
    return;
  }

  int newx = player->pos.x + xdif, newy = player->pos.y + ydif;
  Entity* ent = entities.entity_at(newx, newy);
  std::string tername = map->get_name(newx, newy);
  if (ent) {
    player->attack(ent);
  } else if (player->can_move_to(map, newx, newy)) {
    player->move_to(map, newx, newy);
  } else if (map->open(newx, newy)) {
    add_msg("You open the %s.", tername.c_str());
    player->use_ap(100);
  }
  std::vector<Item> *items = map->items_at(player->pos);
// TODO: Ensure the player has the sense of sight
  if (!items->empty()) {
    std::string item_message = "You see here " + list_items(items);
    add_msg( item_message );
  }
}

void Game::player_move_vertical(int zdif)
{
// TODO: Move entities into a stairs-following queue
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
  if (text[0] >= 'a' && text[0] <= 'z') {
// Capitalize!
    text[0] += 'A' - 'a';
  }
// TODO: Check if turn gap is small enough.
  if (!messages.empty() && messages.back().text == text) {
    messages.back().count++;
    return;
  }
  messages.push_back( Game_message(text) );
  new_messages++;
}

void Game::update_hud()
{
  print_messages();
// Draw minimap
  cuss::element* minimap = i_hud.find_by_name("draw_minimap");
  if (minimap) {
    int cornerx = map->posx - minimap->sizex / 2 + MAP_SIZE / 2;
    int cornery = map->posy - minimap->sizey / 2 + MAP_SIZE / 2;
    worldmap->draw_minimap(minimap, cornerx, cornery);
  }
  if (player->weapon.is_real()) {
    i_hud.set_data("text_weapon", player->weapon.get_name_full());
  } else {
    i_hud.set_data("text_weapon", "None");
  }
  i_hud.set_data("hp_head",  player->hp_text(BODYPART_HEAD     ) );
  i_hud.set_data("hp_torso", player->hp_text(BODYPART_TORSO    ) );
  i_hud.set_data("hp_l_arm", player->hp_text(BODYPART_LEFT_ARM ) );
  i_hud.set_data("hp_r_arm", player->hp_text(BODYPART_RIGHT_ARM) );
  i_hud.set_data("hp_l_leg", player->hp_text(BODYPART_LEFT_LEG ) );
  i_hud.set_data("hp_r_leg", player->hp_text(BODYPART_RIGHT_LEG) );
  
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
      text << " x " << messages[index].count;
    }
    text << '\n';
    //debugmsg("Adding %s", text.str().c_str());
    i_hud.add_data("text_messages", text.str());
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
  if (!w_hud) {
    debugmsg("pickup_items() - w_hud is NULL!");
    return;
  }

  cuss::interface i_pickup;
  if (!i_pickup.load_from_file("cuss/i_pickup.cuss")) {
    debugmsg("Couldn't open cuss/i_pickup.cuss");
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
      player->add_item( (*available)[i] );
      available->erase(available->begin() + i);
      pick_up.erase(pick_up.begin() + i);
    }
  }

  std::string message = "You pick up " + list_items(&items_gotten);
  add_msg(message);
  
}

Point Game::target_selector(int startx, int starty)
{
  std::vector<Point> path = path_selector(startx, starty);
  if (path.empty()) {
    return Point(-1, -1);
  }
  return path.back();
}

std::vector<Point> Game::path_selector(int startx, int starty)
{
  std::vector<Point> ret;
  if (!player) {
    return ret;
  }
  if (startx == -1 || starty == -1) {
    startx = player->pos.x;
    starty = player->pos.y;
  }

  Tripoint target(startx, starty, player->pos.z);

  map->draw(w_map, &entities, player->pos);
  w_map->refresh();
  while (true) {
    long ch = input();
    if (ch == KEY_ESC || ch == 'q' || ch == 'Q') {
      std::vector<Point> empty;
      return empty;
    } else if (ch == '\n') {
      return ret;
    } else {
      Point p = input_direction(ch);
      if (p.x == 0 && p.y == 0) {
        return ret; // Return out path on hitting "pause"
      } else if (p.x != -2 && p.y != -2) {
        target += p;
        ret = map->line_of_sight(player->pos, target);
        map->draw(w_map, &entities, player->pos);
        for (int i = 0; i < ret.size(); i++) {
          map->draw_tile(w_map, &entities, ret[i].x, ret[i].y,
                         player->pos.x, player->pos.y, true); // true==inverted
        }
// TODO: No no no remove this
        w_map->putglyph(w_map->sizex() / 2 - player->pos.x + target.x,
                        w_map->sizey() / 2 - player->pos.y + target.y,
                        glyph('*', c_red, c_black));
        w_map->refresh();
      }
    }
  }
}

int Game::get_item_uid()
{
  return next_item_uid++;
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
        (picking_up ? " +" : " -") << " " << item->get_name();
  return ss.str();
}
