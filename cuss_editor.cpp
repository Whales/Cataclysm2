#include <fstream>
#include <sstream>
#include "cuss.h"
#include "files.h"
#include "stringfunc.h"

using namespace cuss;

bool starting_window(interface &edited);
void bindings_window(interface &edited);
void elements_window(interface &edited);
void update_elements_window(interface &editor, interface &edited);

void init_interface(interface &edited, std::string name, int x=80, int y=24);
void draw_line(interface &edited, int x1, int y1, int x2, int y2);
void draw_box (interface &edited, int x1, int y1, int x2, int y2);
void temp_line(Window &w,         int x1, int y1, int x2, int y2);
void temp_box (Window &w,         int x1, int y1, int x2, int y2);
void paint(interface &edited, int x, int y);
void fix_lines(interface &edited, std::string name,
               int x1 = -1, int y1 = -1, int x2 = -1, int y2 = -1);
bool is_line(long ch);

void help();
void set_pen_symbol();
void set_pen_fg();
void set_pen_bg();
nc_color pick_color();

enum draw_mode {
DM_NULL = 0,
DM_DRAW,
DM_TYPE,
DM_LINE,
DM_BOX,
DM_DELETE,
DM_ELEMENT,
DM_MOVE_ELE,
DM_RESIZE_ELE,
DM_FIXLINES,
DM_MAX
};

glyph pen;

