#include "cuss.h"
#include "stringfunc.h"
#include <sstream>
#include <fstream>

using namespace cuss;

void print_scrollbar(Window *win, int posx, int posy, int length, int offset,
                     int size, bool selected);


#define SELECTCOLOR c_blue

#define PREP_ELEMENT(ele) \
  (ele)->name = name;\
  (ele)->posx = posx;\
  (ele)->posy = posy;\
  (ele)->sizex = szx;\
  (ele)->sizey = szy;\
  (ele)->selected = false;\
  (ele)->selectable = selectable;\
  (ele)->align = ALIGN_LEFT

std::string cuss::element_type_name(cuss::element_type type)
{
  switch (type) {
    case ELE_NULL:      return "NULL";
    case ELE_DRAWING:   return "Drawing";
    case ELE_TEXTBOX:   return "Text";
    case ELE_LIST:      return "List selection";
    case ELE_TEXTENTRY: return "Text Entry";
    case ELE_NUMBER:    return "Number";
    case ELE_MENU:      return "Menu";
    default:            return "Unknown";
  }
  return "What the heck";
}

// Base save/load functions.
std::string element::save_data()
{
 std::stringstream ret;
 ret << name << " " << STD_DELIM << " " << posx << " " << posy << " " <<
        sizex << " " << sizey << " " << selectable << " " << align << " " <<
        v_align;
 return ret.str();
}

void element::load_data(std::istream &datastream)
{
 name = load_to_delim(datastream, STD_DELIM);
 int tmpalign, tmpvalign;

 datastream >> posx >> posy >> sizex >> sizey >> selectable >> tmpalign >>
               tmpvalign;
 align = alignment(tmpalign);
 v_align = vertical_alignment(tmpvalign);
}

bool element::set_data(nc_color FG, nc_color BG)
{
 if (fg != c_null)
  fg = FG;
 if (BG != c_null) // bg defaults to c_null
  bg = BG;

 return true;
}

// *** DRAWING ELEMENT ***
void ele_drawing::draw(Window *win)
{
 std::map<Point, glyph>::iterator it;
 for (it = drawing.begin(); it != drawing.end(); it++)
  win->putglyph( posx + it->first.x, posy + it->first.y, it->second);
}

std::string ele_drawing::save_data()
{
 std::stringstream ret;
 ret << element::save_data() << " " << drawing.size() << " ";
 std::map<Point, glyph>::iterator it;
 for (it = drawing.begin(); it != drawing.end(); it++)
  ret << it->first.x << " " << it->first.y << " " << it->second.save_data() <<
         " ";
 return ret.str();
}

void ele_drawing::load_data(std::istream &datastream)
{
 element::load_data(datastream);
 int tmpsize;
 datastream >> tmpsize;
 for (int i = 0; i < tmpsize; i++) {
  Point tmpPoint;
  glyph tmpglyph;
  datastream >> tmpPoint.x >> tmpPoint.y;
  tmpglyph.load_data(datastream);
  drawing[tmpPoint] = tmpglyph;
 }
}
 
bool ele_drawing::set_data(glyph gl, int posx, int posy)
{
 if (posx < 0 || posx >= sizex || posy < 0 || posy >= sizey)
  return false;

 if (gl.symbol == -1 || gl.symbol == ' ')
  drawing.erase( Point(posx, posy) );
 else
  drawing[ Point(posx, posy) ] = gl;

 return true;
}

bool ele_drawing::set_data(nc_color FG, nc_color BG)
{
 std::map<Point, glyph>::iterator it;
 for (it = drawing.begin(); it != drawing.end(); it++) {
  it->second.fg = FG;
  if (BG != c_null) // bg defaults to c_null
   it->second.bg = BG;
 }

 return true;
}

bool ele_drawing::translate(long from, long to)
{
 std::map<Point, glyph>::iterator it;
 bool found = false;
 for (it = drawing.begin(); it != drawing.end(); it++) {
  if (it->second.symbol == from) {
   it->second.symbol = to;
   found = true;
  }
 }
 return found;
}

// *** TEXTBOX ELEMENT ***
void ele_textbox::draw(Window *win)
{
 std::vector<std::string> broken = break_into_lines(*text, sizex);
 
 win->clear_area(posx, posy, posx + sizex - 1, posy + sizey - 1);

 for (int i = 0; i + offset <= broken.size() && i < sizey; i++) {
  int ypos, index;
  if (v_align == ALIGN_BOTTOM) {
    ypos = posy + sizey - 1 - i;
    index = broken.size() - 1 - i - offset;
  } else { // Default to top-aligned
    ypos = posy + i;
    index = i + offset;
  }
  if (index >= 0 && index < broken.size()) {
    if (align == ALIGN_RIGHT) {
      win->putstr_r(posx, ypos, fg, bg, sizex, broken[index]);
    } else if (align == ALIGN_CENTER) {
      win->putstr_c(posx, ypos, fg, bg, sizex, broken[index]);
    } else {
      win->putstr_n(posx, ypos, fg, bg, sizex, broken[index]);
    }
  }
 }

 if (selectable)
  print_scrollbar(win, posx + sizex - 1, posy, sizey, offset, broken.size(),
                  selected);
}

