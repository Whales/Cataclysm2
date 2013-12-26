#include "game.h"
#include "window.h"
#include "stringfunc.h"
#include <stdarg.h>
#include <sstream>

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
  return true;
}

bool Game::main_loop()
{
// Sanity check
  if (!w_map || !w_hud || !player || !worldmap || !map) {
    return false;
  }
  player->gain_action_points();
  while (player->action_points > 0) {
    update_hud();
    map->draw(w_map, &monsters, player->posx, player->posy);
    w_map->putglyph(w_map->sizex() / 2, w_map->sizey() / 2,
                    player->get_glyph());
    w_map->refresh();

    long ch = input();
    Interface_action act = KEYBINDINGS.bound_to_key(ch);
    do_action(act);
  }
  move_monsters();
  return true;
}

void Game::do_action(Interface_action act)
{
  switch (act) {
    case IACTION_MOVE_N:  player_move( 0, -1); break;
    case IACTION_MOVE_S:  player_move( 0,  1); break;
    case IACTION_MOVE_W:  player_move(-1,  0); break;
    case IACTION_MOVE_E:  player_move( 1,  0); break;
    case IACTION_MOVE_NW: player_move(-1, -1); break;
    case IACTION_MOVE_NE: player_move( 1, -1); break;
    case IACTION_MOVE_SW: player_move(-1,  1); break;
    case IACTION_MOVE_SE: player_move( 1,  1); break;
    case IACTION_MESSAGES_SCROLL_BACK:
      i_hud.add_data("text_messages", -1); break;
    case IACTION_MESSAGES_SCROLL_FORWARD:
      i_hud.add_data("text_messages",  1); break;
    case IACTION_VIEW_WORLDMAP:
      worldmap->draw(10, 10); break;
    case IACTION_QUIT:
      if (query_yn("Commit suicide?")) {
        game_over = true;
      }
      break;
  }
}

void Game::update_hud()
{
  print_messages();
  i_hud.set_data("hp_head",  player->hp_text(BODYPART_HEAD     ) );
  i_hud.set_data("hp_torso", player->hp_text(BODYPART_TORSO    ) );
  i_hud.set_data("hp_l_arm", player->hp_text(BODYPART_LEFT_ARM ) );
  i_hud.set_data("hp_r_arm", player->hp_text(BODYPART_RIGHT_ARM) );
  i_hud.set_data("hp_l_leg", player->hp_text(BODYPART_LEFT_LEG ) );
  i_hud.set_data("hp_r_leg", player->hp_text(BODYPART_RIGHT_LEG) );
  i_hud.draw(w_hud);
  w_hud->refresh();
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
