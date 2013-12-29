#include "game.h"
#include "window.h"
#include "stringfunc.h"
#include "map.h"
#include <stdarg.h>
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
  if (player) {
    delete player;
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
  map->generate(worldmap, 0, 0);

  player = new Player;

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
    shift_if_needed();
    update_hud();
    map->draw(w_map, &monsters, player->posx, player->posy);
    w_map->putglyph(w_map->sizex() / 2, w_map->sizey() / 2,
                    player->get_glyph());
    w_map->refresh();

    long ch = input();
// Special testing function
    if (ch == '!') {
      Monster *z = new Monster;
      z->set_type("zombie");
      z->posx = player->posx - 3;
      z->posy = player->posy - 3;
      monsters.add_monster(z);
    } else if (ch == '?') {
/*
      debugmsg("%d item_types", ITEM_TYPES.size());
      for (std::list<Item_type*>::iterator it = ITEM_TYPES.instances.begin();
           it != ITEM_TYPES.instances.end();
           it++) {
        debugmsg("%s", (*it)->name.c_str());
      }
*/
      add_msg(itos(map->items_at(player->posx, player->posy)->size()).c_str());
    }
    Interface_action act = KEYBINDINGS.bound_to_key(ch);
    do_action(act);
  }
  shift_if_needed();
  move_monsters();
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

    case IACTION_PICK_UP:
      if (map->item_count(player->posx, player->posy) == 0) {
        add_msg("No items here.");
      } else if (map->item_count(player->posx, player->posy) == 1) {
        std::vector<Item> *items = map->items_at(player->posx, player->posy);
        std::string message = "You pick up " + list_items(items);
        player->add_item( (*items)[0] );
        items->clear();
        add_msg(message.c_str());
      } else {
        pickup_items(player->posx, player->posy);
      }
      break;

    case IACTION_INVENTORY:
// TODO: Allow the player to perform an action upon items in their inventory
      player->inventory_single();
      break;

    case IACTION_DROP: {
      std::vector<Item> dropped = player->drop_items();
      std::stringstream message;
      message << "You drop " << list_items(&dropped);
      for (int i = 0; i < dropped.size(); i++) {
        map->add_item(dropped[i], player->posx, player->posy);
      }
      add_msg( message.str().c_str() );
    } break;

    case IACTION_MESSAGES_SCROLL_BACK:
      i_hud.add_data("text_messages", -1);
      break;
    case IACTION_MESSAGES_SCROLL_FORWARD:
      i_hud.add_data("text_messages",  1);
      break;

    case IACTION_VIEW_WORLDMAP:
      worldmap->draw(10, 10);
      break;

    case IACTION_QUIT:
      if (query_yn("Commit suicide?")) {
        game_over = true;
        player->action_points = 0;
      }
      break;
  }
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
  i_hud.set_data("hp_head",  player->hp_text(BODYPART_HEAD     ) );
  i_hud.set_data("hp_torso", player->hp_text(BODYPART_TORSO    ) );
  i_hud.set_data("hp_l_arm", player->hp_text(BODYPART_LEFT_ARM ) );
  i_hud.set_data("hp_r_arm", player->hp_text(BODYPART_RIGHT_ARM) );
  i_hud.set_data("hp_l_leg", player->hp_text(BODYPART_LEFT_LEG ) );
  i_hud.set_data("hp_r_leg", player->hp_text(BODYPART_RIGHT_LEG) );
  
  i_hud.draw(w_hud);
  w_hud->refresh();
}

void Game::shift_if_needed()
{
  int min = SUBMAP_SIZE * (MAP_SIZE / 2), max = min + SUBMAP_SIZE - 1;
  int shiftx = 0, shifty = 0;
  if (player->posx < min) {
    shiftx = -1 + (player->posx - min) / SUBMAP_SIZE;
  } else if (player->posx > max) {
    shiftx =  1 + (player->posx - max) / SUBMAP_SIZE;
  }
  if (player->posy < min) {
    shifty = -1 + (player->posy - min) / SUBMAP_SIZE;
  } else if (player->posy > max) {
    shifty =  1 + (player->posy - max) / SUBMAP_SIZE;
  }
  map->shift(worldmap, shiftx, shifty);
  player->posx -= shiftx * SUBMAP_SIZE;
  player->posy -= shifty * SUBMAP_SIZE;
}

void Game::move_monsters()
{
// First, give all monsters action points
  for (std::list<Monster*>::iterator it = monsters.instances.begin();
       it != monsters.instances.end();
       it++) {
    (*it)->gain_action_points();
  }
/* Loop through the monsters, giving each one a single turn at a time.
 * Stop when we go through a loop without finding any monsters that can
 * take a turn.
 */
  bool all_done = true;
  do {
    all_done = true;
    for (std::list<Monster*>::iterator it = monsters.instances.begin();
         it != monsters.instances.end();
         it++) {
      Monster* mon = *it;
      if (mon->action_points > 0 && !mon->dead) {
        mon->take_turn();
        all_done = false;
      }
    }
  } while (!all_done);
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
  add_msg(message.c_str());
  
}

void Game::player_move(int xdif, int ydif)
{
// TODO: Remove this?
  if (xdif < -1 || xdif > 1 || ydif < -1 || ydif > 1) {
    debugmsg("Game::player_move called with [%d, %d]", xdif, ydif);
    return;
  }

  int newx = player->posx + xdif, newy = player->posy + ydif;
  if (player->can_move_to(map, newx, newy)) {
    player->move_to(map, newx, newy);
  }
  std::vector<Item> *items = map->items_at(player->posx, player->posy);
// TODO: Ensure the player has the sense of sight
  if (!items->empty()) {
    std::string item_message = "You see here " + list_items(items);
    add_msg( item_message.c_str() );
  }
}

void Game::add_msg(const char* msg, ...)
{
  char buff[2048];
  va_list ap;
  va_start(ap, msg);
  vsprintf(buff, msg, ap);
  va_end(ap);
  std::string text(buff);
  if (text.empty()) {
    return;
  }
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

void Game::print_messages()
{
/*
  for (int i = 0; i < new_messages; i++) {
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
  new_messages = 0;
*/
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