int main()
{ 
 init_display();

 int sizex = 80, sizey = 24;
 Window w(0, 0, sizex, sizey);

 draw_mode dm = DM_NULL;
 ele_drawing clipboard;

 bool blink = true;
 bool show_coord = false;
 int posx = 0, posy = 0, bufx = -1, bufy = -1;
 pen = glyph('x', c_white, c_black);

 bool really_done = false;
while (!really_done) {
 cuss::interface edited;
 really_done = starting_window(edited);
 bool done = really_done;
 while (!done) {
  if (dm == DM_DRAW)
   paint(edited, posx, posy);

  edited.draw_prototype(&w);
  glyph gl_orig = w.glyphat(posx, posy);
  element* sel = edited.selected();

  if (blink) {
   if (dm == DM_MOVE_ELE)
     w.putglyph(sel->posx, sel->posy, glyph(LINE_OXXO, c_pink, c_black));
   else if (dm == DM_RESIZE_ELE)
     w.putglyph(sel->posx + sel->sizex - 1, sel->posy + sel->sizey - 1,
                glyph(LINE_XOOX, c_pink, c_black));
   else {
     if (gl_orig == pen) {
       glyph tmp(pen.symbol, pen.fg, hilight(pen.bg));
       w.putglyph(posx, posy, tmp);
     } else
       w.putglyph(posx, posy, pen);
   }
  }

  if (dm == DM_LINE)
   temp_line(w, bufx, bufy, posx, posy);
  if (dm == DM_BOX || dm == DM_ELEMENT || dm == DM_DELETE || dm == DM_FIXLINES)
   temp_box(w, bufx, bufy, posx, posy);


  if (show_coord) {
    std::stringstream coord_ss;
    coord_ss << posx << "," << posy;
    w.putstr(74, 23, c_magenta, c_black, coord_ss.str());
  }
  w.refresh();
  timeout((blink ? 300 : 150));
  long ch = getch();
  timeout(-1);

  if (ch == ERR)
   blink = !blink;
  else {
   blink = true;
   if (dm == DM_TYPE) {
    if (ch == '\n') {
     if (bufx != -1 && bufy != -1 && posy < sizey - 1) {
      posx = bufx;
      posy++;
     }
    } else if (ch == KEY_ESC)
     dm = DM_NULL;
    else if (ch == KEY_LEFT) {
     if (posx > 0) posx--;
    } else if (ch == KEY_RIGHT) {
     if (posx < sizex - 1) posx++;
    } else if (ch == KEY_UP) {
     if (posy > 0) posy--;
    } else if (ch == KEY_DOWN) {
     if (posy < sizey - 1) posy++;
    } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
     edited.set_data("BG", glyph(-1, c_black, c_black), posx, posy);
     if (posx > 0) posx--;
    } else {
     pen.symbol = ch;
     edited.set_data("BG", pen, posx, posy);
     pen.symbol = '_';
     if (posx < sizex - 1) posx++;
    }
   } else { // Not typing.
 
    int movex = 0, movey = 0;
    if (ch == 'y' || ch == 'h' || ch == 'b' || ch == '7' || ch == '4' ||
        ch == '1' || ch == KEY_LEFT)
     movex = -1;
    if (ch == 'u' || ch == 'l' || ch == 'n' || ch == '9' || ch == '6' ||
        ch == '3' || ch == KEY_RIGHT)
     movex = 1;
    if (ch == 'y' || ch == 'k' || ch == 'u' || ch == '7' || ch == '8' ||
        ch == '9' || ch == KEY_UP)
     movey = -1;
    if (ch == 'b' || ch == 'j' || ch == 'n' || ch == '1' || ch == '2' ||
        ch == '3' || ch == KEY_DOWN)
     movey = 1;

    if (movex != 0 || movey != 0) {

     if (dm == DM_MOVE_ELE && sel) {
      sel->posx += movex;
      if (sel->posx < 0)
       sel->posx = 0;
      if (sel->posx + sel->sizex - 1 >= sizex)
       sel->posx = sizex - sel->sizex;
      if (sel->posx + sel->sizex - 1 >= edited.sizex)
       sel->posx = edited.sizex - sel->sizex;

      sel->posy += movey;
      if (sel->posy < 0)
       sel->posy = 0;
      if (sel->posy + sel->sizey - 1 >= sizey)
       sel->posy = sizey - sel->sizey;
      if (sel->posy + sel->sizey - 1 >= edited.sizey)
       sel->posy = edited.sizey - sel->sizey;

     } else if (dm == DM_RESIZE_ELE && sel) {
      sel->sizex += movex;
      if (sel->sizex < 1)
       sel->sizex = 1;
      if (sel->posx + sel->sizex - 1 >= sizex)
       sel->sizex = sizex - sel->posx;
      if (sel->posx + sel->sizex - 1 >= edited.sizex)
       sel->sizex = edited.sizex - sel->posx;

      sel->sizey += movey;
      if (sel->sizey < 1)
       sel->sizey = 1;
      if (sel->posy + sel->sizey - 1 >= sizey)
       sel->sizey = sizey - sel->posy;
      if (sel->posy + sel->sizey - 1 >= edited.sizey)
       sel->sizey = edited.sizey - sel->posy;

     } else { // Normal cursor movement
      posx += movex;
      if (posx < 0) posx = 0;
      if (posx >= sizex) posx = sizex - 1;
      if (posx >= edited.sizex) posx = edited.sizex - 1;
      posy += movey;
      if (posy < 0) posy = 0;
      if (posy >= sizey) posy = sizey - 1;
      if (posy >= edited.sizey) posy = edited.sizey - 1;
     }

    } else if (ch == 'g') {
     posy = 0;

    } else if (ch == 'G') {
     posy = sizey - 1;

    } else if (ch == '^' || ch == '0') {
     posx = 0;

    } else if (ch == '$') {
     posx = sizex - 1;

    } else if (ch == '?') {
     help();

    } else if (ch == '!') {
     show_coord = !show_coord;

    } else if (ch == '-') {
     elements_window(edited);

    } else if (ch == '_') {
     bindings_window(edited);

    } else if (ch == '<') {
     sel = edited.select_last(true);

    } else if (ch == '>') {
     sel = edited.select_next(true);

    } else if (ch == '{' && sel) {
     sel->align = ALIGN_LEFT;

    } else if (ch == '}' && sel) {
     sel->align = ALIGN_RIGHT;

    } else if (ch == '(' && sel) {
     sel->v_align = ALIGN_TOP;

    } else if (ch == ')' && sel) {
     sel->v_align = ALIGN_BOTTOM;

    } else if (ch == '|' && sel) {
     sel->align = ALIGN_CENTER;

    } else if (ch == 'm' && sel) {
     dm = DM_MOVE_ELE;
     bufx = sel->posx; bufy = sel->posy;

    } else if (ch == 'r' && sel) {
     dm = DM_RESIZE_ELE;
     bufx = sel->sizex; bufy = sel->sizey;

    } else if (ch == 'c' || ch == 'C') {
     pen = gl_orig;

    } else if (ch == '\'') {
     set_pen_symbol();

    } else if (ch == '"') {
     switch (pen.symbol) {
      case LINE_XXXX: pen.symbol = LINE_XOXO; break;
      case LINE_XOXO: pen.symbol = LINE_OXOX; break;
      case LINE_OXOX: pen.symbol = LINE_XXOO; break;
      case LINE_XXOO: pen.symbol = LINE_OXXO; break;
      case LINE_OXXO: pen.symbol = LINE_OOXX; break;
      case LINE_OOXX: pen.symbol = LINE_XOOX; break;
      case LINE_XOOX: pen.symbol = LINE_XXOX; break;
      case LINE_XXOX: pen.symbol = LINE_XXXO; break;
      case LINE_XXXO: pen.symbol = LINE_OXXX; break;
      case LINE_OXXX: pen.symbol = LINE_XOXX; break;
      case LINE_XOXX: pen.symbol = LINE_XXXX; break;
      default:        pen.symbol = LINE_XXXX; break;
     }
      
    } else if (ch == '[') {
     set_pen_fg();

    } else if (ch == ']') {
     set_pen_bg();

    } else if (ch == 'i' || ch == 'I') {
     dm = DM_TYPE;
     pen.symbol = '_';
     bufx = posx; bufy = posy;

    } else if (ch == ';') {
     bufx = posx; bufy = posy;
     dm = DM_LINE;

    } else if (ch == ':') {
     bufx = posx; bufy = posy;
     dm = DM_BOX;

    } else if (ch == 'd' || ch == 'D') {
     bufx = posx; bufy = posy;
     dm = DM_DELETE;

    } else if (ch == 'p' || ch == 'P') {
     if (!clipboard.drawing.empty()) {
      for (std::map<Point, glyph>::iterator it = clipboard.drawing.begin();
           it != clipboard.drawing.end(); it++) {
       edited.set_data("BG", it->second, it->first.x + posx, it->first.y+posy);
      }
     } else {
      debugmsg("paste was empty");
     }

    } else if (ch == ',') {
     dm = DM_DRAW;

    } else if (ch == '.') {
     paint(edited, posx, posy);

    } else if (ch == 'x') {
     if (sel && (popup_getkey("Really delete %s?", sel->name.c_str()) == 'Y')) {
      edited.erase_element(sel->name);
      sel = NULL;
     } else
      edited.set_data("BG", glyph(-1, c_black, c_black), posx, posy);

    } else if (ch == '/') {
     bufx = posx; bufy = posy;
     dm = DM_FIXLINES;

    } else if (ch == 'S' || ch == 's') {
     char quitconf = popup_getkey("Quit & Save?");
     if (quitconf == 'y' || quitconf == 'Y' || quitconf == 's' ||
         quitconf == 'S')
      done = true;
     if (ch == 'S')
      really_done = true;

    } else if (ch == '\n') {
     switch (dm) {
      case DM_NULL:
       dm = DM_ELEMENT;
       bufx = posx;
       bufy = posy;
       break;
 
      case DM_DRAW:
       if (bufx != -1 && bufy != -1 && posy < sizey - 1) {
        posx = bufx;
        posy++;
       }
       break;
 
      case DM_LINE:
       draw_line(edited, bufx, bufy, posx, posy);
       dm = DM_NULL;
       bufx = -1;
       bufy = -1;
       break;
 
      case DM_BOX:
       draw_box(edited, bufx, bufy, posx, posy);
       dm = DM_NULL;
       bufx = -1;
       bufy = -1;
       break;

      case DM_FIXLINES:
       fix_lines(edited, "BG", bufx, bufy, posx, posy);
       dm = DM_NULL;
       bufx = -1;
       bufy = -1;
       break;

      case DM_DELETE:
       clipboard.clear_data();
       clipboard.sizex = posx - bufx + 1;
       clipboard.sizey = posy - bufy + 1;
       for (int x = bufx; x <= posx; x++) {
        for (int y = bufy; y <= posy; y++) {
         Point p(x, y);
         element* tmp = edited.find_by_name("BG");
         if (!tmp) {
          debugmsg("Couldn't find BG for cutting");
         }
         ele_drawing* bg = static_cast<ele_drawing*>(tmp);
         if (bg->drawing.count(p)) {
          //debugmsg("added %d %d", p.x, p.y);
          clipboard.set_data( bg->drawing[p], x - bufx, y - bufy );
          //debugmsg("%d", clipboard.drawing.size());
         }
         edited.set_data("BG", glyph(-1, c_black, c_black), x, y);
        }
       }
       dm = DM_NULL;
       bufx = -1;
       bufy = -1;
       break;
 
      case DM_ELEMENT: {
       element_type type = ELE_NULL;
       dm = DM_NULL;
       switch (menu("Element type:", "Drawing", "Text", "List", "Text Entry",
                    "Number", "Drop-down Menu", "Cancel", NULL)) {
        case 0: type = ELE_DRAWING;   break;
        case 1: type = ELE_TEXTBOX;   break;
        case 2: type = ELE_LIST;      break;
        case 3: type = ELE_TEXTENTRY; break;
        case 4: type = ELE_NUMBER;    break;
        case 5: type = ELE_MENU;      break;
        case 7: type = ELE_NULL;      break;
       }
       if (type != ELE_NULL) {
        int sizex = posx - bufx + 1;
        int sizey = posy - bufy + 1;
        int x1, y1;
        if (bufx < posx)
         x1 = bufx;
        else
         x1 = posx;
        if (bufy < posy)
         y1 = bufy;
        else
         y1 = posy;
        std::string name = string_input_popup("Name element:");
        char selch = popup_getkey("Selectable?");
        bool selectable = (selch == 'y' || selch == 'Y');
        edited.add_element(type, name, x1, y1, sizex, sizey, selectable);
        edited.draw(&w);
        bufx = -1;
        bufy = -1;
       }
      } break;

      case DM_MOVE_ELE:
      case DM_RESIZE_ELE:
       dm = DM_NULL;
       break;
     } // switch (dm)
    } else if (ch == KEY_ESC) {
     if (sel) {
      if (dm == DM_MOVE_ELE && bufx != -1 && bufy != -1) {
       sel->posx = bufx;
       sel->posy = bufy;
      }
      if (dm == DM_RESIZE_ELE && bufx != -1 && bufy != -1) {
       sel->sizex = bufx;
       sel->sizey = bufy;
      }
      bufx = -1;
      bufy = -1;
      edited.select_none();
      sel = edited.selected();
     } else {
      edited.select_none();
      sel = edited.selected();
     }

     if (dm != DM_NULL) {
      dm = DM_NULL;
      if (bufx != -1 && bufy != -1) {
       posx = bufx;
       posy = bufy;
       bufx = -1;
       bufy = -1;
      }
     }
    }
   } // Not typing
  } // ch != ERR

 } // while !done
  if (!edited.name.empty()) {
    std::ofstream fout;
    std::stringstream foutname;
    foutname << "cuss/" << edited.name << ".cuss";
    std::string fname = foutname.str();
    fout.open(fname.c_str());
    if (fout.is_open()) {
      fout << edited.save_data();
      fout.close();
    } else
      popup("Couldn't open %s for saving", fname.c_str());
  }
}


 endwin();
 return 0;
}

