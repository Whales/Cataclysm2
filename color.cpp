#include "color.h"
#if (defined _WIN32 || defined WINDOWS)
  #include "catacurse.h"
#else
  #include <curses.h>
#endif

void init_colors()
{
 start_color();

 init_pair( 1, COLOR_BLACK,   COLOR_BLACK  );
 init_pair( 2, COLOR_WHITE,   COLOR_BLACK  );
 init_pair( 3, COLOR_RED,     COLOR_BLACK  );
 init_pair( 4, COLOR_GREEN,   COLOR_BLACK  );
 init_pair( 5, COLOR_BLUE,    COLOR_BLACK  );
 init_pair( 6, COLOR_CYAN,    COLOR_BLACK  );
 init_pair( 7, COLOR_MAGENTA, COLOR_BLACK  );
 init_pair( 8, COLOR_YELLOW,  COLOR_BLACK  );

 init_pair( 9, COLOR_BLACK,   COLOR_WHITE  );
 init_pair(10, COLOR_WHITE,   COLOR_WHITE  );
 init_pair(11, COLOR_RED,     COLOR_WHITE  );
 init_pair(12, COLOR_GREEN,   COLOR_WHITE  );
 init_pair(13, COLOR_BLUE,    COLOR_WHITE  );
 init_pair(14, COLOR_CYAN,    COLOR_WHITE  );
 init_pair(15, COLOR_MAGENTA, COLOR_WHITE  );
 init_pair(16, COLOR_YELLOW,  COLOR_WHITE  );

 init_pair(17, COLOR_BLACK,   COLOR_RED    );
 init_pair(18, COLOR_WHITE,   COLOR_RED    );
 init_pair(19, COLOR_RED,     COLOR_RED    );
 init_pair(20, COLOR_GREEN,   COLOR_RED    );
 init_pair(21, COLOR_BLUE,    COLOR_RED    );
 init_pair(22, COLOR_CYAN,    COLOR_RED    );
 init_pair(23, COLOR_MAGENTA, COLOR_RED    );
 init_pair(24, COLOR_YELLOW,  COLOR_RED    );

 init_pair(25, COLOR_BLACK,   COLOR_GREEN  );
 init_pair(26, COLOR_WHITE,   COLOR_GREEN  );
 init_pair(27, COLOR_RED,     COLOR_GREEN  );
 init_pair(28, COLOR_GREEN,   COLOR_GREEN  );
 init_pair(29, COLOR_BLUE,    COLOR_GREEN  );
 init_pair(30, COLOR_CYAN,    COLOR_GREEN  );
 init_pair(31, COLOR_MAGENTA, COLOR_GREEN  );
 init_pair(32, COLOR_YELLOW,  COLOR_GREEN  );

 init_pair(33, COLOR_BLACK,   COLOR_BLUE   );
 init_pair(34, COLOR_WHITE,   COLOR_BLUE   );
 init_pair(35, COLOR_RED,     COLOR_BLUE   );
 init_pair(36, COLOR_GREEN,   COLOR_BLUE   );
 init_pair(37, COLOR_BLUE,    COLOR_BLUE   );
 init_pair(38, COLOR_CYAN,    COLOR_BLUE   );
 init_pair(39, COLOR_MAGENTA, COLOR_BLUE   );
 init_pair(40, COLOR_YELLOW,  COLOR_BLUE   );

 init_pair(41, COLOR_BLACK,   COLOR_CYAN   );
 init_pair(42, COLOR_WHITE,   COLOR_CYAN   );
 init_pair(43, COLOR_RED,     COLOR_CYAN   );
 init_pair(44, COLOR_GREEN,   COLOR_CYAN   );
 init_pair(45, COLOR_BLUE,    COLOR_CYAN   );
 init_pair(46, COLOR_CYAN,    COLOR_CYAN   );
 init_pair(47, COLOR_MAGENTA, COLOR_CYAN   );
 init_pair(48, COLOR_YELLOW,  COLOR_CYAN   );

 init_pair(49, COLOR_BLACK,   COLOR_MAGENTA);
 init_pair(50, COLOR_WHITE,   COLOR_MAGENTA);
 init_pair(51, COLOR_RED,     COLOR_MAGENTA);
 init_pair(52, COLOR_GREEN,   COLOR_MAGENTA);
 init_pair(53, COLOR_BLUE,    COLOR_MAGENTA);
 init_pair(54, COLOR_CYAN,    COLOR_MAGENTA);
 init_pair(55, COLOR_MAGENTA, COLOR_MAGENTA);
 init_pair(56, COLOR_YELLOW,  COLOR_MAGENTA);

 init_pair(57, COLOR_BLACK,   COLOR_YELLOW );
 init_pair(58, COLOR_WHITE,   COLOR_YELLOW );
 init_pair(59, COLOR_RED,     COLOR_YELLOW );
 init_pair(60, COLOR_GREEN,   COLOR_YELLOW );
 init_pair(61, COLOR_BLUE,    COLOR_YELLOW );
 init_pair(62, COLOR_CYAN,    COLOR_YELLOW );
 init_pair(63, COLOR_MAGENTA, COLOR_YELLOW );
 init_pair(64, COLOR_YELLOW,  COLOR_YELLOW );

}

long get_color_pair(nc_color fg, nc_color bg)
{
 if (fg == c_null)
  fg = c_ltgray;
 if (bg == c_null)
  bg = c_black;

 int pairnum = int(fg) % 8 + 1 + 8 * ( int(bg) % 8 );
 long ret = COLOR_PAIR(pairnum);
 if (fg >= c_dkgray)
  ret |= A_BOLD;
 if (bg >= c_dkgray)
  ret |= A_BLINK;

 return ret;
}

