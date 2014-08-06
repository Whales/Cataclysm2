#include "worldmap.h"
#include "rng.h"
#include "map.h"
#include "files.h"  // for SAVE_DIR
#include "stringfunc.h" // For name randomization
#include "game.h"
#include <fstream>
#include <sstream>

glyph Worldmap_tile::top_glyph()
{
  if (!terrain) {
    return glyph();
  }
  return terrain->sym;
}

std::string Worldmap_tile::get_name()
{
  if (!terrain) {
    return "Unknown";
  }
  return terrain->get_name();
}

bool Worldmap_tile::has_flag(World_terrain_flag flag)
{
  if (!terrain) {
    return false;
  }
  return terrain->has_flag(flag);
}

void Worldmap_tile::set_terrain(std::string name)
{
  World_terrain *ter = WORLD_TERRAIN.lookup_name(name);
  if (!ter) {
    debugmsg("Couldn't find world terrain named '%s'", name.c_str());
  } else {
    terrain = ter;
  }
}

std::string Worldmap_tile::save_data()
{
  std::stringstream ret;
  if (terrain) {
    ret << terrain->uid;
  } else {
    ret << -1;
  }
  ret << " " << monsters.size() << " ";
  for (int i = 0; i < monsters.size(); i++) {
    if (monsters[i].genus) {
      ret << monsters[i].genus->uid;
    } else {
      ret << -1;
    }
    ret << " " << monsters[i].population << " ";
  }
  return ret.str();
}

void Worldmap_tile::load_data(std::istream& data)
{
  int tmpter;
  data >> tmpter;
  if (tmpter == -1) {
    terrain = NULL;
  } else {
    terrain = WORLD_TERRAIN.lookup_uid(tmpter);
  }
  int monster_size;
  data >> monster_size;
  for (int i = 0; i < monster_size; i++) {
    Monster_spawn tmpspawn;
    int tmpgenus;
    data >> tmpgenus;
    if (tmpgenus == -1) {
      tmpspawn.genus = NULL;
    } else {
      tmpspawn.genus = MONSTER_GENERA.lookup_uid(tmpgenus);
    }
    data >> tmpspawn.population;
    if (tmpspawn.genus) {
      monsters.push_back(tmpspawn);
    }
  }
}

Worldmap::Worldmap()
{
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      tiles[x][y].terrain = NULL;
    }
  }

  for (std::list<World_terrain*>::iterator it = WORLD_TERRAIN.instances.begin();
       it != WORLD_TERRAIN.instances.end();
       it++) {
    if ( (*it)->has_flag(WTF_SHOP) ) {
      shops.push_back( (*it) );
    }
  }

  if (shops.empty()) {
    debugmsg("No shops available!");
  }
}

Worldmap::~Worldmap()
{
}

std::string Worldmap::get_name()
{
  return name;
}

void Worldmap::set_name(std::string N)
{
  name = N;
}