bool starting_window(interface &edited)
{
 Window w_start(0, 0, 80, 24);
 cuss::interface i_start;
 cuss::interface selected;

 if (!i_start.load_from_file("cuss/editor/i_start.cuss"))
  return false;

 i_start.set_data("list_interfaces", files_in("cuss", "cuss"));
 std::string selname = i_start.get_str("list_interfaces");

 if (selname != "") {
  std::stringstream filename;
  filename << "cuss/" << selname;
  selected.load_from_file(filename.str());
  i_start.set_data("list_elements", selected.element_names());
  i_start.set_data("num_x", selected.sizex);
  i_start.set_data("num_y", selected.sizey);
 }
 i_start.select("list_interfaces");

 bool done = false;
 while (!done) {
  i_start.draw(&w_start);
  w_start.refresh();
  long ch = getch();
  if (ch == 'j' || ch == '2' || ch == KEY_DOWN) {
   i_start.add_data("list_interfaces",  1);
   selname = i_start.get_str("list_interfaces");
   if (selname != "") {
    std::stringstream filename;
    filename << "cuss/" << selname;
    selected.load_from_file(filename.str());
    i_start.set_data("list_elements", selected.element_names());
    i_start.set_data("num_x", selected.sizex);
    i_start.set_data("num_y", selected.sizey);
   }
  }
  if (ch == 'k' || ch == '8' || ch == KEY_UP) {
   i_start.add_data("list_interfaces", -1);
   selname = i_start.get_str("list_interfaces");
   if (selname != "") {
    std::stringstream filename;
    filename << "cuss/" << selname;
    selected.load_from_file(filename.str());
    i_start.set_data("list_elements", selected.element_names());
    i_start.set_data("num_x", selected.sizex);
    i_start.set_data("num_y", selected.sizey);
   }
  }
  if (ch == 'l' || ch == 'L' || ch == '\n') {
   done = true;
   std::stringstream filename;
   filename << "cuss/" << i_start.get_str("list_interfaces");
   edited.load_from_file(filename.str());
  }
  if (ch == 'c' || ch == 'C') {
    done = true;
    std::stringstream filename;
    std::string name = i_start.get_str("list_interfaces");
    filename << "cuss/" << name;
    name = name.substr(0, name.length() - 5); // Strip off ".cuss"
    edited.load_from_file(filename.str());
    edited.name = string_edit_popup(name, "Name: ");
  }
  if (ch == 'n' || ch == 'N') {
   std::string name = string_input_popup("Name: ");
   if (name != "") {
    int x = int_input_popup("X dim:"), y = int_input_popup("Y dim:");
    if (x > 0 && y > 0) {
      done = true;
      init_interface(edited, name, x, y);
    }
   }
  }
  if (ch == 'q' || ch == 'Q')
   return true;
 }
 return false;
}

