#include "worldmap.h"
#include "biome.h"
#include "rng.h"
#include <math.h>

#define PI 3.141
#define NUM_RADIAL_POINTS 150

void draw_island(std::vector<std::vector<int> > &altitude, Point center,
                 int height, int edge_dist);

void Worldmap::generate()
{
// Points_live is used below to track which points to update
  std::vector<Point> points_live;
// Used below when deciding when to turn lakes into ocean
  Lake_status lake[WORLDMAP_SIZE][WORLDMAP_SIZE];
  std::vector<Point> lake_seeds;
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      biomes[x][y] = NULL;
      lake[x][y] = LAKE_UNCHECKED;
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
      } else {
        lake[p.x][p.y] = LAKE_NOTLAKE;
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
      points_live.push_back( growth );
    }
  }

// Now look at the biomes and randomly select a terrain for each
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if (biomes[x][y]) {
        tiles[x][y].terrain = biomes[x][y]->pick_terrain();
      } else {
        tiles[x][y].terrain = WORLD_TERRAIN.lookup_name("ocean");
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
      }
    } else {
      river_seeds.push_back( lake_seeds[i] );
    }
  }

// For each river seed, draw a river that *tends* to slope down until it hits
// ocean.

/*
  for (int i = 0; i < river_seeds.size(); i++) {
    Point river_point = river_seeds[i];
    while (altitude[river_point.x][river_point.y] > 0) {
      if (terrain
*/

// Take everything with altitude <= 0 and set it to be ocean.
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if (altitude[x][y] <= 0) {
        tiles[x][y].terrain = WORLD_TERRAIN.lookup_name("ocean");
      } else {
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