std::string ele_textbox::save_data()
{
 std::stringstream ret;
 ret << element::save_data() << " " << (*text) << " " << STD_DELIM;

 return ret.str();
}

void ele_textbox::load_data(std::istream &datastream)
{
 element::load_data(datastream);
 (*text) = load_to_delim(datastream, STD_DELIM);
}

bool ele_textbox::self_reference()
{
 if (owns_data)
  return false;

 text = new std::string;
 owns_data = true;
 return true;
}

bool ele_textbox::set_data(std::string data)
{
 (*text) = data;
 return true;
}

bool ele_textbox::add_data(std::string data)
{
 (*text) += data;
 return true;
}

bool ele_textbox::set_data(std::vector<std::string> data)
{
 (*text) = "";
 add_data(data);
 return true;
}

bool ele_textbox::add_data(std::vector<std::string> data)
{
 for (int i = 0; i < data.size(); i++)
  (*text) += data[i] + '\n';
 return true;
}

bool ele_textbox::ref_data(std::string *data)
{
 if (owns_data)
  delete text;

 text = data;
 owns_data = false;
 return true;
}

bool ele_textbox::set_data(int data)
{
 std::vector<std::string> broken = break_into_lines(*text, sizex);
 if (data <= 0)
  offset = 0;
 else if (data > broken.size() - 1)
  offset = broken.size() - 1;
/*
 else if (data > sizey - broken.size())
  offset = sizey - broken.size();
*/
 else
  offset = data;
 return true;
}

bool ele_textbox::add_data(int data)
{
 return set_data(offset + data);
}

std::vector<std::string> ele_textbox::get_str_list()
{
 return break_into_lines(*text, sizex);
}

// *** LIST ELEMENT ***
void ele_list::draw(Window *win)
{
 win->clear_area(posx, posy, posx + sizex - 1, posy + sizey - 1);

 for (int i = 0; i + offset < list->size() && i < sizey; i++) {
  nc_color hilite = (selection == i + offset ? SELECTCOLOR : bg);
  int ypos, index;
  if (v_align == ALIGN_BOTTOM) {
    ypos = posy + sizey - 1 - i;
    index = list->size() - 1 - i - offset;
  } else { // Default to top-aligned
    ypos = posy + i;
    index = i + offset;
  }
  if (!selected)
   hilite = bg;
  if (align == ALIGN_RIGHT) {
    win->putstr_r(posx, ypos, fg, hilite, sizex, (*list)[index]);
  } else if (align == ALIGN_CENTER) {
    win->putstr_c(posx, ypos, fg, hilite, sizex, (*list)[index]);
  } else {
    win->putstr_n(posx, ypos, fg, hilite, sizex, (*list)[index]);
  }
 }

 if (selectable)
  print_scrollbar(win, posx + sizex - 1, posy, sizey, offset, list->size(),
                  selected);
}

std::string ele_list::save_data()
{
 std::stringstream ret;
 ret << element::save_data() << " " << list->size() << " ";
 for (int i = 0; i < list->size(); i++)
  ret << (*list)[i] << " " << STD_DELIM << " ";

 return ret.str();
}

void ele_list::load_data(std::istream &datastream)
{
 element::load_data(datastream);
 int tmpsize;
 datastream >> tmpsize;
 for (int i = 0; i < tmpsize; i++) {
  std::string tmp = load_to_delim(datastream, STD_DELIM);
  list->push_back(tmp);
 }
}

bool ele_list::self_reference()
{
 if (owns_data)
  return false;

 list = new std::vector<std::string>;
 owns_data = true;
 return true;
}

bool ele_list::set_data(std::string data)
{
  list->clear();
  add_data(data);
  return true;
}

bool ele_list::add_data(std::string data)
{
 list->push_back(data);
 return true;
}

bool ele_list::set_data(std::vector<std::string> data)
{
 (*list) = data;
 selection = 0;
 offset = 0;
 return true;
}

bool ele_list::add_data(std::vector<std::string> data)
{
 for (int i = 0; i < data.size(); i++)
  list->push_back(data[i]);
 return true;
}

bool ele_list::ref_data(std::vector<std::string> *data)
{
 if (owns_data)
  delete list;

 list = data;
 owns_data = false;
 return true;
}

bool ele_list::set_data(int data)
{
 selection = data;

 if (selection < 0)
  selection = 0;
 if (selection >= list->size())
  selection = list->size() - 1;

 if (selection < sizey)
  offset = 0;
 else if (selection >= list->size() - sizey)
  offset = list->size() - sizey;
 else
  offset = selection;

 return true;
}

