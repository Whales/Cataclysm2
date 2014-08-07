#include <sstream>
#include "stringfunc.h"
#include "window.h"

std::vector<std::string> break_into_lines(const std::string& text, int linesize)
{
  std::vector<std::string> words = split_string(text, " \n", true);

  std::vector<std::string> ret;
  std::string line;

  for (int i = 0; i < words.size(); i++) {
    int word_lng = remove_color_tags(words[i]).length();
    int line_lng = remove_color_tags(line).length();
    bool newline = false;
    if (line_lng + word_lng <= linesize) {
      line += words[i];
      newline = (line[ line.size() - 1] == '\n');
    } else if (line.empty()) { // Need to split the word.
// Find where to split.
      size_t pos = 0;
      int chars = 0;
      bool done = false;
      while (!done) {
        if (words[i].substr(pos, 3) == "<c=") { // Color tag; skip it
          pos = text.find('>', pos) + 1;
        } else {
          chars++;
          pos++;
        }
        if (chars == linesize - 2 || pos >= words[i].size() - 1) {
          done = true;
        }
      }
      ret.push_back(words[i].substr(0, pos) + '-');
      words[i] = words[i].substr(pos + 1);
      i--;
    } else {  // need to split.
      newline = true;
    }

    if (newline) {
      ret.push_back(line);
    
    if (line_lng + word_lng > linesize) { // Too long to add!

/*
std::vector<std::string> break_into_lines(std::string text, int linesize)
{
  std::vector<std::string> ret;

// Check for trivial case
  if (remove_color_tags(text).size() <= linesize) {
    ret.push_back(text);
    return ret;
  }
  std::string ctag;
  std::string cur_line;
  int cur_line_size = 0;

  while (!text.empty()) {
    if (text.substr(0, 3) == "<c=") { // It's a color tag
      size_t tag_end = text.find(">");
      if (tag_end == std::string::npos) {
        //debugmsg("Bad color tag! [%s]", text.c_str());
      } else {
// Stick the color tag text into color_tag and jump to the end
        ctag = text.substr(0, tag_end + 1); // +1 to include the '>'
        cur_line += ctag; // This is "free;" doesn't add to line_size
        text = text.substr(tag_end + 1);
        if (ctag == "<c=/>") {  // Don't save "cancel color" tags
          ctag = "";
        }
      }
    }
    size_t split_pos = text.find_first_of(" \n");
    if (split_pos == std::string::npos) { // No whitespace left!
// Just stick the text in if it'll fit.
      int mod_cur_line_size = cur_line_size + (cur_line.empty() ? 0 : 1);
      if (mod_cur_line_size + remove_color_tags(text).size() < linesize) {
        if (!cur_line.empty()) {
          cur_line += " ";
        }
        cur_line += text;
        text = "";
// Make a newline if the text won't fit in this one, but will by itself.
      } else if (remove_color_tags(text).size() <= linesize) {
        ret.push_back(cur_line);
        cur_line = ctag + text;
        text = "";
      } else {  // Gonna have to hyphenate it.
        if (!cur_line.empty()) {
          cur_line += " ";
          cur_line_size++;
        }
        int max = linesize - cur_line_size - 1; // - 1 for the '-'
        cur_line += text.substr(0, max) + '-';
        ret.push_back(cur_line);
        cur_line = ctag;
        cur_line_size = 0;
        text = text.substr(max);
      }
    } else {  // Whitespace found!

// If the whitespace we find is a line break, then we ALWAYS create a new line.
      bool force_split = (text[split_pos] == '\n');

      std::string word = text.substr(0, split_pos);
      text = text.substr(split_pos + 1); // +1 to skip the whitespace

      if (!word.empty()) {
        if (cur_line_size > 0) {  // Word needs a space in front
          word.insert(0, " ");
        }

        if (cur_line_size + word.size() > linesize) { // Need a linebreak.
          ret.push_back(cur_line);
          cur_line = ctag;
          cur_line_size = 0;
          if (word[0] == ' ') { // Undo insertion of space
            word = word.substr(1);
          }
        }
        cur_line += word;
        cur_line_size += word.size();
      }

      if (force_split) {
        ret.push_back(cur_line);
        cur_line = ctag;
        cur_line_size = 0;
      }
    }// End of whitespace found block
  } // while (!text.empty())

  if (!cur_line.empty()) {
    ret.push_back(cur_line);
  }

  return ret;
}
*/

/*
std::vector<std::string> break_into_lines(std::string text, int linesize)
{
 std::vector<std::string> ret;

 size_t chars = 0; // Number of actually-printed characters at...
 size_t pos = 0; // ... this point in the string
 size_t linebreak = std::string::npos; // The last acceptable breakpoint
 std::string active_color_tag;
 while ((text.length() > linesize || text.find('\n') != std::string::npos) &&
        pos < text.size()) {
 while (text.length() > linesize && pos < text.size()) {
  bool force = false;
  if (text.substr(pos, 3) == "<c=") {
   size_t tmppos = text.find('>', pos);
   if (tmppos == std::string::npos) {
    //debugmsg("Bad colortag!");
    return ret;
   } else {
    active_color_tag = text.substr(pos, tmppos - pos + 1);
    pos = tmppos;
   }
   linebreak = pos;
   chars--;
  } else if (text[pos] == '\n') {
   linebreak = pos;
   force = true;
  } else if (text[pos] == ' ') {
   linebreak = pos;
  }
  pos++;
  chars++;
  if (force || chars > linesize) {
   std::string tmp;
   if (linebreak == std::string::npos) {
    linebreak = linesize - 1;
    tmp = text.substr(0, linebreak) + "-";
    text = text.substr(linebreak);
   } else if (text[linebreak] == '\n' || text[linebreak] == ' ') {
    tmp = text.substr(0, linebreak);
    text = text.substr(linebreak + 1);
// Ostensibly for color tags, but could cause a problem?
   } else if (text[linebreak] == '>') {
    linebreak++;
    tmp = text.substr(0, linebreak);
    text = text.substr(linebreak);
   }
   ret.push_back(tmp);
   if (!active_color_tag.empty()) {
    text = active_color_tag + text;
   }
   pos = 0;
   chars = 0;
   linebreak = std::string::npos;
  }
 }
 if (!text.empty()) {
  ret.push_back(text);
 }
 return ret;
  size_t linebreak = text.find_last_of(" ", linesize);
  std::string tmp;
  if (linebreak == std::string::npos) {
   linebreak = linesize - 1;
   tmp = text.substr(0, linebreak) + "-";
  } else
   tmp = text.substr(0, linebreak);

  ret.push_back(tmp);
  text = text.substr(linebreak + 1);
 }

 ret.push_back(text);

 return ret;
}
*/

std::vector<std::string> split_string(const std::string& text, char split,
                                      bool keep_split)
{
  return split_string(text, std::string(split), keep_split);
}

std::vector<std::string> split_string(const std::string& text,
                                      const std::string& split,
                                      bool keep_split)
{
  std::vector<std::string> ret;
  bool done = false;
  size_t cur_pos = 0;
  while (!done) {
    size_t split_pos = text.find_first_of(split, cur_pos);
    if (split_pos == std::string::npos) {
      ret.push_back( text.substr(cur_pos) );
      done = true;
    } else {
      size_t end_pos = (keep_split ? split_pos + 1 : split_pos);
      ret.push_back( text.substr(cur_pos, end_pos) );
      cur_pos = split_pos + 1;
    }
  }
  return ret;
}

std::string load_to_delim(std::istream &datastream, std::string delim)
{
 std::string ret, tmp;
 do {
  datastream >> tmp;
  if (tmp != delim)
   ret += tmp + " ";
 } while (tmp != delim && !(datastream.eof()));

 if (!ret.empty() && ret[ret.size() - 1] == ' ')
  ret = ret.substr(0, ret.size() - 1);

 return ret;
}

std::string load_to_character(std::istream &datastream, char ch, bool _trim)
{
  std::string ret;
  char *tmp = new char [1];
  do {
    datastream.read(tmp, 1);
    if (tmp[0] != ch) {
      ret.push_back(tmp[0]);
    }
    debugmsg(ret.c_str());
  } while (tmp[0] != ch && !(datastream.eof()));

  if (_trim)
    ret = trim(ret);
  delete [] tmp;
  return ret;
}

std::string load_to_character(std::istream &datastream, std::string chars,
                              bool _trim)
{
  std::string ret;
  char *tmp = new char [1];
  do {
    datastream.read(tmp, 1);
    if (chars.find(tmp[0]) == std::string::npos) {
      ret.push_back(tmp[0]);
    }
    debugmsg(ret.c_str());
  } while (chars.find(tmp[0]) == std::string::npos && chars[0] >= 0 &&
           !(datastream.eof()));

  if (_trim)
    ret = trim(ret);

  delete [] tmp;
  return ret;
}

std::string trim(const std::string &orig)
{
  std::string ret = orig;
  int front = 0, back = ret.length() - 1;
  while (front < ret.length() &&
         (ret[front] == ' ' || ret[front] == '\n' || ret[front] == '\t')) {
    front++;
  }

  ret = ret.substr(front);

  back = ret.length() - 1;

  while (back >= 0 &&
         (ret[back] == ' ' || ret[back] == '\n' || ret[back] == '\t')) {
    back--;
  }

  ret = ret.substr(0, back + 1);

  return ret;
}

std::string all_caps(const std::string &orig)
{
  std::string ret = orig;
  for (int i = 0; i < ret.length(); i++) {
    if (ret[i] >= 'a' && ret[i] <= 'z')
      ret[i] += 'A' - 'a';
  }

  return ret;
}

std::string no_caps(const std::string &orig)
{
  std::string ret = orig;
  for (int i = 0; i < ret.length(); i++) {
    if (ret[i] >= 'A' && ret[i] <= 'Z')
      ret[i] += 'a' - 'A';
  }

  return ret;
}

std::string capitalize(const std::string &orig)
{
  std::string ret = orig;
  size_t tagpos = orig.find("<c="); // Find the first tag
  size_t start; // Used below
  if (tagpos != std::string::npos) {
    for (int i = 0; i < tagpos; i++) {  // Can we capitalize before the tag?
      if (ret[i] >= 'a' && ret[i] <= 'z') {
        ret[i] += 'A' - 'a';  // Capitalize!
        return ret;
      } else if (ret[i] != ' ') {
        return ret; // We're already capitalized!
      }
    }
// If we reach this point, we found a tag but there's nothing before it.
    start = orig.find(">", tagpos);
    start++;
  } else {  // No tags - start from the beginning of the string
    start = 0;
  }
  for (int i = start; i < ret.size(); i++) {
    if (ret[i] >= 'a' && ret[i] <= 'z') {
      ret[i] += 'A' - 'a';
      return ret;
    } else if (ret[i] != ' ') {
      return ret; // We're already capitalized!
    }
  }
  return ret; // All blank spaces??
}

std::string remove_color_tags(const std::string &orig)
{
  std::string ret;
  bool in_tag = false;
  for (int i = 0; i < orig.size(); i++) {
    if (in_tag) {
      if (orig[i] == '>') {
        in_tag = false;
      }
    } else {
      if (orig[i] == '<') {
        in_tag = true;
      } else {
        ret += orig[i];
      }
    }
  }
  return ret;
}

std::string itos(int num)
{
  std::stringstream ret;
  ret << num;
  return ret.str();
}

std::string color_gradient(int value, std::vector<int> breakpoints,
                           std::vector<nc_color> colors)
{
/* We need one more color than breakpoints, since breakpoints are the spots
 * BETWEEN colors:
 *  gray  <split: 5>  white  <split: 12>  green
 */
  if (breakpoints.size() + 1 != colors.size()) {
    debugmsg("color_gradient() called with mismatched breakpoints/colors!");
    return "";
  }

  nc_color col = c_null;
  for (int i = 0; i < breakpoints.size(); i++) {
    if (value <= breakpoints[i]) {
      col = colors[i];
    }
  }
  if (col == c_null) {
    col = colors.back();
  }
  std::stringstream ret;
  ret << "<c=" << color_tag(col) << ">";
  return ret.str();
}

bool is_vowel(char ch)
{
  return (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' ||
          ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U');
}
