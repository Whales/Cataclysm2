#include "worldmap.h"
#include "biome.h"
#include "rng.h"
#include <math.h>

#define PI 3.141
#define NUM_RADIAL_POINTS 150

int radial_y_limit_above(const std::vector<int> &points, int x);
int radial_y_limit_below(const std::vector<int> &points, int x);

void draw_island(std::vector<std::vector<int> > &altitude, Point center,
                 int height, int edge_dist);

void Worldmap::generate()
{
  std::vector<Point> points_live;
  Biome_type biomes[WORLDMAP_SIZE][WORLDMAP_SIZE];
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      biomes[x][y] = BIOME_NULL;
    }
  }
// Randomly seed biomes
  for (int i = 1; i < BIOME_MAX; i++) {
    for (int n = 0; n < 8; n++) {
      Point p( rng(0, WORLDMAP_SIZE - 1), rng(0, WORLDMAP_SIZE - 1) );
      points_live.push_back(p);
      biomes[p.x][p.y] = Biome_type(i);
    }
  }

  while (!points_live.empty()) {
    std::vector<Point> new_points;
    //std::vector<Point> points_copy = points_live;
    //for (int i = 0; i < points_live.size(); i++) {
    int i = rng(0, points_live.size() - 1);
      std::vector<Point> valid_growth;
      Point p = points_live[i];
      if (p.x > 0 && biomes[p.x - 1][p.y] == BIOME_NULL) {
        valid_growth.push_back( Point(p.x - 1, p.y) );
      }
      if (p.y > 0 && biomes[p.x][p.y - 1] == BIOME_NULL) {
        valid_growth.push_back( Point(p.x, p.y - 1) );
      }
      if (p.x < WORLDMAP_SIZE - 1 && biomes[p.x + 1][p.y] == BIOME_NULL) {
        valid_growth.push_back( Point(p.x + 1, p.y) );
      }
      if (p.y < WORLDMAP_SIZE - 1 && biomes[p.x][p.y + 1] == BIOME_NULL) {
        valid_growth.push_back( Point(p.x, p.y + 1) );
      }
      if (valid_growth.empty()) { // No valid points - this point is dead!
        points_live.erase(points_live.begin() + i);
        i--;
      } else {
        Point growth = valid_growth[rng(0, valid_growth.size() - 1)];
        biomes[growth.x][growth.y] = biomes[p.x][p.y];
        //new_points.push_back(growth);
        points_live.push_back( growth );
      }
    //}
/*
    for (int i = 0; i < new_points.size(); i++) {
      points_live.push_back( new_points[i] );
    }
*/
  }

// Now look at the biomes and randomly select a terrain for each
  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      tiles[x][y].terrain = terrain_from_biome( biomes[x][y] );
    }
  }

/* At this point, we have a lot of blobs of terrain, but no ocean!
 * So: use radials, go around, and decide how far from the edge of the map the
 * ocean will extend.
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

  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    for (int y = 0; y < WORLDMAP_SIZE; y++) {
      if (altitude[x][y] <= 0) {
        tiles[x][y].terrain = terrain_from_biome(BIOME_NULL);
      }
    }
  }
/*
  std::vector<int> radial_points;
  int min = WORLDMAP_SIZE / 30;
  int max = WORLDMAP_SIZE / 10;
  int biggest_step = 2;
  radial_points.push_back( rng(min, max) );
  for (int i = 1; i < NUM_RADIAL_POINTS; i++) {
    int last = radial_points[i - 1];
    int min_diff, max_diff;
    if (last < min + biggest_step) {
      min_diff = min - last;
    } else {
      min_diff = 0 - biggest_step;
    }
    if (last > max - biggest_step) {
      max_diff = max - last;
    } else {
      max_diff = biggest_step;
    }
    radial_points.push_back(last + rng(min_diff, max_diff));
  }

  for (int x = 0; x < WORLDMAP_SIZE; x++) {
    int y_top = radial_y_limit_above(radial_points, x);
    debugmsg("x %d y_top %d", x, y_top);
    if (y_top == -1) {
      for (int y = 0; y < WORLDMAP_SIZE; y++) {
        tiles[x][y].terrain = terrain_from_biome(BIOME_NULL);
      }
    } else {
      if (y_top > WORLDMAP_SIZE) {
        debugmsg("y_top %d x %d", y_top, x);
      }
      for (int y = 0; y < y_top; y++) {
        tiles[x][y].terrain = terrain_from_biome(BIOME_NULL);
      }
      int y_bottom = radial_y_limit_below(radial_points, x);
      if (y_bottom < 0 || y_bottom > WORLDMAP_SIZE) {
        debugmsg("y_bottom %d x %d", y_bottom, x);
      }
      for (int y = y_bottom + 1; y < WORLDMAP_SIZE; y++) {
        tiles[x][y].terrain = terrain_from_biome(BIOME_NULL);
      }
    }
  }
*/
}

int radial_y_limit_above(const std::vector<int> &points, int x)
{
  int low_x = -1, high_x = -1, low_y, high_y;
  int num_points = NUM_RADIAL_POINTS;
  int radius = WORLDMAP_SIZE / 2;
  for (int i = 0; high_x == -1 && i < points.size(); i++) {
    double angle = (PI * i) / num_points;
    int radius_i = radius - points[i];
    int x_i = (angle <= 0.01 ? points[i] : radius - (radius_i * cos(angle)));
    if (x <= 2 && i <= 2) {
      debugmsg("x %d angle %f, radius_i %d, x_i %d", x, angle, radius_i, x_i);
    }
    if (x_i == x) {
      return int(radius_i * sin(angle) + 0.5);
    }
    if (x_i > x) {
      if (i == 0) {
        return -1;
      }
      high_x = x_i;
      high_y = radius - int(radius_i * sin(angle) + 0.5);
    }
    low_x = x_i;
    low_y = radius - int(radius_i * sin(angle) + 0.5);
  }
  return low_y;
  if (low_y == high_y) {
    return low_y;
  }
  int dx = high_x - low_x, dy = high_y - low_y;
  double slope = dx/dy;
  return (low_y + int(slope * (x - low_x) + 0.5));
}

int radial_y_limit_below(const std::vector<int> &points, int x)
{
  std::vector<int> new_points;
  int new_x = WORLDMAP_SIZE - 1 - x;
  int start = new_points.size() / 2;
  for (int i = start; i < NUM_RADIAL_POINTS; i++) {
    new_points.push_back(points[i]);
  }

  int ret = radial_y_limit_above(new_points, new_x);
  return WORLDMAP_SIZE - ret;
}

void draw_island(std::vector<std::vector<int> > &altitude, Point center,
                 int height, int edge_dist)
{
  if (center.x < 0 || center.x >= WORLDMAP_SIZE ||
      center.y < 0 || center.y >= WORLDMAP_SIZE   ) {
    debugmsg("center [%d,%d]", center.x, center.y);
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
