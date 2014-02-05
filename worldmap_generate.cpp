#include "worldmap.h"
#include "biome.h"
#include "rng.h"
#include "globals.h"
#include "window.h"
#include <math.h>

#define NUM_RADIAL_POINTS 150

void draw_island(std::vector<std::vector<int> > &altitude, Point center,
                 int height, int edge_dist);

void Worldmap::generate()
{
// Points_live is used below to track which points to update
  std::vector<Point> points_live;
// Used below when deciding when to turn lakes into ocean
  Lake_status lake[WORLDMAP_SIZE][WORLDMAP_SIZE];
  City_status city[WORLDMAP_SIZE][WORLDMAP_SIZE];
  std::vector<Point> lake_seeds;
  std::vector<Point> city_seeds;
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      biomes[x][y] = NULL;
      lake[x][y] = LAKE_NOTLAKE;
      city[x][y] = CITY_NOTCITY;
    }
  }
// Randomly seed biomes
  for (std::list<Biome*>::iterator it = BIOMES.instances.begin();
       it != BIOMES.instances.end();
       it++) {
    for (int n = 0; n < WORLDMAP_SIZE / 10; n++) {
      Point p( rng(0, WORLDMAP_SIZE - 1), rng(0, WORLDMAP_SIZE - 1) );
      points_live.push_back(p);
      if ((*it)->has_flag(BIOME_FLAG_LAKE)) {
        lake_seeds.push_back(p);
        lake[p.x][p.y] = LAKE_UNCHECKED;
      }
      if ((*it)->has_flag(BIOME_FLAG_CITY)) {
        city_seeds.push_back(p);
        city[p.x][p.y] = CITY_HUB;
      }
      biomes[p.x][p.y] = (*it);
    }
  }

  while (!points_live.empty()) {
    std::vector<Point> new_points;
    //std::vector<Point> points_copy = points_live;
    int i = rng(0, points_live.size() - 1);
    std::vector<Point> valid_growth;
    Point p = points_live[i];
    if (p.x > 0 && biomes[p.x - 1][p.y] == NULL) {
      valid_growth.push_back( Point(p.x - 1, p.y) );
    }
    if (p.y > 0 && biomes[p.x][p.y - 1] == NULL) {
      valid_growth.push_back( Point(p.x, p.y - 1) );
    }
    if (p.x < WORLDMAP_SIZE - 1 && biomes[p.x + 1][p.y] == NULL) {
      valid_growth.push_back( Point(p.x + 1, p.y) );
    }
    if (p.y < WORLDMAP_SIZE - 1 && biomes[p.x][p.y + 1] == NULL) {
      valid_growth.push_back( Point(p.x, p.y + 1) );
    }
    if (valid_growth.empty()) { // No valid points - this point is dead!
      points_live.erase(points_live.begin() + i);
      i--;
    } else {
      Point growth = valid_growth[rng(0, valid_growth.size() - 1)];
      biomes[growth.x][growth.y] = biomes[p.x][p.y];
      lake[growth.x][growth.y] = lake[p.x][p.y];
      if (city[p.x][p.y] == CITY_HUB) {
        city[growth.x][growth.y] = CITY_RAW;
      } else {
        city[growth.x][growth.y] = city[p.x][p.y];
      }
      points_live.push_back( growth );
    }
  }

// Now look at the biomes and randomly select a terrain for each
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if (biomes[x][y]) {
        tiles[x][y].terrain = biomes[x][y]->pick_terrain();
      } else {
        tiles[x][y].set_terrain("ocean");
      }
    }
  }

/* At this point, we have a lot of blobs of terrain, but no ocean!
 * The draw_island function sets altitude to 100 at its center and randomly
 * slopes down in a way that introduces penisulas &c
 */

  int center = WORLDMAP_SIZE / 2, shift = WORLDMAP_SIZE / 10;
  Point island_center( rng(center - shift, center + shift),
                       rng(center - shift, center + shift) );
  std::vector<std::vector<int> > altitude;
  std::vector<int> tmpvec;
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    tmpvec.push_back(0);
  }
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    altitude.push_back(tmpvec);
  }

  draw_island(altitude, island_center, 400, 20);

