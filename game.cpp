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
  map->generate(worldmap, 10, 10);

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
    case 'j': player->posy++; break;
    case 'k': player->posy--; break;
    case 'h': player->posx--; break;
    case 'l': player->posx++; break;
    case ':': worldmap->draw(10, 10); break;
    case 'q': return false;
  }
  return true;
}