bool ele_list::add_data(int data)
{
 selection += data;
 if (selection < 0)
  selection = 0;
 if (selection >= list->size())
  selection = list->size() - 1;

 while (selection < offset)
  offset--;
 if (offset + sizey <= selection)
  offset = selection - sizey + 1;

 return true;
}

int ele_list::get_int()
{
 return selection;
}

std::string ele_list::get_str()
{
 if (selection < 0 || selection >= list->size()) {
  std::string ret;
  return ret;
 }

 return (*list)[selection];
}

// *** TEXT ENTRY ELEMENT ***
void ele_textentry::draw(Window *win)
{
 nc_color hilite = (selected ? SELECTCOLOR : bg);
// Ensure we see the end of the word--and a blank space
 int start = (selected ? text->size() + 1 - sizex : 0);
 if (start < 0)
  start = 0;
 int length = (selected ? sizex - 1 : sizex);

 std::string print = text->substr(start, length);

 win->putstr_raw(posx, posy, fg, hilite, print);
 for (int x = posx + print.length(); x < posx + sizex; x++)
  win->putch(x, posy, bg, hilite, '_');
}

std::string ele_textentry::save_data()
{
 std::stringstream ret;
 ret << element::save_data() << " " << (*text) << " " << STD_DELIM;
 return ret.str();
}

void ele_textentry::load_data(std::istream &datastream)
{
 element::load_data(datastream);
 (*text) = load_to_delim(datastream, STD_DELIM);
}

bool ele_textentry::self_reference()
{
 if (owns_data)
  return false;

 text = new std::string;
 owns_data = true;
 return true;
}

bool ele_textentry::set_data(std::string data)
{
 (*text) = data;
 return true;
}

bool ele_textentry::add_data(std::string data)
{
 (*text) += data;
 return true;
}

bool ele_textentry::ref_data(std::string *data)
{
 if (owns_data)
  delete text;

 text = data;
 owns_data = false;
 return true;
}

bool ele_textentry::handle_keypress(long ch)
{
  if (is_backspace(ch) && !text->empty()) {
    (*text) = text->substr(0, text->length() - 1);
    return true;
  }
  if (ch >= 32 && ch <= 126) { // Printable chars
    (*text) += char(ch);
    return true;
  }
  return false;
}

// *** NUMBER ELEMENT ***
void ele_number::draw(Window *win)
{
 nc_color hilite = (selected ? SELECTCOLOR : bg);
 if (align == ALIGN_RIGHT) {
  win->putstr_r(posx, posy, fg, hilite, sizex, "%d", (*value));
 } else if (align == ALIGN_CENTER) {
  win->putstr_c(posx, posy, fg, hilite, sizex, "%d", (*value));
 } else {
  win->putstr_n(posx, posy, fg, hilite, sizex, "%d", (*value));
 }
}

std::string ele_number::save_data()
{
 std::stringstream ret;
 ret << element::save_data() << " " << (*value);
 return ret.str();
}

void ele_number::load_data(std::istream &datastream)
{
 element::load_data(datastream);
 datastream >> (*value);
}

bool ele_number::self_reference()
{
 if (owns_data)
  return false;

 value = new int;
 (*value) = 0;
 owns_data = true;
 return true;
}

bool ele_number::set_data(int data)
{
 (*value) = data;
 return true;
}

bool ele_number::add_data(int data)
{
 (*value) += data;
 return true;
}

bool ele_number::ref_data(int *data)
{
 if (owns_data)
  delete value;

 value = data;
 owns_data = false;
 return true;
}

bool ele_number::handle_keypress(long ch)
{
  if (ch >= '0' && ch <= '9') {
    if (recently_selected) {
      (*value) = (ch - '0');
    } else {
      (*value) *= 10;
      (*value) += (ch - '0');
    }
    return true;
  } else if (is_backspace(ch)) {
    (*value) /= 10;
    return true;
  } else if (ch == '-') {
    (*value)*= -1;
  }
  return false;
}

