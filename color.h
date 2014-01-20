#ifndef _COLOR_H_
#define _COLOR_H_

#include <string>

void init_colors();

enum nc_color {
c_black = 0,
c_ltgray,
c_red,
c_green,
c_blue,
c_cyan,
c_magenta,
c_brown,
// "Bright" colors
c_dkgray,
c_white,
c_ltred,
c_ltgreen,
c_ltblue,
c_ltcyan,
c_pink,
c_yellow,
c_null
};

long get_color_pair(nc_color fg, nc_color bg);
void extract_colors(long color, long attributes, nc_color &fg, nc_color &bg);

nc_color color_string(std::string id);
std::string color_name(nc_color color);
std::string color_tag (nc_color color);

nc_color hilight (nc_color orig);
nc_color opposite(nc_color orig);
nc_color contrast(nc_color orig);

bool is_bright(nc_color col);
nc_color non_bright(nc_color col);

#endif