// Now draw several (8) more, small islands
  for (int i = 0; i < 8; i++) {
    Point islet;
    switch (rng(1, 4)) { // Which side to place it along?
      case 1:
        islet.x = rng(0, WORLDMAP_SIZE - 1);
        islet.y = rng(15, 40);
        break;
      case 2:
        islet.x = rng(WORLDMAP_SIZE - 41, WORLDMAP_SIZE - 16);
        islet.y = rng(0, WORLDMAP_SIZE - 1);
        break;
      case 3:
        islet.x = rng(0, WORLDMAP_SIZE - 1);
        islet.y = rng(WORLDMAP_SIZE - 41, WORLDMAP_SIZE - 16);
        break;
      case 4:
        islet.x = rng(15, 40);
        islet.y = rng(0, WORLDMAP_SIZE - 1);
        break;
    }
    int size = 80;
    draw_island(altitude, islet, size, 2);
    while (one_in(3)) { // island chain
      if (one_in(2)) {
        islet.x -= rng(size / 5, size / 3);
      } else {
        islet.x += rng(size / 5, size / 3);
      }
      if (one_in(2)) {
        islet.y -= rng(size / 5, size / 3);
      } else {
        islet.y += rng(size / 5, size / 3);
      }
      size -= rng(0, 20);
      draw_island(altitude, islet, size, 2);
    }
  }

// Now find all lake biomes that are ocean-adjacent and make them shallows.
// Also, all the surviving lakes should become a river seed
  std::vector<Point> river_seeds;
  for (int i = 0; i < lake_seeds.size(); i++) {
    std::vector<Point> lake_points;
    std::vector<Point> live_points;
    lake_points.push_back( lake_seeds[i] );
    live_points.push_back( lake_seeds[i] );
    bool ocean = false;
    while (!live_points.empty()) {
      Point p = live_points[0];
      for (int x = p.x - 1; x <= p.x + 1; x++) {
        for (int y = p.y - 1; y <= p.y + 1; y++) {
          if (x >= 0 && x < WORLDMAP_SIZE && y >= 0 && y < WORLDMAP_SIZE) {
            if (lake[x][y] == LAKE_UNCHECKED) {
              lake_points.push_back( Point(x, y) );
              live_points.push_back( Point(x, y) );
              lake[x][y] = LAKE_CHECKED;
            } else if (!ocean && altitude[x][y] <= 0) {
              ocean = true;
            }
          }
        }
      }
      live_points.erase(live_points.begin());
    }
    if (ocean) {
      for (int i = 0; i < lake_points.size(); i++) {
        Point p = lake_points[i];
        altitude[p.x][p.y] = 0;
        //set_terrain(p.x, p.y, "tester");
      }
    } else {
      river_seeds.push_back( lake_seeds[i] );
    }
  }

// For each river seed, draw a river that *tends* to slope down until it hits
// ocean.

  for (int i = 0; i < river_seeds.size(); i++) {
    Point rp = river_seeds[i];
    bool done = false;
// TODO: This occasionally fails to terminate.
    while (!done) {
      if (!tiles[rp.x][rp.y].terrain->has_flag(WTF_NO_RIVER) &&
          !tiles[rp.x][rp.y].terrain->has_flag(WTF_WATER)      ) {
        tiles[rp.x][rp.y].set_terrain("river");
      }
      std::vector<Point> next;
      std::vector<int> chances;
      int total_chance = 0;
      for (int n = 1; n <= 4; n++) {
        int x, y;
        switch (n) {
          case 1: x = rp.x - 1; y = rp.y    ; break;
          case 2: x = rp.x + 1; y = rp.y    ; break;
          case 3: x = rp.x    ; y = rp.y - 1; break;
          case 4: x = rp.x    ; y = rp.y + 1; break;
        }
        if (x < 0 || x >= WORLDMAP_SIZE || y < 0 || y >= WORLDMAP_SIZE ||    
            tiles[x][y].terrain->has_flag(WTF_SALTY) || altitude[x][y] <= 0) {
          done = true;
// no_river tiles are only acceptable if it has the water flag too
        } else if (!tiles[x][y].terrain->has_flag(WTF_NO_RIVER) ||
                   tiles[x][y].terrain->has_flag(WTF_WATER)) {
          next.push_back( Point(x, y) );
          int chance;
          if (altitude[x][y] > altitude[rp.x][rp.y]) {
            //chance = 100 + altitude[rp.x][rp.y] - altitude[x][y];
            chance = 5;
          } else { // Better chance for places we slope down to
            //chance = 150 + altitude[rp.x][rp.y] - altitude[x][y];
            chance = 20;
          }
          if (tiles[x][y].terrain->has_flag(WTF_WATER)) {
            chance += 7;
          }
          chances.push_back(chance);
          total_chance += chance;
        }
      }
      if (chances.empty()) {
        done = true;
      }
// Now pick from among those options.
      if (!done) {
        int index = rng(1, total_chance);
        bool pick_done = false;
        for (int i = 0; !pick_done && i < chances.size(); i++) {
          index -= chances[i];
          if (index <= 0) {
            rp = next[i];
            pick_done = true;
          }
        }
      }
    }
  }