// *** MENU ELEMENT ***
void ele_menu::draw(Window *win)
{
  std::string text = title;
  if (selection >= 0) {
    text = (*list)[selection];
  }

  if (!selected || !open) {
    if (align == ALIGN_RIGHT) {
      win->putstr_r(posx + 1, posy, fg, (selected ? SELECTCOLOR : bg),
                    sizex - 2, text);
    } else if (align == ALIGN_CENTER) {
      win->putstr_c(posx + 1, posy, fg, (selected ? SELECTCOLOR : bg),
                    sizex - 2, text);
    } else {
      win->putstr_n(posx + 1, posy, fg, (selected ? SELECTCOLOR : bg),
                    sizex - 2, text);
    }
    return;
  }
// The rest is for when it's selected, i.e. open
// Draw outline first
  int tmpsizey = list->size() + 2;
  if (posy + tmpsizey > win->sizey()) {
    tmpsizey = win->sizey() - posy;
  }
// Vertical lines
  for (int y = posy + 1; y < posy + tmpsizey - 1; y++) {
    win->putch(posx, y, fg, bg, LINE_XOXO);
    win->putch(posx + sizex - 2, y, fg, bg, LINE_XOXO);
  }

// Horizontal lines
  for (int x = posx + 1; x < posx + sizex - 1; x++) {
    win->putch(x, posy, fg, bg, LINE_OXOX);
    win->putch(x, posy + tmpsizey - 1, fg, bg, LINE_OXOX);
  }

// Corners
  win->putch(posx,             posy,                fg, bg, LINE_OXXO);
  win->putch(posx + sizex - 2, posy,                fg, bg, LINE_OOXX);
  win->putch(posx,             posy + tmpsizey - 1, fg, bg, LINE_XXOO);
  win->putch(posx + sizex - 2, posy + tmpsizey - 1, fg, bg, LINE_XOOX);

// Then draw menu items
  if (align == ALIGN_RIGHT) {
    win->putstr_r(posx + 1, posy, fg, bg, sizex - 2, title);
  } else if (align == ALIGN_CENTER) {
    win->putstr_c(posx + 1, posy, fg, bg, sizex - 2, title);
  } else {
    win->putstr_n(posx + 1, posy, fg, bg, sizex - 2, title);
  }
  for (int i = 0; i < tmpsizey && i < list->size(); i++) {
    int n = i, line = i + posy + 1;
    if ((*list)[n] == "-") { // Single dash indicates a horizontal line
      win->putch(posx, line, fg, bg, LINE_XXXO);
      win->putch(posx + sizex - 1, line, fg, bg, LINE_XOXX);
      for (int x = posx + 1; x < posx + sizex - 2; x++) {
        win->putch(x, line, fg, bg, LINE_OXOX);
      }
    } else {
// Clear the line using black Xs
      for (int x = posx + 1; x < posx + sizex - 2; x++) {
        win->putch(x, line, c_black, c_black, 'x');
      }
      nc_color back = (n == selection ? SELECTCOLOR : bg);
      if (align == ALIGN_RIGHT) {
        win->putstr_r(posx + 1, line, fg, back, sizex - 2, (*list)[n]);
      } else if (align == ALIGN_CENTER) {
        win->putstr_c(posx + 1, line, fg, back, sizex - 2, (*list)[n]);
      } else {
        win->putstr_n(posx + 1, line, fg, back, sizex - 2, (*list)[n]);
      }
    }
  }
}

std::string ele_menu::save_data()
{
 std::stringstream ret;
 ret << element::save_data() << " " << title << " " << STD_DELIM << " " <<
        list->size() << " ";
 for (int i = 0; i < list->size(); i++)
  ret << (*list)[i] << " " << STD_DELIM << " ";

 return ret.str();
}

void ele_menu::load_data(std::istream &datastream)
{
 element::load_data(datastream);
 title = load_to_delim(datastream, STD_DELIM);
 int tmpsize;
 datastream >> tmpsize;
 for (int i = 0; i < tmpsize; i++) {
  std::string tmp = load_to_delim(datastream, STD_DELIM);
  list->push_back(tmp);
 }
}

bool ele_menu::self_reference()
{
 if (owns_data)
  return false;

 list = new std::vector<std::string>;
 owns_data = true;
 return true;
}

bool ele_menu::set_data(std::string data)
{
 title = data;
 return true;
}

bool ele_menu::add_data(std::string data)
{
 list->push_back(data);
 return true;
}

bool ele_menu::handle_keypress(long ch)
{
  if (ch == '\n') {
    open = false;
    return true;
  }
  return false;
}

bool ele_menu::set_data(std::vector<std::string> data)
{
 (*list) = data;
 selection = 0;
 open = false;
 return true;
}

bool ele_menu::add_data(std::vector<std::string> data)
{
 for (int i = 0; i < data.size(); i++)
  list->push_back(data[i]);
 return true;
}

bool ele_menu::ref_data(std::vector<std::string> *data)
{
 if (owns_data)
  delete list;

 list = data;
 owns_data = false;
 return true;
}

bool ele_menu::set_data(int data)
{
  selection = data;

  if (selection < 0)
    selection = 0;
  if (selection >= list->size())
    selection = list->size() - 1;

  if (data != -1) {
    open = true;
  }

 return true;
}

bool ele_menu::add_data(int data)
{
 return set_data(selection + data);
}

std::string ele_menu::get_str()
{
 if (selection < 0 || selection >= list->size()) {
  std::string ret;
  return ret;
 }
 return (*list)[selection];
}

int ele_menu::get_int()
{
 return selection;
}

void print_scrollbar(Window *win, int posx, int posy, int length, int offset,
                     int size, bool selected)
{
 nc_color barcol = (selected ? SELECTCOLOR : c_ltgray);
 int barsize = (length >= size ? -1 : 1 + ((length * length) / size));

 if (barsize == -1) {
// Don't print a scroll bar
  for (int y = posy; y < posy + length; y++)
   win->putch(posx, y, barcol, c_black, LINE_XOXO);
 } else {
  int barpos = (offset * length) / size;
  if (barpos + barsize > length)
   barpos = length - barsize;
  for (int y = 0; y < length; y++) {
   long ch = ((y >= barpos && y < barpos + barsize) ? '#' : LINE_XOXO);
   win->putch(posx, posy + y, barcol, c_black, ch);
  }
 }
}