// TODO:  At some point we should create a Random_name class or file.
//        At that point, this should be rewritten to use that code.
void Worldmap::randomize_name()
{
  std::string sub_a, sub_b, sub_c;
  name = "";
  switch (rng(1, 25)) {
    case  1:  sub_a = "An";   break;
    case  2:  sub_a = "Ber";  break;
    case  3:  sub_a = "Can";  break;
    case  4:  sub_a = "Dar";  break;
    case  5:  sub_a = "Ein";  break;
    case  6:  sub_a = "Far";  break;
    case  7:  sub_a = "Gor";  break;
    case  8:  sub_a = "Her";  break;
    case  9:  sub_a = "In";   break;
    case 10:  sub_a = "Jin";  break;
    case 11:  sub_a = "Kin";  break;
    case 12:  sub_a = "Ler";  break;
    case 13:  sub_a = "Mon";  break;
    case 14:  sub_a = "Nar";  break;
    case 15:  sub_a = "Os";   break;
    case 16:  sub_a = "Per";  break;
    case 17:  sub_a = "Qua";  break;
    case 18:  sub_a = "Rus";  break;
    case 19:  sub_a = "Sun";  break;
    case 20:  sub_a = "Tor";  break;
    case 21:  sub_a = "Un";   break;
    case 22:  sub_a = "Vor";  break;
    case 23:  sub_a = "Win";  break;
    case 24:  sub_a = "Yu";   break;
    case 25:  sub_a = "Zin";  break;
  }

  switch (rng(1, 40)) {
    case  1:  sub_b = "ad";   break;
    case  2:  sub_b = "ang";  break;
    case  3:  sub_b = "bal";  break;
    case  4:  sub_b = "bon";  break;
    case  5:  sub_b = "cal";  break;
    case  6:  sub_b = "cad";  break;
    case  7:  sub_b = "dov";  break;
    case  8:  sub_b = "der";  break;
    case  9:  sub_b = "ev";   break;
    case 10:  sub_b = "fer";  break;
    case 11:  sub_b = "gan";  break;
    case 12:  sub_b = "gol";  break;
    case 13:  sub_b = "iv";   break;
    case 14:  sub_b = "il";   break;
    case 15:  sub_b = "kol";  break;
    case 16:  sub_b = "kan";  break;
    case 17:  sub_b = "lin";  break;
    case 18:  sub_b = "ov";   break;
    case 19:  sub_b = "ol";   break;
    case 20:  sub_b = "op";   break;
    case 21:  sub_b = "os";   break;
    case 22:  sub_b = "per";  break;
    case 23:  sub_b = "ser";  break;
    case 24:  sub_b = "san";  break;
    case 25:  sub_b = "tan";  break;
    case 26:  sub_b = "tor";  break;
    case 27:  sub_b = "til";  break;
    case 28:  sub_b = "uv";   break;
    case 29:  sub_b = "urn";  break;
    case 30:  sub_b = "vuv";  break;
    case 31:  sub_b = "vil";  break;
    case 32:  sub_b = "x";    break;
    case 33:  sub_b = "zan";  break;
    case 34:  sub_b = "zil";  break;
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
      if (is_vowel(sub_a[sub_a.size() - 1])) {
        sub_b = "";
      } else {
        sub_b = sub_a[sub_a.size() - 1];
      }
      break;
  }

  switch (rng(1, 15)) {
    case  1:
    case  2:
    case  3:  sub_c = "ia";   break;
    case  4:  sub_c = "a";    break;
    case  5:  sub_c = "land"; break;
    case  6:  sub_c = "iers"; break;
    case  7:  sub_c = "e";    break;
    case  8:  sub_c = "oa";   break;
    case  9:  sub_c = "ary";  break;
    case 10:  sub_c = "en";   break;
    case 11:  sub_c = "ium";  break;
    case 12:  sub_c = "us";   break;
    case 13:  sub_c = "any";  break;
    case 14:  sub_c = "ein";  break;
    case 15:  sub_c = "al";   break;
  }

  if (is_vowel(sub_a[sub_a.size() - 1]) && is_vowel(sub_b[0])) {
    sub_a += "'";
  }
  if (is_vowel(sub_b[sub_b.size() - 1]) && is_vowel(sub_c[0])) {
    sub_b += "'";
  }

  name = sub_a + sub_b + sub_c;
}

void Worldmap::set_terrain(int x, int y, std::string terrain_name)
{
  if (x < 0 || x >= WORLDMAP_SIZE || y < 0 || y >= WORLDMAP_SIZE) {
    return;
  }
  World_terrain* ter = WORLD_TERRAIN.lookup_name(terrain_name);
  if (!ter) {
    debugmsg("Worldmap::set_terrain() couldn't find '%s'",
             terrain_name.c_str());
    return;
  }
  tiles[x][y].terrain = ter;
}

void Worldmap::init_shop_picker()
{
  for (int i = 0; i < shops.size(); i++) {
    shop_count[ shops[i] ] = 0;
  }
}

World_terrain* Worldmap::random_shop()
{
  int most_shops = 0;
  for (int i = 0; i < shops.size(); i++) {
    if (shop_count[ shops[i] ] > most_shops) {
      most_shops = shop_count[ shops[i] ];
    }
  }

  int total_chance = 0;
  std::vector<int> chance;
  for (int i = 0; i < shops.size(); i++) {
    chance.push_back( most_shops * 2 - shop_count[ shops[i] ] );
    total_chance += chance.back();
  }

  int index = rng(1, total_chance);
  for (int i = 0; i < shops.size(); i++) {
    index -= chance[i];
    if (index <= 0) {
      shop_count[ shops[i] ]++;
      return shops[i];
    }
  }
  shop_count[ shops.back() ]++;
  return shops.back();
}