void extract_colors(long color, long attributes, nc_color &fg, nc_color &bg)
{
// Run through all COLOR_PAIRs -- is there a better way to do this?
 bool found = false;
 for (int i = 1; i <= 64 && !found; i++) {
  if (color == COLOR_PAIR(i)) {
   fg = nc_color((i - 1) % 8);
   bg = nc_color((i - 1) / 8);
   found = true;
  }
 }
 if (!found) {
  fg = c_black;
  bg = c_black;
 }
 if (attributes & A_BOLD)
  fg = nc_color(int(fg) + 8);
 if (attributes & A_BLINK)
  bg = nc_color(int(bg) + 8);
}

nc_color color_string(std::string id)
{
 if (id == "black")
  return c_black;
 if (id == "grey" || id == "gray" || id == "ltgray")
  return c_ltgray;
 if (id == "red")
  return c_red;
 if (id == "green")
  return c_green;
 if (id == "blue")
  return c_blue;
 if (id == "cyan")
  return c_cyan;
 if (id == "magenta")
  return c_magenta;
 if (id == "brown")
  return c_brown;
 if (id == "dkgray")
  return c_dkgray;
 if (id == "white")
  return c_white;
 if (id == "ltred")
  return c_ltred;
 if (id == "ltgreen")
  return c_ltgreen;
 if (id == "ltblue")
  return c_ltblue;
 if (id == "ltcyan")
  return c_ltcyan;
 if (id == "pink")
  return c_pink;
 if (id == "yellow")
  return c_yellow;

 return c_null;
}

std::string color_name(nc_color color)
{
 switch (color) {
  case c_black:   return "Black";
  case c_ltgray:  return "Light Gray";
  case c_red:     return "Red";
  case c_green:   return "Green";
  case c_blue:    return "Blue";
  case c_cyan:    return "Cyan";
  case c_magenta: return "Magenta";
  case c_brown:   return "Brown";
  case c_dkgray:  return "Dark Gray";
  case c_white:   return "White";
  case c_ltred:   return "Light Red";
  case c_ltgreen: return "Light Green";
  case c_ltblue:  return "Light Blue";
  case c_ltcyan:  return "Light Cyan";
  case c_pink:    return "Pink";
  case c_yellow:  return "Yellow";
  case c_null:    return "Unchanged";
  default:        return "???";
 }
 return "???";
}

std::string color_tag(nc_color color)
{
 switch (color) {
  case c_black:   return "black";
  case c_ltgray:  return "ltgray";
  case c_red:     return "red";
  case c_green:   return "green";
  case c_blue:    return "blue";
  case c_cyan:    return "cyan";
  case c_magenta: return "magenta";
  case c_brown:   return "brown";
  case c_dkgray:  return "dkgray";
  case c_white:   return "white";
  case c_ltred:   return "ltred";
  case c_ltgreen: return "ltgreen";
  case c_ltblue:  return "ltblue";
  case c_ltcyan:  return "ltcyan";
  case c_pink:    return "pink";
  case c_yellow:  return "yellow";
  case c_null:    return "/";
  default:        return "/";
 }
 return "/";
}


nc_color hilight(nc_color orig)
{
 if (orig < c_dkgray)
  return nc_color(int(orig) + 8);
 else
  return nc_color(int(orig) - 8);
}

nc_color opposite(nc_color orig)
{
 switch (orig) {
  case c_black:   return c_white;
  case c_ltgray:  return c_dkgray;
  case c_red:     return c_ltcyan;
  case c_green:   return c_pink;
  case c_blue:    return c_yellow;
  case c_cyan:    return c_ltred;
  case c_magenta: return c_ltgreen;
  case c_brown:   return c_ltblue;
  case c_dkgray:  return c_ltgray;
  case c_white:   return c_dkgray; // Don't want to use black
  case c_ltred:   return c_cyan;
  case c_ltgreen: return c_magenta;
  case c_ltblue:  return c_brown;
  case c_ltcyan:  return c_red;
  case c_pink:    return c_green;
  case c_yellow:  return c_blue;
  case c_null:
  default:        return c_null;
 }
 return c_null;
}

nc_color contract(nc_color orig)
{
 switch (orig) {
  case c_black:   return c_white;
  case c_ltgray:  return c_red;
  case c_red:     return c_ltblue;
  case c_green:   return c_ltred;
  case c_blue:    return c_green;
  case c_cyan:    return c_yellow;
  case c_magenta: return c_ltcyan;
  case c_brown:   return c_ltblue;
  case c_dkgray:  return c_red;
  case c_white:   return c_blue;
  case c_ltred:   return c_blue;
  case c_ltgreen: return c_brown;
  case c_ltblue:  return c_brown;
  case c_ltcyan:  return c_magenta;
  case c_pink:    return c_blue;
  case c_yellow:  return c_blue;
  case c_null:  
  default:        return c_null;
 }
 return c_null;
}

bool is_bright(nc_color col)
{
  return (col == c_dkgray   || col == c_white   || col == c_ltred  ||
          col == c_ltgreen  || col == c_ltblue  || col == c_ltcyan ||
          col == c_pink     || col == c_yellow);
}

nc_color non_bright(nc_color col)
{
 switch (col) {
  case c_dkgray:  return c_ltgray; // Don't want to use black
  case c_white:   return c_ltgray;
  case c_ltred:   return c_red;
  case c_ltgreen: return c_green;
  case c_ltblue:  return c_blue;
  case c_ltcyan:  return c_cyan;
  case c_pink:    return c_magenta;
  case c_yellow:  return c_brown;
  default:        return col;
 }
 return c_null;
}