std::string binding::save_data()
{
 std::stringstream ret;
 ret << int(act) << " " << target << " " STD_DELIM << " " << a << " " << b;
 return ret.str();
}

void binding::load_data(std::istream &datastream)
{
 int tmpact;
 datastream >> tmpact;
 target = load_to_delim(datastream, STD_DELIM);
 datastream >> a >> b;
 act = action_id(tmpact);
}

interface::interface(std::string N, int X, int Y)
{
 active_element = -1;
 use_bindings = false;
 name = N;
 sizex = X;
 sizey = Y;
}

interface::~interface()
{
 for (int i = 0; i < elements.size(); i++)
  delete elements[i];
}

void interface::add_element(element_type type, std::string name, int posx,
                            int posy, int szx, int szy, bool selectable)
{
 if (posx < 0 || posx >= sizex || posy < 0 || posy >= sizey)
  return;

 if (posx + szx >= sizex)
  szx = sizex - posx;
 if (posy + szy >= sizey)
  szy = sizey - posy;

 if (name.find(' ') != std::string::npos)
  return;

 switch (type) {
 
  case ELE_NULL:
   return; // We don't have any reason to actually add these, right?
 
  case ELE_DRAWING: {
   ele_drawing *ele = new ele_drawing;
   PREP_ELEMENT(ele);
   elements.push_back(ele);
  } break;
 
  case ELE_TEXTBOX: {
   ele_textbox *ele = new ele_textbox;
   PREP_ELEMENT(ele);
   elements.push_back(ele);
  } break;
 
  case ELE_LIST: {
   ele_list *ele = new ele_list;
   PREP_ELEMENT(ele);
   elements.push_back(ele);
  } break;
 
  case ELE_TEXTENTRY: {
   ele_textentry *ele = new ele_textentry;
   PREP_ELEMENT(ele);
   elements.push_back(ele);
  } break;
 
  case ELE_NUMBER: {
   ele_number *ele = new ele_number;
   PREP_ELEMENT(ele);
   elements.push_back(ele);
  } break;

  case ELE_MENU: {
   ele_menu *ele = new ele_menu;
   PREP_ELEMENT(ele);
   elements.push_back(ele);
  } break;
 
  default:
   debugmsg("Unknown element type %d", type);
   return;
 }

 if (active_element == -1)
  active_element = 0;
}

bool interface::erase_element(std::string name)
{
 for (int i = 0; i < elements.size(); i++) {
  if (elements[i]->name == name) {
   element *el = elements[i];
   elements.erase(elements.begin() + i);
   delete el;
   if (active_element == i)
    active_element = -1;
   return true;
  }
 }
 return false;
}

void interface::draw(Window *win)
{
 win->clear();
 std::vector<element*> draw_last; // Menus need to be layered at the top
 for (int i = 0; i < elements.size(); i++) {
  if (elements[i]->type() == ELE_MENU && elements[i]->selected)
   draw_last.push_back(elements[i]);
  else
   elements[i]->draw(win);
 }

 for (int i = 0; i < draw_last.size(); i++)
  draw_last[i]->draw(win);

 win->refresh();
}

void interface::draw_prototype(Window *win)
{
 win->clear();
 for (int i = 0; i < elements.size(); i++) {
  if (elements[i]->name != "BG") {
   int x1 = elements[i]->posx, y1 = elements[i]->posy;
   int x2 = x1 + elements[i]->sizex - 1, y2 = y1 + elements[i]->sizey - 1;
// Draw the background color
   for (int x = x1; x <= x2; x++) {
    for (int y = y1; y <= y2; y++) {
     element_type type = elements[i]->type();
     win->putch(x, y, c_black,
                type == ELE_DRAWING ? c_dkgray : nc_color(2 + type), ' ');
    }
   }
// Draw the corner/side delineators
   if (y1 == y2) {
    win->putch(x1, y1, c_white, c_black, LINE_XXXO);
    win->putch(x2, y2, c_white, c_black, LINE_XOXX);
   } else if (x1 == x2) {
    win->putch(x1, y1, c_white, c_black, LINE_OXXX);
    win->putch(x2, y2, c_white, c_black, LINE_XXOX);
   } else {
    win->putch(x1, y1, c_white, c_black, LINE_OXXO);
    win->putch(x2, y2, c_white, c_black, LINE_XOOX);
   }
   if (elements[i]->align == ALIGN_RIGHT) {
    win->putstr_r(x1 + 1, y1, (elements[i]->selected ? c_magenta : c_yellow),
                  c_black, elements[i]->sizex - 2, elements[i]->name);
   } else if (elements[i]->align == ALIGN_CENTER) {
    win->putstr_c(x1 + 1, y1, (elements[i]->selected ? c_magenta : c_yellow),
                  c_black, elements[i]->sizex - 2, elements[i]->name);
   } else {
    win->putstr_n(x1 + 1, y1, (elements[i]->selected ? c_magenta : c_yellow),
                  c_black, elements[i]->sizex - 2, elements[i]->name);
   }
  }
  if (elements[i]->type() == ELE_DRAWING)
   elements[i]->draw(win);
 }
}

