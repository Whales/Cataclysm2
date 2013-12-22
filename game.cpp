#include "game.h"
#include "window.h"

Game::Game()
{
  map     = NULL;
  worldmap = NULL;
  w_map   = NULL;
  player  = NULL;
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
/*
  if (!i_main.load_from_file("cuss/i_main.cuss")) {
    debugmsg("Can't load cuss/i_main.cuss!");
    return false;
  }
*/
  int xdim, ydim;
  get_screen_dims(xdim, ydim);
  int win_size = ydim;
  if (win_size % 2 == 0) {
    win_size--; // Only odd numbers allowed!
  }
  w_map = new Window(0, 0, win_size, win_size);

  worldmap = new Worldmap;
  worldmap->generate();

  map = new Map;
  map->generate(worldmap, 0, 0);

  player = new Player;
  return true;
}

bool Game::main_loop()
{
  if (!w_map || !map) {
    return false;
  }

  map->draw(w_map, player->posx, player->posy);
  w_map->putglyph( w_map->sizex() / 2, w_map->sizey() / 2, player->get_glyph());
  w_map->refresh();
  long ch = input();

  switch (ch) {
    case KEY_RESIZE: {
      int xdim, ydim;
      get_screen_dims(xdim, ydim);
      w_map->resize(xdim, ydim);
    } break;
    case 'j': player_move( 0,  1); break;
    case 'k': player_move( 0, -1); break;
    case 'h': player_move(-1,  0); break;
    case 'l': player_move( 1,  0); break;
    case 'y': player_move(-1, -1); break;
    case 'u': player_move( 1, -1); break;
    case 'b': player_move(-1,  1); break;
    case 'n': player_move( 1,  1); break;
    case ':': worldmap->draw(10, 10); break;
    case 'q': return false;
  }
  return true;
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
    player->move_to(newx, newy);
  }
}