Point Worldmap::get_point(int posx, int posy)
{
  cuss::interface i_legend;
  if (!i_legend.load_from_file( CUSS_DIR + "/i_world_map.cuss" )) {
    return Point(-1, -1);
  }

  int xdim, ydim;
  get_screen_dims(xdim, ydim);
  Window w_worldmap(0, 0, xdim - 20, ydim);
  Window w_legend(xdim - 20, 0, 20, ydim);

  int origx = posx, origy = posy;
// Size of the window
  int winx = w_worldmap.sizex(), winy = w_worldmap.sizey();

  bool done = false;
  std::vector<Point> results;
  std::string search_term;
  int result_index = 0;
  i_legend.ref_data("entry_search", &search_term);
  i_legend.select_none();

  while (!done) {
// Fill out the legend
    i_legend.set_data("text_terrain_name", get_name(posx, posy));
    i_legend.set_data("num_distance", rl_dist(origx, origy, posx, posy));
// TODO: Fill out travel time.
    i_legend.set_data("num_found", results.size());
    for (int x = 0; x < winx; x++) {
      for (int y = 0; y < winy; y++) {
        int terx = posx + x - (winx / 2), tery = posy + y - (winy / 2);
        glyph sym = get_glyph(terx, tery);
        if ((terx == posx && tery == posy) ||
            (terx == origx && tery == origy) ) {
          sym = sym.invert();
// TODO: Remove this (except for testing mode)
        } else if (terx >= 0 && tery >= 0 &&
                   terx < WORLDMAP_SIZE && tery < WORLDMAP_SIZE &&
                   !tiles[terx][tery].monsters.empty()) {
          sym = sym.hilite(c_red);
        }
        w_worldmap.putglyph(x, y, sym);
      }
    }
    if (TESTING_MODE) {
      w_worldmap.putstr(0, 0, c_red, c_black, "[%d:%d]", posx, posy);
    }
    w_worldmap.refresh();
    i_legend.draw(&w_legend);
    w_legend.refresh();
    long ch = input();

    if (i_legend.selected()) {  // We've selected something!
      if (ch == KEY_ESC) {
        search_term = "";
        results.clear();
        result_index = 0;
        i_legend.select_none(); // Cancel entry.
      } else if (i_legend.handle_keypress(ch)) {
        ch = 0;
      }
    }

    switch (ch) {
      case '/':
        search_term = "";
        i_legend.select("entry_search");
        break;

      case '>':
        if (!results.empty()) {
          result_index++;
          if (result_index >= results.size()) {
            result_index = 0;
          }
          posx = results[result_index].x;
          posy = results[result_index].y;
        }
        break;
          
      case '<':
        if (!results.empty()) {
          result_index--;
          if (result_index < 0) {
            result_index = results.size() - 1;
          }
          posx = results[result_index].x;
          posy = results[result_index].y;
        }
        break;
          
// A bunch of movement
      case 'j':
      case '2':
      case KEY_DOWN:    posy++; break;
      case 'k':
      case '8':
      case KEY_UP:      posy--; break;
      case 'h':
      case '4':
      case KEY_LEFT:    posx--; break;
      case 'l':
      case '6':
      case KEY_RIGHT:   posx++; break;
      case 'y':
      case '7': posx--; posy--; break;
      case 'u':
      case '9': posx++; posy--; break;
      case 'b':
      case '1': posx--; posy++; break;
      case 'n':
      case '3': posx++; posy++; break;

      case '0':
        posx = origx;
        posy = origy;
        break;

      case 'q':
      case 'Q':
        done = true;
        posx = origx;
        posy = origy;
        break;

      case '\n':
        if (i_legend.selected()) {  // We were entering data!
          results = find_terrain(search_term);
          result_index = 0;
          if (!results.empty()) {
            posx = results[0].x;
            posy = results[0].y;
          }
          i_legend.select_none();
        } else {
          done = true;
        }
        break;
    }
  }
  return Point(posx, posy);
}