std::string interface::save_data()
{
 std::stringstream ret;

 ret << name << " " << STD_DELIM << " " << sizex << " " << sizey << " " <<
        elements.size() << " ";
 for (int i = 0; i < elements.size(); i++)
  ret << elements[i]->type() << " " << elements[i]->save_data() << std::endl;

 ret << bindings.size() << " ";
 std::map<long, binding>::iterator it;
 for (it = bindings.begin(); it != bindings.end(); it++)
  ret << it->first << " " << it->second.save_data() << std::endl;

 return ret.str();
}

void interface::load_data(std::istream &datastream)
{
 name = load_to_delim(datastream, STD_DELIM);
 datastream >> sizex >> sizey;
 elements.clear();
 int tmpcount, tmpbind;
 datastream >> tmpcount;
 for (int i = 0; i < tmpcount; i++) {
  int tmptype;
  datastream >> tmptype;
  switch ( element_type(tmptype) ) {

   case ELE_NULL:
    debugmsg("Loaded NULL element!");
    break;

   case ELE_DRAWING: {
    ele_drawing *tmp = new ele_drawing;
    tmp->load_data(datastream);
    elements.push_back(tmp);
   } break;

   case ELE_TEXTBOX: {
    ele_textbox *tmp = new ele_textbox;
    tmp->load_data(datastream);
    elements.push_back(tmp);
   } break;

   case ELE_LIST: {
    ele_list *tmp = new ele_list;
    tmp->load_data(datastream);
    elements.push_back(tmp);
   } break;

   case ELE_TEXTENTRY: {
    ele_textentry *tmp = new ele_textentry;
    tmp->load_data(datastream);
    elements.push_back(tmp);
   } break;

   case ELE_NUMBER: {
    ele_number *tmp = new ele_number;
    tmp->load_data(datastream);
    elements.push_back(tmp);
   } break;

   case ELE_MENU: {
    ele_menu *tmp = new ele_menu;
    tmp->load_data(datastream);
    elements.push_back(tmp);
   } break;
  }
 } // for (int i = 0; i < tmpcount; i++)

 datastream >> tmpbind;
 for (int i = 0; i < tmpbind; i++) {
  long tmpch;
  binding bind;
  datastream >> tmpch;
  bind.load_data(datastream);
  bindings[tmpch] = bind;
 }
}

bool interface::save_to_file(std::string filename)
{
 std::ofstream fout;
 fout.open(filename.c_str());
 if (!fout.is_open())
  return false;

 fout << save_data();
 fout.close();
 return true;
}

bool interface::load_from_file(std::string filename, bool warn)
{
  std::ifstream fin;
  fin.open(filename.c_str());
  if (!fin.is_open()) {
    if (warn) {
      debugmsg("Can't load %s!", filename.c_str());
    }
    return false;
  }

  load_data(fin);
  fin.close();
  return true;
}

std::vector<std::string> interface::element_names()
{
 std::vector<std::string> ret;
 for (int i = 0; i < elements.size(); i++)
  ret.push_back( elements[i]->name );

 return ret;
}

element* interface::selected()
{
 if (active_element < 0 || active_element >= elements.size())
  return NULL;

 return elements[active_element];
}

element* interface::find_by_name(std::string name)
{
 for (int i = 0; i < elements.size(); i++) {
  if (elements[i]->name == name)
   return elements[i];
 }
 return NULL;
}

element* interface::select_next(bool force)
{
 if (elements.empty()) {
  active_element = -1;
  return NULL;
 }

 if (active_element >= 0 && active_element < elements.size())
  elements[active_element]->selected = false;

 int tried = 0;
 do {
  tried++;
  if (active_element >= elements.size() - 1)
   active_element = 0;
  else
   active_element++;
 } while ((!force && tried < elements.size() &&
           !elements[active_element]->selectable) ||
          elements[active_element]->name == "BG");

 if (tried == elements.size() && !elements[active_element]->selectable) {
  active_element = -1;
  return NULL;
 } else {
  elements[active_element]->selected = true;
  elements[active_element]->recently_selected = true;
  return elements[active_element];
 }
 return NULL;
}