void bindings_window(interface &edited)
{
 Window w_bindings(0, 0, 80, 24);

 cuss::interface i_bindings;
 std::vector<long> bind_keys; // For temp use only
 if (!i_bindings.load_from_file("cuss/editor/i_bindings.cuss")) {
  return;
 }
 i_bindings.set_data("e_list_bindings", edited.binding_list());
 i_bindings.select("e_list_bindings");

 bool done = false;
 while (!done) {
  i_bindings.draw(&w_bindings);
  long ch = getch();

  switch (ch) {
   case 'j':
   case 'J':
   case KEY_DOWN:
   case '2':
    i_bindings.add_data("e_list_bindings", 1);
    break;

   case 'k':
   case 'K':
   case KEY_UP:
   case '8':
    i_bindings.add_data("e_list_bindings", -1);
    break;

   case 'a':
   case 'A': {
// TODO: Insert add binding code
    long key = popup_getkey("Key to bind:");
    binding *used = edited.bound_to(key);
    if (used) {
     std::stringstream errormes;
     errormes << "<c=red>ERROR: " << key_name(key) << " is already bound to " <<
                 action_name(used->act) << "!";
     i_bindings.set_data("e_text_error", errormes.str());
    } else {
     int a = 0, b = 0;

     std::vector<std::string> action_options;
     for (int i = 0; i < ACT_MAX; i++)
      action_options.push_back( action_name( action_id(i) ) );
     action_options.push_back("Cancel");
     action_id act_sel = action_id( menu_vec("Action:", action_options) );

     std::string ele_sel;
     if (action_needs_element(act_sel)) {
      std::vector<std::string> ele_options = edited.element_names();
      ele_options[0] = "<S>"; // Replace BG with <S>
      ele_options.push_back("Cancel");
      ele_sel = ele_options[ menu_vec("Element:", ele_options) ];
     }

     if (act_sel != ACT_MAX && ele_sel != "Cancel") {
      switch (act_sel) {
       case ACT_SET_COLORS: {
        i_bindings.add_data("e_color_fg", "Unchanged");
        i_bindings.add_data("e_color_bg", "Unchanged");
        for (int i = c_black; i < c_null; i++) {
         i_bindings.add_data("e_color_fg", color_name( nc_color(i) ));
         i_bindings.add_data("e_color_bg", color_name( nc_color(i) ));
        }
        i_bindings.set_selectable("e_color_fg", true);
        i_bindings.set_selectable("e_color_bg", true);
        i_bindings.set_selectable("e_list_bindings", false);
        i_bindings.select("e_color_fg");
         
        bool done = false;
        while (!done) {
         i_bindings.draw(&w_bindings);
         long ch = getch();
         if (ch == 'j' || ch == 'J' || ch == '2' || ch == KEY_DOWN)
          i_bindings.selected()->add_data(1);
         if (ch == 'k' || ch == 'K' || ch == '8' || ch == KEY_UP)
          i_bindings.selected()->add_data(-1);
         if (ch == '\t')
          i_bindings.select_next();
         if (ch == '\n')
          done = true;
        }
        a = i_bindings.get_int("e_color_fg");
        b = i_bindings.get_int("e_color_bg");
        if (a == 0)
         a = c_null;
        else
         a--;
        if (b == 0)
         b = c_null;
        else
         b--;
        i_bindings.set_selectable("e_color_fg", false);
        i_bindings.set_selectable("e_color_bg", false);
        i_bindings.clear_data("e_color_fg");
        i_bindings.clear_data("e_color_bg");
        i_bindings.set_selectable("e_list_bindings", true);
        i_bindings.select("e_list_bindings");
       } break;

       case ACT_TRANSLATE:
        a = popup_getkey("From:");
        b = popup_getkey("To:");
        break;

       case ACT_SCROLL:
        a = int_input_popup("Amount:");
      }
      edited.add_binding(key, act_sel, ele_sel, a, b);
      bind_keys.push_back(key);
      i_bindings.set_data("e_list_bindings", edited.binding_list());
     } // !Canceled
    } // !used

   } break;

   case 'd':
   case 'D': {
    int selindex = i_bindings.get_int("e_list_bindings");
    if (selindex > 0 && selindex < bind_keys.size()) {
     long key = bind_keys[selindex];
     edited.rem_binding(key);
     bind_keys.erase( bind_keys.begin() + selindex );
    }
    i_bindings.set_data("e_list_bindings", edited.binding_list());
   } break;

   case 'c':
   case 'C': {
    std::vector<std::string> eles = edited.element_names();
    std::vector<std::string> options;
    for (int i = 0; i < ACT_MAX; i++) {
     if (edited.has_bindings_for( action_id(i) ))
      options.push_back( action_name( action_id(i) ) );
    }
    for (int i = 0; i < eles.size(); i++) {
     if (edited.has_bindings_for( eles[i] ))
      options.push_back(eles[i]);
    }
    options.push_back("All");
    int sel = menu_vec("Group to delete:", options);
    if (sel == options.size() - 1) {
     char confirm = popup_getkey("Really delete all?");
     if (confirm == 'y' || confirm == 'Y')
      edited.rem_all_bindings();
    } else if (sel < ACT_MAX)
     edited.rem_all_bindings( action_id(sel) );
    else
     edited.rem_all_bindings(options[sel]);

    i_bindings.set_data("e_list_bindings", edited.binding_list());
   } break;

   case KEY_ESC:
    done = true;
    break;
  } // switch (ch)
 } // while (!done)
}