std::vector<Point> Worldmap::find_terrain(std::string name,
                                          Point origin, int range)
{
// Standardize our search term.
  name = no_caps( trim( name ) );

// Confirm that a World_terrain with that name exists - could save a lot of time
  if (!WORLD_TERRAIN.lookup_name(name)) {
    return std::vector<Point>();  // Terrain does not exist at all!
  }

// origin defaults to (-1, -1) and range defaults to -1
  std::vector<Point> ret;
  if (origin.x == -1) { // Default; use player's position
    origin = GAME.map->get_center_point();
  }
  int x0, x1, y0, y1; // Area to search in
  if (range < 0) { // Default; use infinite range
    x0 = 0;
    y0 = 0;
    x1 = WORLDMAP_SIZE - 1;
    y1 = WORLDMAP_SIZE - 1;
  } else {
    x0 = origin.x - range;
    x1 = origin.x + range;
    y0 = origin.y - range;
    y1 = origin.y + range;
    if (x0 < 0) {
      x0 = 0;
    }
    if (x1 >= WORLDMAP_SIZE) {
      x1 = WORLDMAP_SIZE - 1;
    }
    if (y0 < 0) {
      y0 = 0;
    }
    if (y1 >= WORLDMAP_SIZE) {
      y1 = WORLDMAP_SIZE - 1;
    }
  }

  for (int x = x0; x <= x1; x++) {
    for (int y = y0; y <= y1; y++) {
      std::string ter_name = no_caps( trim( get_name(x, y) ) );
      if (ter_name == name) {
        ret.push_back( Point(x, y) );
      }
    }
  }

  return ret;
}

void Worldmap::draw_minimap(cuss::element *drawing, int cornerx, int cornery)
{
  for (int x = 0; x < drawing->sizex; x++) {
    for (int y = 0; y < drawing->sizey; y++) {
      int terx = cornerx + x, tery = cornery + y;
      glyph sym = get_glyph(terx, tery);
      if (x == drawing->sizex / 2 && y == drawing->sizey / 2) {
        sym = sym.invert();
      }
      drawing->set_data(sym, x, y);
    }
  }
}


Worldmap_tile* Worldmap::get_tile(int x, int y, bool warn)
{
  if (x < 0 || x >= WORLDMAP_SIZE || y < 0 || y >= WORLDMAP_SIZE) {
    tile_oob.terrain = WORLD_TERRAIN.lookup_uid(0);
    tile_oob.monsters.clear();
    if (warn) {
      debugmsg("Worldmap::get_tile(%d, %d) OOB", x, y);
    }
    return &tile_oob;
  }

  return &(tiles[x][y]);
}

Worldmap_tile* Worldmap::get_tile(Point p, bool warn)
{
  return get_tile(p.x, p.y, warn);
}

glyph Worldmap::get_glyph(int x, int y)
{
/*
  if (x < 0 || x >= WORLDMAP_SIZE || y < 0 || y >= WORLDMAP_SIZE) {
    return glyph();
  }
*/
  Worldmap_tile* tile = get_tile(x, y, false);
  if (!tile) {
    return glyph();
  }
  glyph ret = tile->top_glyph();
  if (tile->terrain->has_flag(WTF_LINE_DRAWING)) {
    bool north = (get_tile(x, y - 1)->terrain->has_flag(WTF_LINE_DRAWING));
    bool  east = (get_tile(x + 1, y)->terrain->has_flag(WTF_LINE_DRAWING));
    bool south = (get_tile(x, y + 1)->terrain->has_flag(WTF_LINE_DRAWING));
    bool  west = (get_tile(x - 1, y)->terrain->has_flag(WTF_LINE_DRAWING));
    ret.make_line_drawing(north, east, south, west);
  }

  return ret;
}

std::string Worldmap::get_name(int x, int y)
{
  Worldmap_tile* tile = get_tile(x, y, false);
  if (!tile) {
    return "Unknown";
  }
  return tile->get_name();
}

std::vector<Monster_spawn>* Worldmap::get_spawns(int x, int y)
{
  Worldmap_tile* tile = get_tile(x, y);
  if (!tile) {
    return NULL;
  }
  return &(tile->monsters);
}

bool Worldmap::has_flag(World_terrain_flag flag, int x, int y)
{
  Worldmap_tile* tile = get_tile(x, y);
  if (!tile) {
    return false;
  }
  return tile->has_flag(flag);
}

Generic_map Worldmap::get_generic_map()
{
  Generic_map ret(WORLDMAP_SIZE, WORLDMAP_SIZE);
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      ret.set_cost(x, y, tiles[x][y].terrain->road_cost);
    }
  }
  return ret;
}

Point Worldmap::random_tile_with_terrain(std::string name, int island)
{
  return random_tile_with_terrain( WORLD_TERRAIN.lookup_name(name), island );
}