// Take everything with altitude <= 0 and set it to be ocean.
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if (altitude[x][y] <= 0 && !biomes[x][y]->has_flag(BIOME_FLAG_NO_OCEAN)) {
        tiles[x][y].set_terrain("ocean");
        biomes[x][y] = BIOMES.lookup_name("ocean");
// If it's a hub, i.e. a city_seed, try to reposition it within 5 tiles
        if (city[x][y] == CITY_HUB) {
          for (int i = 0; i < city_seeds.size(); i++) {
            if (city_seeds[i].x == x && city_seeds[i].y == y) {
              i = city_seeds.size();
              bool done = false;
              for (int rad = 1; !done && rad <= 5; rad++) {
                for (int rx = x - rad; !done && rx <= x + rad; rx++) {
                  int ry = y - rad;
                  if (rx >= 0 && rx < WORLDMAP_SIZE &&
                      ry >= 0 && ry < WORLDMAP_SIZE &&
                      altitude[rx][ry] > 0 && city[rx][ry] != CITY_NOTCITY &&
                      !done) {
                    city[rx][ry] = CITY_RAW;
                    city_seeds.push_back( Point(rx, ry) );
                    done = true;
                  }
                  ry = y + rad;
                  if (rx >= 0 && rx < WORLDMAP_SIZE &&
                      ry >= 0 && ry < WORLDMAP_SIZE &&
                      altitude[rx][ry] > 0 && city[rx][ry] != CITY_NOTCITY &&
                      !done) {
                    city[rx][ry] = CITY_RAW;
                    city_seeds.push_back( Point(rx, ry) );
                    done = true;
                  }
                }
                for (int ry = y - rad; !done && ry <= y + rad; ry++) {
                  int rx = x - rad;
                  if (rx >= 0 && rx < WORLDMAP_SIZE &&
                      ry >= 0 && ry < WORLDMAP_SIZE &&
                      altitude[rx][ry] > 0 && city[rx][ry] != CITY_NOTCITY &&
                      !done) {
                    city[rx][ry] = CITY_RAW;
                    city_seeds.push_back( Point(rx, ry) );
                    done = true;
                  }
                  rx = x + rad;
                  if (rx >= 0 && rx < WORLDMAP_SIZE &&
                      ry >= 0 && ry < WORLDMAP_SIZE &&
                      altitude[rx][ry] > 0 && city[rx][ry] != CITY_NOTCITY &&
                      !done) {
                    city[rx][ry] = CITY_RAW;
                    city_seeds.push_back( Point(rx, ry) );
                    done = true;
                  }
                }
              }
            }
          }
        }
        city[x][y] = CITY_NOTCITY;
      } else {
        if (city[x][y] == CITY_HUB) {
          city[x][y] = CITY_RAW;
        }
        int range = tiles[x][y].terrain->beach_range;
        if (range != -1) {
          for (int xn = x - range; xn <= x + range; xn++) {
            for (int yn = y - range; yn <= y + range; yn++) {
              if (xn >= 0 && xn < WORLDMAP_SIZE &&
                  yn >= 0 && yn < WORLDMAP_SIZE && altitude[xn][yn] <= 0) {
                tiles[x][y].terrain = make_into_beach(tiles[x][y].terrain);
              }
            }
          }
        }
      }
    }
  }

// Erase any remaining below-water cities
  for (int i = 0; i < city_seeds.size(); i++) {
    Point p = city_seeds[i];
    if (tiles[p.x][p.y].terrain->road_cost <= 0 ||
        altitude[p.x][p.y] <= 0) {
      city_seeds.erase(city_seeds.begin() + i);
      i--;
    }
  }