void elements_window(interface &edited)
{
 Window w_elements(0, 0, 80, 24);

 cuss::interface i_ele;
 if (!i_ele.load_from_file("cuss/editor/i_elements.cuss")) {
  return;
 }

 i_ele.set_data("e_elelist", edited.element_names());
 element* cur = i_ele.select("e_elelist");
 element* selected = edited.select("BG");

 bool done = false;
/*
 std::string value_str;
 std::vector<std::string> list_array;
 i_ele.ref_data("e_list_values",   &list_array);
 i_ele.ref_data("e_value_setting", &value_str);
*/
 update_elements_window(i_ele, edited);

 do {

  i_ele.draw(&w_elements);

  long ch = getch();

  if (ch == KEY_ESC)
   done = true;
  else if (cur && cur->name == "e_elelist") {
   if (ch == 'j' || ch == 'J' || ch == '2' || ch == KEY_DOWN) {
    i_ele.add_data("e_elelist", 1);
    selected = edited.select(cur->get_str());
    update_elements_window(i_ele, edited);
   }
   if (ch == 'k' || ch == 'K' || ch == '8' || ch == KEY_UP) {
    i_ele.add_data("e_elelist", -1);
    selected = edited.select(cur->get_str());
    update_elements_window(i_ele, edited);
   }
   if (cur && ch == '>') {
    int pos = i_ele.get_int("e_elelist");
    if (edited.move_element_up(cur->get_str()))
     i_ele.set_data("e_elelist", edited.element_names());
    i_ele.set_data("e_elelist", pos + 1);
   }
   if (cur && ch == '<') {
    int pos = i_ele.get_int("e_elelist");
    if (edited.move_element_down(cur->get_str()))
     i_ele.set_data("e_elelist", edited.element_names());
    i_ele.set_data("e_elelist", pos - 1);
   }
   if (ch == '\t')
    cur = i_ele.select_next();
   if (ch == ']')
    cur = i_ele.select_next();
   if (ch == '[')
    cur = i_ele.select_last();
   if (selected && ch == '{')
    selected->align = ALIGN_LEFT;
   if (selected && ch == '}')
    selected->align = ALIGN_RIGHT;
   if (selected && ch == '(')
    selected->v_align = ALIGN_TOP;
   if (selected && ch == ')')
    selected->v_align = ALIGN_BOTTOM;
   if (selected && ch == '|')
    selected->align = ALIGN_CENTER;
   if (selected && ch == '!' )
    selected->selectable = !(selected->selectable);

  } else { // else if (cur && cur->name == "e_elelist")
   if (selected && ch == '{')
    selected->align = ALIGN_LEFT;
   else if (selected && ch == '}')
    selected->align = ALIGN_RIGHT;
   else if (selected && ch == '(')
    selected->v_align = ALIGN_TOP;
   else if (selected && ch == ')')
    selected->v_align = ALIGN_BOTTOM;
   else if (selected && ch == '|')
    selected->align = ALIGN_CENTER;
   else if (ch == '!' )
    selected->selectable = !(selected->selectable);
   else if (ch == '\t') {
    cur = i_ele.select_next();
    if (cur && cur->name == "e_list_values")
     cur->set_data(99999); // Scroll to bottom
   } else if (cur && selected && cur->name == "e_elename") {
    if (ch == ']')
     cur = i_ele.select_next();
    else if (ch == '[')
     cur = i_ele.select_last();
    else if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) &&
             selected) {
     if (!selected->name.empty()) {
      selected->name = selected->name.substr(0, selected->name.size() - 1);
      i_ele.set_data("e_elelist", edited.element_names());
     }
    } else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9') || ch == '_') {
     selected->name += ch;
     i_ele.set_data("e_elelist", edited.element_names());
    }

   } else if (cur && cur->name == "e_value_setting") {
    if (selected && selected->type() == ELE_NUMBER) { // Editing a number
     if (ch == 'k' || ch == 'K' || ch == KEY_UP)
      selected->add_data(1);
     if (ch == 'j' || ch == 'J' || ch == KEY_DOWN)
      selected->add_data(-1);
     if (ch == 'x' || ch == 'X')
      selected->set_data(0);
     if (ch == KEY_BACKSPACE || ch == 127 || ch == 8)
      selected->set_data( selected->get_int() / 10 );
     if (ch >= '0' && ch <= '9')
      selected->set_data( (ch - '0') + selected->get_int() * 10);
    } else { // Editing a string
     std::string strtmp;
     if (selected->type() == ELE_MENU) {
      ele_menu* e_m = static_cast<ele_menu*>(selected);
      strtmp = e_m->title;
     } else
      strtmp = selected->get_str();
     if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8)) {
      if (!strtmp.empty())
       strtmp = strtmp.substr(0, strtmp.size() - 1);
     } else
      strtmp += ch;
     selected->set_data(strtmp);
    }

   } else if (cur && cur->name == "e_list_values") {
    if (selected && selected->type() == ELE_TEXTBOX) {
     std::string alltext = selected->get_str();

     if (ch == KEY_UP)
      i_ele.add_data("e_list_values", -1);

     else if (ch == KEY_DOWN)
      i_ele.add_data("e_list_values", 1);

     else if  (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
      if (!alltext.empty()) {
       alltext = alltext.substr(0, alltext.size() - 1);
       selected->set_data(alltext);
       if (cur->sizex >= selected->sizex)
        i_ele.set_data("e_list_values", selected->get_str_list());
       else
        i_ele.set_data("e_list_values", break_into_lines(alltext, cur->sizex));
       i_ele.set_data("e_list_values", 99999); // Scroll to the bottom
      }

     } else {
      alltext += ch;
      selected->set_data(alltext);
      if (cur->sizex >= selected->sizex)
       i_ele.set_data("e_list_values", selected->get_str_list());
      else
       i_ele.set_data("e_list_values", break_into_lines(alltext, cur->sizex));
      i_ele.set_data("e_list_values", 99999); // Scroll to the bottom
     }

    } else if (selected &&
               (selected->type() == ELE_LIST || selected->type() == ELE_MENU)) {
     int pos = i_ele.get_int("e_list_values");
     std::vector<std::string> *list_array = NULL;
     if (selected->type() == ELE_LIST) {
      ele_list* e_l = static_cast<ele_list*>(selected);
      list_array = e_l->list;
     } else if (selected->type() == ELE_MENU) {
      ele_menu* e_m = static_cast<ele_menu*>(selected);
      list_array = e_m->list;
     }

     if (ch == 'j' || ch == 'J' || ch == '2' || ch == KEY_DOWN)
      i_ele.add_data("e_list_values", 1);

     if (ch == 'k' || ch == 'K' || ch == '8' || ch == KEY_UP)
      i_ele.add_data("e_list_values", -1);

     if (ch == 'a' || ch == 'A') {
      selected->add_data( string_input_popup("New item:") );
      i_ele.set_data("e_list_values", 99999); // move to end of list
     }

     if (list_array && pos >= 0 && pos < list_array->size() &&
         (ch == 'e' || ch == 'E'))
      (*list_array)[pos] = string_edit_popup((*list_array)[pos], "");

     if (list_array && pos >= 0 && pos < list_array->size() &&
         (ch == 'd' || ch == 'D')) {
      list_array->erase( list_array->begin() + pos );
      i_ele.set_data("e_list_values", pos);
     }

     if (ch == '>' && list_array && pos >= 0 && pos < list_array->size() - 1) {
      std::string buff = (*list_array)[pos];
      (*list_array)[pos] = (*list_array)[pos + 1];
      (*list_array)[pos + 1] = buff;
      i_ele.set_data("e_list_values", pos + 1);
     }

     if (ch == '<' && list_array && pos >= 1 && pos < list_array->size()) {
      std::string buff = (*list_array)[pos];
      (*list_array)[pos] = (*list_array)[pos - 1];
      (*list_array)[pos - 1] = buff;
      i_ele.set_data("e_list_values", pos - 1);
     }
    }
   }
  }
 } while (!done);
}