element* interface::select_last(bool force)
{
 if (elements.empty()) {
  active_element = -1;
  return NULL;
 }

 if (active_element >= 0 && active_element < elements.size())
  elements[active_element]->selected = false;

 int tried = 0;
 do {
  tried++;
  if (active_element <= 0)
   active_element = elements.size() - 1;
  else
   active_element--;
 } while ((!force && tried < elements.size() &&
           !elements[active_element]->selectable) ||
          elements[active_element]->name == "BG");

 if (tried == elements.size() && !elements[active_element]->selectable) {
  active_element = -1;
  return NULL;
 } else {
  elements[active_element]->selected = true;
  elements[active_element]->recently_selected = true;
  return elements[active_element];
 }
 return NULL;
}

element* interface::select(std::string name)
{
 for (int i = 0; i < elements.size(); i++) {
  if (elements[i]->name == name) {
   if (active_element >= 0 && active_element < elements.size())
    elements[active_element]->selected = false;
   active_element = i;
   elements[active_element]->selected = true;
   elements[active_element]->recently_selected = true;
   return elements[active_element];
  }
 }
 return NULL;
}

void interface::select_none()
{
 if (active_element >= 0 && active_element < elements.size())
  elements[active_element]->selected = false;

 active_element = -1;
}

bool interface::move_element_up(std::string name)
{
// Check all except the last element
 for (int i = 0; i < elements.size() - 1; i++) {
  if (elements[i]->name == name) {
   element* tmp = elements[i + 1];
   elements[i + 1] = elements[i];
   elements[i] = tmp;
   return true;
  }
 }
 return false;
}

bool interface::move_element_down(std::string name)
{
// Check all except the first element
 for (int i = 1; i < elements.size(); i++) {
  if (elements[i]->name == name) {
   element* tmp = elements[i - 1];
   elements[i - 1] = elements[i];
   elements[i] = tmp;
   return true;
  }
 }
 return false;
}


bool interface::set_selectable(std::string name, bool setting)
{
 element* el = find_by_name(name);
 if (!el)
  return false;

 el->selectable = setting;
 return true;
}

bool interface::set_data(std::string name, std::string data)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->set_data(data);
}

bool interface::add_data(std::string name, std::string data)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->add_data(data);
}

bool interface::set_data(std::string name, std::vector<std::string> data)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->set_data(data);
}

bool interface::add_data(std::string name, std::vector<std::string> data)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->add_data(data);
}

bool interface::set_data(std::string name, int data)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->set_data(data);
}

bool interface::add_data(std::string name, int data)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->add_data(data);
}

bool interface::set_data(std::string name, glyph gl, int x, int y)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->set_data(gl, x, y);
}

// bg defaults to c_null
bool interface::set_data(std::string name, nc_color fg, nc_color bg)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->set_data(fg, bg);
}

bool interface::self_reference(std::string name)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->self_reference();
}

bool interface::ref_data(std::string name, std::string *data)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->ref_data(data);
}

bool interface::ref_data(std::string name, std::vector<std::string> *data)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->ref_data(data);
}

bool interface::ref_data(std::string name, int *data)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 return ele->ref_data(data);
}

bool interface::clear_data(std::string name)
{
 element* ele = find_by_name(name);
 if (!ele)
  return false;

 ele->clear_data();
 return true;
}

std::string interface::get_str(std::string name)
{
 element* ele = find_by_name(name);
 if (!ele) {
  std::string ret;
  return ret;
 }

 return ele->get_str();
}

int interface::get_int(std::string name)
{
 element* ele = find_by_name(name);
 if (!ele)
  return 0;

 return ele->get_int();
}

std::vector<std::string> interface::get_str_list(std::string name)
{
 element* ele = find_by_name(name);
 if (!ele) {
  std::vector<std::string> ret;
  return ret;
 }
 return ele->get_str_list();
}

std::vector<std::string> interface::binding_list()
{
 std::vector<std::string> ret;

 std::map<long, binding>::iterator it;
 for (it = bindings.begin(); it != bindings.end(); it++) {
  std::stringstream info;
  info << key_name(it->first) << ": " << action_name(it->second.act);
  if (it->second.act == ACT_SELECT_STR)
   info << " " << it->second.target;
  else if (it->second.act == ACT_SCROLL)
   info << " (" << it->second.target << (it->second.a >= 0 ? " +" : " ") <<
           it->second.a << ")";
  else if (it->second.act == ACT_SET_COLORS)
   info <<  "(" << it->second.target << ", " <<
           color_name( nc_color(it->second.a) ) << ", " <<
           color_name( nc_color(it->second.b) ) << ")";
  else if (it->second.act == ACT_TRANSLATE)
   info << " (" << it->second.target << "; " << char(it->second.a) << " to " <<
           char(it->second.b) << ")";

  ret.push_back(info.str());
 }
 return ret;
}

bool interface::add_binding(long ch, action_id act, std::string target)
{
 if (bindings.count(ch)) {
  debugmsg("Binding exists for %d!", ch);
  return false;
 }
 if (action_needs_element(act) && target != "<S>" && !find_by_name(target)) {
  debugmsg("Couldn't find element \"%s\"!", target.c_str());
  return false;
 }

 binding newbind(act, target);
 bindings[ch] = newbind;
 return true;
}