Point Worldmap::random_tile_with_terrain(World_terrain* terrain, int island)
{
  if (!terrain) {
    return Point(0, 0);
  }
  std::vector<Point> ret;
  if (island == -1) { // island defaults to -1, e.g., any island
    for (int x = 0; x < WORLDMAP_SIZE; x++) {
      for (int y = 0; y < WORLDMAP_SIZE; y++) {
        if (get_tile(x, y)->terrain == terrain) {
          ret.push_back( Point(x, y) );
        }
      }
    }
  } else {
    if (islands.count(island) == 0) {
      return Point(-1, -1);
    }
    for (int i = 0; i < islands[island].size(); i++) {
      if (get_tile( (islands[island])[i] )->terrain == terrain) {
        ret.push_back( (islands[island])[i] );
      }
    }
  }
  if (ret.empty()) {
    return Point(-1, -1);
  }

  return ret[rng(0, ret.size() - 1)];
}

bool Worldmap::save_to_name()
{
  if (name.empty()) {
    debugmsg("Worldmap::save_to_name() called on a nameless Worldmap!");
    return false;
  }
  std::string filename = SAVE_DIR + "/worlds/" + name + ".map";
  std::ofstream fout;
  fout.open(filename.c_str());
  if (!fout.is_open()) {
    debugmsg("Couldn't open '%s' for saving Worldmap.", filename.c_str());
    return false;
  }
  fout << save_data();
  fout.close();
  return true;
}

std::string Worldmap::save_data()
{
  std::stringstream ret;
// First, save name
  ret << name << std::endl;
// Next, islands
  std::map< int, std::vector<Point> >::iterator it;
  ret << islands.size() << " ";
  for (it = islands.begin(); it != islands.end(); it++) {
    ret << it->first << " ";
    ret << it->second.size() << " ";
    for (int i = 0; i < it->second.size(); i++) {
      ret << it->second[i].x << " " << it->second[i].y << " ";
    }
    ret << std::endl;
  }

/* Now, the terrain.  We don't need to save a "key" for this - the contents of
 * WORLDMAP_TERRAIN will be placed in the save folder along with this, and that
 * will serve as a key!
 */
  for (int y = 0; y < WORLDMAP_SIZE; y++) {
    for (int x = 0; x < WORLDMAP_SIZE; x++) {
      ret << tiles[x][y].save_data() << " ";
    }
    ret << std::endl;
  }

// And the biomes.  Ditto for the "key."
  for (int y = 0; y < WORLDMAP_SIZE; y++) {
    for (int x = 0; x < WORLDMAP_SIZE; x++) {
      if (biomes[x][y]) { // Sanity check
        ret << biomes[x][y]->uid;
      } else {
        ret << -1;
      }
      ret << " ";
    }
  }

// And we're done!  Relatively painless...
  return ret.str();
}

bool Worldmap::load_from_file(std::string filename)
{
  std::ifstream fin;
  fin.open(filename.c_str());
  if (!fin.is_open()) {
    debugmsg("Couldn't open '%s' for reading.", filename.c_str());
    return false;
  }
  return load_data(fin);
}

bool Worldmap::load_data(std::istream& data)
{
// Name's the first line
  std::getline(data, name);

// Then islands...
  int num_islands;
  data >> num_islands;
  for (int i = 0; i < num_islands; i++) {
    int island_key, num_points;
    data >> island_key >> num_points;
    std::vector<Point> island_points;
    for (int n = 0; n < num_points; n++) {
      Point tmppoint;
      data >> tmppoint.x >> tmppoint.y;
      island_points.push_back(tmppoint);
    }
    islands[island_key] = island_points;
  }

// Then all the tiles...
  for (int y = 0; y < WORLDMAP_SIZE; y++) {
    for (int x = 0; x < WORLDMAP_SIZE; x++) {
      tiles[x][y].load_data(data);
    }
  }

// And the biomes.
  for (int y = 0; y < WORLDMAP_SIZE; y++) {
    for (int x = 0; x < WORLDMAP_SIZE; x++) {
      int biome_uid;
      data >> biome_uid;
      if (biome_uid == -1) {
        biomes[x][y] = NULL;
      } else {
        biomes[x][y] = BIOMES.lookup_uid(biome_uid);
        if (!biomes[x][y]) {
          debugmsg("Failed to find Biome of UID %s.", biome_uid);
        }
      }
    }
  }

  return true;
}