void update_elements_window(interface &editor, interface &edited)
{
 element* selected = edited.selected();

 if (selected) {
  editor.set_data("e_elename", selected->name);
  editor.set_data("e_eletype", element_type_name(selected->type()) );
  editor.set_data("e_editable", (selected->selectable ? "Yes" : "No"));
  if (selected->align == ALIGN_LEFT)
   editor.set_data("e_alignment", "Left");
  else if (selected->align == ALIGN_RIGHT)
   editor.set_data("e_alignment", "Right");
  else if (selected->align == ALIGN_CENTER)
   editor.set_data("e_alignment", "Center");
  if (selected->v_align == ALIGN_TOP)
   editor.set_data("e_v_alignment", "Top");
  if (selected->v_align == ALIGN_BOTTOM)
   editor.set_data("e_v_alignment", "Bottom");
// Set up the "basic value" area
  switch (selected->type()) {

   case ELE_TEXTENTRY: {
    ele_textentry* e_te = static_cast<ele_textentry*>(selected);
    editor.set_data("e_value_name", "Default:");
    editor.set_selectable("e_value_setting", true);
    editor.ref_data("e_value_setting", e_te->text);
   } break;

   case ELE_NUMBER:
    editor.set_data("e_value_name", "Default:");
    editor.self_reference("e_value_setting");
    editor.set_selectable("e_value_setting", true);
   break;

   case ELE_MENU: {
    ele_menu* e_men = static_cast<ele_menu*>(selected);
    editor.set_data("e_value_name", "Title:");
    editor.ref_data("e_value_setting", &(e_men->title));
    editor.set_selectable("e_value_setting", true);
   } break;

   default:
    editor.set_data("e_value_name", "");
    editor.set_data("e_value_setting", "");
    editor.set_selectable("e_value_setting", false);
    editor.self_reference("e_value_setting");
    editor.clear_data("e_value_setting");
  }
// Set up the list area
  switch (selected->type()) {

   case ELE_TEXTBOX: {
    ele_textbox* e_tb = static_cast<ele_textbox*>(selected);
    editor.set_data("e_list_name", "Contents:");
    editor.set_data("e_list_instructions", "(Just type to add)");
    editor.self_reference("e_list_values");
    editor.set_data("e_list_values", e_tb->get_str_list());
    editor.set_selectable("e_list_values", true);
   } break;

   case ELE_LIST:{
    ele_list* e_list = static_cast<ele_list*>(selected);
    editor.set_data("e_list_name", "Options:");
    editor.set_data("e_list_instructions",
              "<c=blue>A<c=/>dd <c=blue>D<c=/>elete <c=blue>E<c=/>dit");
    editor.ref_data("e_list_values", e_list->list);
    editor.set_selectable("e_list_values", true);
   } break;

   case ELE_MENU: {
    ele_menu* e_menu = static_cast<ele_menu*>(selected);
    editor.set_data("e_list_name", "Options:");
    editor.set_data("e_list_instructions",
              "<c=blue>A<c=/>dd <c=blue>D<c=/>elete <c=blue>E<c=/>dit");
    editor.ref_data("e_list_values", e_menu->list);
    editor.set_selectable("e_list_values", true);
   } break;

   default:
    editor.clear_data("e_list_name");
    editor.clear_data("e_list_instructions");
    editor.clear_data("e_list_values");
    editor.self_reference("e_list_values");
    editor.clear_data("e_list_values");
    editor.set_selectable("e_list_values", false);
    break;
  }
 }
}