// Draw some roads between cities.
  if (city_seeds.size() > 1) {
    for (int i = 0; i < city_seeds.size(); i++) {
      Generic_map gmap = get_generic_map();
      Pathfinder pf(gmap);
      pf.set_allow_diagonal(false);
      pf.set_bounds(20);
      Point from = city_seeds[i];
// This is a roll to get any index EXCEPT the current one;
// If we roll the current one, use the last one (which the roll skips)
      int index = rng(0, city_seeds.size() - 2);
      if (index == i) {
        index = city_seeds.size() - 1;
      }
      Point to = city_seeds[index];

      Path path = pf.get_path(PATH_A_STAR, from, to);
      if (path.get_cost() <= 150000) {
        for (int n = 0; n < path.size(); n++) {
          Tripoint p = path[n];
          if (!tiles[p.x][p.y].terrain->has_flag(WTF_NO_ROAD)) {
            if (tiles[p.x][p.y].terrain->has_flag(WTF_BRIDGE)) {
              tiles[p.x][p.y].set_terrain("bridge");
            } else {
              tiles[p.x][p.y].set_terrain("road");
              if (city[p.x][p.y] != CITY_NOTCITY) {
                city[p.x][p.y] = CITY_ROAD;
              }
            }
          }
        }
      }
    }
  }

// Now fill in cities!
  for (int i = 0; i < city_seeds.size(); i++) {
    std::vector<Point> active;
    active.push_back( city_seeds[i] );
    bool vertical_blocks = one_in(2);
    int block_size = rng(4, 8);
    while (!active.empty()) {
      int index = 0;
      Point p = active[index];
      if (( vertical_blocks && (p.x % 3 != 0 && p.y % block_size != 0)) ||
          (!vertical_blocks && (p.y % 3 != 0 && p.x % block_size != 0))   ) {
        index = rng(0, active.size() - 1);
        p = active[index];
      }
      active.erase(active.begin() + index);
      bool expansions = false;
      int roads = 0;
      City_status stat = city[p.x][p.y];
      for (int n = 0; n < 4; n++) {
        Point expand;
        switch (n) {
          case 0: expand = Point(p.x - 1, p.y    ); break;
          case 1: expand = Point(p.x + 1, p.y    ); break;
          case 2: expand = Point(p.x    , p.y - 1); break;
          case 3: expand = Point(p.x    , p.y + 1); break;
        }
        if (expand.x >= 0 && expand.x < WORLDMAP_SIZE &&
            expand.y >= 0 && expand.y < WORLDMAP_SIZE) {
          if (city[expand.x][expand.y] == CITY_RAW &&
              stat != CITY_BUILDING_CLOSED) {
            if (( vertical_blocks &&
                 (expand.x % 3 == 0 || expand.y % block_size == 0)) ||
                (!vertical_blocks && 
                 (expand.y % 3 == 0 || expand.x % block_size == 0))   ) {
              active.insert(active.begin(), expand);
            } else {
              active.push_back( expand );
            }
            city[expand.x][expand.y] = CITY_BUILDING;
            if (rl_dist(expand.x, expand.y, city_seeds[i].x, city_seeds[i].y) <=
                rng(1, 6)) {
              tiles[expand.x][expand.y].terrain = random_shop();
            } else {
              tiles[expand.x][expand.y].set_terrain("house");
            }
            expansions = true;
          } else if (city[expand.x][expand.y] == CITY_ROAD) {
            if (stat != CITY_BUILDING_CLOSED) {
              active.push_back( expand );
            }
            roads++;
          } else if (city[expand.x][expand.y] == CITY_ROAD_CLOSED) {
            roads++;
          } else if (city[expand.x][expand.y] == CITY_BUILDING && 
                     stat != CITY_BUILDING_CLOSED) {
            city[expand.x][expand.y] = CITY_BUILDING_CLOSED;
          }
        }
       
        if (!expansions) {
          bool block_closer = (( vertical_blocks && p.x % 3 == 0) ||
                               (!vertical_blocks && p.y % 3 == 0)   );
          if (block_closer) {
            if (vertical_blocks && p.y % block_size != 1 &&
                                   p.y % block_size != block_size - 1) {
              block_closer = false;
            } else if (!vertical_blocks && p.x % block_size != 1 &&
                                           p.x % block_size != block_size - 1) {
              block_closer = false;
            }
          }
          if (roads == 2 && block_closer) {
            city[p.x][p.y] = CITY_ROAD_CLOSED;
            tiles[p.x][p.y].set_terrain("road");
          } else if (city[p.x][p.y] == CITY_BUILDING) {
            city[p.x][p.y] = CITY_BUILDING_CLOSED;
          } else if (city[p.x][p.y] == CITY_ROAD) {
            city[p.x][p.y] = CITY_ROAD_CLOSED;
          }
        } else {
          city[p.x][p.y] = CITY_ROAD_CLOSED;
          tiles[p.x][p.y].set_terrain("road");
        }
      }
    }
  }