bool interface::add_binding(long ch, action_id act, std::string target,
                            int a, int b)
{
 if (bindings.count(ch)) {
  debugmsg("Binding exists for %d!", ch);
  return false;
 }
 if (action_needs_element(act) && target != "<S>" && !find_by_name(target)) {
  debugmsg("Couldn't find element \"%s\"!", target.c_str());
  return false;
 }

 binding newbind(act, target, a, b);
 bindings[ch] = newbind;
 return true;
}

binding* interface::bound_to(long ch)
{
 if (!bindings.count(ch))
  return NULL;

 return &(bindings[ch]);
}

bool interface::has_bindings_for(action_id act)
{
 std::map<long, binding>::iterator it;
 for (it = bindings.begin(); it != bindings.end(); it++) {
  if (it->second.act == act)
   return true;
 }
 return false;
}

bool interface::has_bindings_for(std::string target)
{
 std::map<long, binding>::iterator it;
 for (it = bindings.begin(); it != bindings.end(); it++) {
  if (it->second.target == target)
   return true;
 }
 return false;
}

bool interface::rem_binding(long ch)
{
 if (!bindings.count(ch))
  return false;

 bindings.erase(ch);
 return true;
}

bool interface::rem_all_bindings(action_id act)
{
 if (bindings.empty())
  return false;

 if (act == ACT_NULL) {
  bindings.clear();
  return true;
 }
 std::vector<long> to_delete;
 std::map<long, binding>::iterator it;
 for (it = bindings.begin(); it != bindings.end(); it++) {
  if (it->second.act == act)
   to_delete.push_back(it->first);
 }
 if (to_delete.empty())
  return false;

 for (int i = 0; i < to_delete.size(); i++)
  bindings.erase( to_delete[i] );

 return true;
}

bool interface::rem_all_bindings(std::string target)
{
 if (bindings.empty())
  return false;

 std::vector<long> to_delete;
 std::map<long, binding>::iterator it;
 for (it = bindings.begin(); it != bindings.end(); it++) {
  if (it->second.target == target)
   to_delete.push_back(it->first);
 }
 if (to_delete.empty())
  return false;

 for (int i = 0; i < to_delete.size(); i++)
  bindings.erase( to_delete[i] );

 return true;
}

bool interface::set_use_bindings(bool set)
{
 use_bindings = set;
 return true;
}

bool interface::handle_action(long ch)
{
 if (!bindings.count(ch))
  return false;

 binding* used = &(bindings[ch]);
 element* found = (used->target == "" ||used->target == "<S>" ? selected() :
                   find_by_name(used->target));

 switch (used->act) {

  case ACT_NULL:
   return true;

  case ACT_SELECT_NEXT:
   select_next();
   return true;

  case ACT_SELECT_LAST:
   select_last();
   return true;

  case ACT_SELECT_NONE:
   select_none();
   return true;

  case ACT_SELECT_STR:
   if (select(used->target))
    return true;
   return false;

  case ACT_SCROLL:
   if (!found)
    return false;
   if (found->type() != ELE_TEXTBOX && found->type() != ELE_LIST &&
       found->type() != ELE_NUMBER  && found->type() != ELE_MENU)
    return false;
   found->add_data(used->a);
   return true;

  case ACT_SET_COLORS:
   if (!found)
    return false;
   found->set_data( nc_color(used->a), nc_color(used->b) );
   return true;

  case ACT_TRANSLATE:
   if (!found)
    return false;
   if (found->type() == ELE_DRAWING) {
    ele_drawing* draw = static_cast<ele_drawing*>(found);
    draw->translate(used->a, used->b);
    return true;
   } else
    return false;
   
   default:
    return false;
 }

 return false;
}

bool interface::handle_keypress(long ch)
{
  if (handle_action(ch)) {
    return true; // We had a keybinding for it!
  }
  if (!selected()) {
    return false;
  } else {
    bool ret = selected()->handle_keypress(ch);
    selected()->recently_selected = false;
    return ret;
  }
}

bool cuss::action_needs_element(cuss::action_id act)
{
 return (act == ACT_SELECT_STR || act == ACT_SCROLL || act == ACT_SET_COLORS ||
         act == ACT_TRANSLATE);
}

std::string cuss::action_name(cuss::action_id act)
{
 switch (act) {

  case ACT_NULL:        return "Nothing";

  case ACT_SELECT_NEXT: return "Select Next";
  case ACT_SELECT_LAST: return "Select Last";
  case ACT_SELECT_NONE: return "Select None";
  case ACT_SELECT_STR:  return "Select Element";

  case ACT_SCROLL:      return "Scroll";

  case ACT_SET_COLORS:  return "Set Colors";
  case ACT_TRANSLATE:   return "Translate";

  default:              return "Oops we forgot to name this";

 }
 return "What the heck?!";
}