void init_interface(interface &edited, std::string name, int x, int y)
{
 std::stringstream filename;
 filename << "cuss/" << name << ".cuss";
 std::ifstream fin;
 fin.open(filename.str().c_str());
 if (fin.is_open()) {
  edited.load_data(fin);
  fin.close();
 } else {
  edited.name = name;
  edited.sizex = x;
  edited.sizey = y;
  edited.add_element(ELE_DRAWING, "BG", 0, 0, x, y, false);
 }
}

void draw_line(interface &edited, int x1, int y1, int x2, int y2)
{
 int xdir = 0, ydir = 0;
 long sym = '*';
 if (x1 == x2 && y1 == y2)
  return;

 if (x1 == x2) {
  sym = LINE_XOXO;
  ydir = (y2 > y1 ? 1 : -1);
 } else if (y1 == y2) {
  sym = LINE_OXOX;
  xdir = (x2 > x1 ? 1 : -1);
 } else if (y2 - y1 == x2 - x1) {
  sym = '\\';
  xdir = (x2 > x1 ? 1 : -1);
  ydir = (y2 > y1 ? 1 : -1);
 } else if (y2 - y1 == x1 - x2) {
  sym = '/';
  xdir = (x2 > x1 ? 1 : -1);
  ydir = (y2 > y1 ? 1 : -1);
 } else {
  debugmsg("No Bresenham here!");
  return;
 }

 int x = x1, y = y1;
 glyph gl(sym, pen.fg, pen.bg);
 do {
  edited.set_data("BG", gl, x, y);
  x += xdir;
  y += ydir;
 } while (x != x2 || y != y2);
 edited.set_data("BG", gl, x, y);
}

void temp_line(Window &w, int x1, int y1, int x2, int y2)
{
 int xdir = 0, ydir = 0;
 long sym = '*';
 if (x1 == x2 && y1 == y2)
  return;

 if (x1 == x2) {
  sym = LINE_XOXO;
  ydir = (y2 > y1 ? 1 : -1);
 } else if (y1 == y2) {
  sym = LINE_OXOX;
  xdir = (x2 > x1 ? 1 : -1);
 } else if (y2 - y1 == x2 - x1) {
  sym = '\\';
  xdir = (x2 > x1 ? 1 : -1);
  ydir = (y2 > y1 ? 1 : -1);
 } else if (y2 - y1 == x1 - x2) {
  sym = '/';
  xdir = (x2 > x1 ? 1 : -1);
  ydir = (y2 > y1 ? 1 : -1);
 } else {
  return;
 }

 int x = x1, y = y1;
 glyph gl(sym, c_magenta, c_blue);
 do {
  w.putglyph(x, y, gl);
  x += xdir;
  y += ydir;
 } while (x != x2 || y != y2);
 w.putglyph(x, y, gl);
}

void draw_box (interface &edited, int x1, int y1, int x2, int y2)
{
 if (x1 == x2 && y1 == y2)
  return;

 if (x1 > x2) {
  int buf = x2;
  x2 = x1;
  x1 = buf;
 }
 if (y1 > y2) {
  int buf = y2;
  y2 = y1;
  y1 = buf;
 }

 if (is_line(pen.symbol)) {
  if (x1 + 1 < x2) {
   draw_line(edited, x1, y1, x2, y1);
   draw_line(edited, x1, y2, x2, y2);
  }
  if (y1 + 1 < y2) {
   draw_line(edited, x1, y1, x1, y2);
   draw_line(edited, x2, y1, x2, y2);
  }

  edited.set_data("BG", glyph(LINE_OXXO, pen.fg, pen.bg), x1, y1);
  edited.set_data("BG", glyph(LINE_OOXX, pen.fg, pen.bg), x2, y1);
  edited.set_data("BG", glyph(LINE_XXOO, pen.fg, pen.bg), x1, y2);
  edited.set_data("BG", glyph(LINE_XOOX, pen.fg, pen.bg), x2, y2);

 } else { // Not a line-drawing box; so draw & fill
  for (int x = x1; x <= x2; x++) {
   for (int y = y1; y <= y2; y++)
     paint(edited, x, y);
  }
 }
}