// Take any usued city tiles and make them field
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if (city[x][y] == CITY_RAW) {
        tiles[x][y].set_terrain("field");
        biomes[x][y] = BIOMES.lookup_name("grassland");
        int range = tiles[x][y].terrain->beach_range;
        for (int xn = x - range; xn <= x + range; xn++) {
          for (int yn = y - range; yn <= y + range; yn++) {
            if (xn >= 0 && xn < WORLDMAP_SIZE &&
                yn >= 0 && yn < WORLDMAP_SIZE && altitude[xn][yn] <= 0) {
              tiles[x][y].terrain = make_into_beach(tiles[x][y].terrain);
            }
          }
        }
      }
    }
  }
// Finally, place mosnters!
  place_monsters();
}

void Worldmap::place_monsters()
{
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if (biomes[x][y]) {
        Variable_monster_genus var = biomes[x][y]->monsters;
// Decide how many genera to place here.
        int placed = var.pick_number();
        if (placed > 0) {
          std::vector<Monster_genus*> gens = var.pick(placed);
          for (int i = 0; i < gens.size(); i++) {
            Monster_genus* genus = gens[i];
            int population = biomes[x][y]->monster_population.roll() / placed;
            if (genus && population > 0) {
              Monster_spawn tmpspawn;
              tmpspawn.genus = genus;
              tmpspawn.population = population;
              tiles[x][y].monsters.push_back(tmpspawn);
            }
          }
        }
      }
    }
  }
}

void draw_island(std::vector<std::vector<int> > &altitude, Point center,
                 int height, int edge_dist)
{
  if (center.x < 0 || center.x >= WORLDMAP_SIZE ||
      center.y < 0 || center.y >= WORLDMAP_SIZE   ) {
    return;
  }
  altitude[center.x][center.y] = height;
  std::vector<Point> points_active;
  points_active.push_back(center);
  int center_point = WORLDMAP_SIZE / 2;
  int shift = WORLDMAP_SIZE / 10;
  while (!points_active.empty()) {
    std::vector<Point> new_points;
    while (!points_active.empty()) {
      int index = rng(0, points_active.size() - 1);
      Point p = points_active[index];
      for (int i = 0; i < 4; i++) {
        int x, y;
        switch (i) {
          case 0: x = p.x - 1; y = p.y;     break;
          case 1: x = p.x + 1; y = p.y;     break;
          case 2: x = p.x;     y = p.y - 1; break;
          case 3: x = p.x;     y = p.y + 1; break;
        }
        if (x > 0 && x < WORLDMAP_SIZE && y > 0 && y < WORLDMAP_SIZE &&
            altitude[x][y] == 0) {
          int dist_from_edge = (x > center_point ? WORLDMAP_SIZE - x : x);
          int y_dist = (y > center_point ? WORLDMAP_SIZE - y : y);
          if (y_dist < dist_from_edge) {
            dist_from_edge = y_dist;
          }
          new_points.push_back( Point(x, y) );
          altitude[x][y] = altitude[p.x][p.y];
          if (dist_from_edge < rng(0, edge_dist)) {
            altitude[x][y] -= rng(0, 100);
          } else if (one_in(30)) {
            altitude[x][y] -= rng(0, 100);
          } else if (!one_in(10)) {
            altitude[x][y] -= rng(0, shift);
          }
        }
      }
      points_active.erase(points_active.begin() + index);
    }
    points_active = new_points;
  }
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if (altitude[x][y] < 0) {
        altitude[x][y] = 0;
      }
    }
  }
}