void temp_box (Window &w, int x1, int y1, int x2, int y2)
{
 if (x1 == x2 && y1 == y2)
  return;

 if (x1 > x2) {
  int buf = x2;
  x2 = x1;
  x1 = buf;
 }
 if (y1 > y2) {
  int buf = y2;
  y2 = y1;
  y1 = buf;
 }

 w.putglyph(x1, y1, glyph(LINE_OXXO, c_magenta, c_blue));
 w.putglyph(x2, y1, glyph(LINE_OOXX, c_magenta, c_blue));
 w.putglyph(x1, y2, glyph(LINE_XXOO, c_magenta, c_blue));
 w.putglyph(x2, y2, glyph(LINE_XOOX, c_magenta, c_blue));

 if (x1 + 1 < x2) {
  temp_line(w, x1 + 1, y1, x2 - 1, y1);
  temp_line(w, x1 + 1, y2, x2 - 1, y2);
 }
 if (y1 + 1 < y2) {
  temp_line(w, x1, y1 + 1, x1, y2 - 1);
  temp_line(w, x2, y1 + 1, x2, y2 - 1);
 }
}

void paint(interface &edited, int x, int y)
{
 edited.set_data("BG", pen, x, y);
/*
 if (pen.symbol == LINE_XXXX)
  fix_lines(edited, "BG");
*/
}

void fix_lines(interface &edited, std::string name,
               int x1, int y1, int x2, int y2)
{
 if (x1 == -1 || x2 == -1 || y1 == -1 || y2 == -1) {
  x1 = 0;
  y1 = 0;
  x2 = edited.sizex;
  y2 = edited.sizey;
 }
 element* ele = edited.find_by_name(name);
 ele_drawing* bg = static_cast<ele_drawing*>(ele);
 if (!bg)
  return;
 std::map<Point, glyph>::iterator it;
 for (it = bg->drawing.begin(); it != bg->drawing.end(); it++) {
  Point p = it->first;
  if (p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2 &&
      is_line(it->second.symbol)) {
    Point north(it->first.x    , it->first.y - 1);
    Point  east(it->first.x + 1, it->first.y    );
    Point south(it->first.x    , it->first.y + 1);
    Point  west(it->first.x - 1, it->first.y    );
    bool north_line = (north.y >= y1 && is_line(bg->drawing[north].symbol) );
    bool  east_line = ( east.x <= x2 && is_line(bg->drawing[east ].symbol) );
    bool south_line = (south.y <= y2 && is_line(bg->drawing[south].symbol) );
    bool  west_line = ( west.x >= x1 && is_line(bg->drawing[west ].symbol) );
    if (north_line) {
      if (east_line) {
        if (south_line) {
          if (west_line)
            it->second.symbol =LINE_XXXX;
          else
            it->second.symbol =LINE_XXXO;
        } else {
          if (west_line)
            it->second.symbol =LINE_XXOX;
          else
            it->second.symbol =LINE_XXOO;
        }
      } else {
        if (south_line) {
          if (west_line)
            it->second.symbol =LINE_XOXX;
          else
            it->second.symbol =LINE_XOXO;
        } else {
          if (west_line)
            it->second.symbol =LINE_XOOX;
        }
      }
    } else {
      if (east_line) {
        if (south_line) {
          if (west_line)
            it->second.symbol =LINE_OXXX;
          else
            it->second.symbol =LINE_OXXO;
        } else {
          it->second.symbol =LINE_OXOX;
        }
      } else {
        if (south_line) {
          if (west_line)
            it->second.symbol =LINE_OOXX;
        } else {
          it->second.symbol =LINE_OXOX;
        }
      }
   }
  }
 }
}

bool is_line(long ch)
{
 return (ch == LINE_OOXX || ch == LINE_OXOX || ch == LINE_OXXO ||
         ch == LINE_OXXX || ch == LINE_XOOX || ch == LINE_XOXO ||
         ch == LINE_XOXX || ch == LINE_XXOO || ch == LINE_XXOX ||
         ch == LINE_XXXO || ch == LINE_XXXX);
}

void help()
{
 popup("\
S     Save & quit\n\
-     Open element browser\n\
_     Open binding browser\n\
Enter Create element\n\
<>    Select last / next element\n\
m     Move element\n\
r     Resize element\n\
i     Enter typing mode\n\
,     Enter drawing mode\n\
;     Draw line\n\
:     Draw box\n\
Esc   Cancel drawing, unselect element, exist typing/drawing mode\n\
.     place current symbol\n\
x     delete drawing under cursor OR delete selected element\n\
d     delete a range (by drawing a square)\n\
/     Fix lines\n\
'     Set pen symbol\n\
\"     Set pen symbol to line drawings\n\
c     Copy symbol & colors under pen\n\
[]    Set foreground / background color\n\
!     Toggle coordinates display\
");
}

void set_pen_symbol()
{
 long ch = getch();
 if (ch != ' ')
  pen.symbol = ch;
}

void set_pen_fg()
{
 nc_color tmp = pick_color();
 if (tmp != c_null)
  pen.fg = tmp;
}

void set_pen_bg()
{
 nc_color tmp = pick_color();
 if (tmp != c_null)
  pen.bg = tmp;
}

nc_color pick_color()
{
 std::stringstream text;
 Window w_col(1, 1, 20, 6);
 w_col.outline();
 for (int i = 0; i < c_dkgray; i++) {
  w_col.putch(i + 1, 1, nc_color(i), c_black, '#');
  w_col.putch(i + 1, 3, nc_color(i + 8), c_black, '#');
 }
 w_col.putstr(1, 2, c_white, c_black, "12345678");
 w_col.putstr(1, 4, c_white, c_black, "abcdefgh");

 w_col.refresh();
 long ch = getch();

 if (ch >= '1' && ch <= '8')
  return nc_color(ch - '1');
 if (ch >= 'a' && ch <= 'h')
  return nc_color(ch - 'a' + c_dkgray);

 return c_null;
}
