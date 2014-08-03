#include "help.h"
#include "window.h"
#include "cuss.h"
#include "files.h"
#include <sstream>

void help_screen(std::string file, int line)
{
// This is a list of text files bound to different number input.
// They'll be looked for in DATA_DIR/help/
  std::vector<std::string> help_files;
  help_files.push_back("story.txt");
  help_files.push_back("introduction.txt");
  help_files.push_back("interface.txt");
  help_files.push_back("movement.txt");
  help_files.push_back("status_effects.txt");
  help_files.push_back("combat.txt");
  help_files.push_back("items.txt");
  help_files.push_back("configuration.txt");
  help_files.push_back("skills.txt");

  cuss::interface i_help;
  if (!i_help.load_from_file( CUSS_DIR + "/i_help.cuss" ) ) {
    return;
  }

  Window w_help(0, 0, 80, 24);

// We were told to read a specific file, and go to a specific line
  if (!file.empty()) {
    std::string filename = DATA_DIR + "/help/" + file;

    if (!file_exists(filename)) {
      i_help.set_data("text_help",
                      "Help file '" + filename + "' doesn't exist.");
    } else {
      i_help.set_data("text_help", slurp_file(filename));
      i_help.set_data("text_help", line);  // Scroll to specified line
    }
  }

  bool done = false;
  while (!done) {
    i_help.draw(&w_help);
    long ch = getch();
    if (ch >= 'A' && ch <= 'Z') {
      ch = ch - 'A' + 'a';
    }
    if (ch == 'c') {  // Interface tour is handled specially
      Window w_tour(0, 0, 80, 24);
      cuss::interface i_tour;
      for (int i = 1; i <= 6; i++) {
        std::stringstream tour_name;
        tour_name << CUSS_DIR << "/i_help_interface_" << i << ".cuss";
        if (!i_tour.load_from_file(tour_name.str())) {
          i = 100;
        }
        i_tour.draw(&w_tour);
        long ch;
        do { ch = getch(); } while (ch != ' ');
      }
    } else if (ch >= 'a' && ch - 'a' < help_files.size()) {
      std::string filename = DATA_DIR + "/help/" + help_files[ch - 'a'];
      if (!file_exists(filename)) {
        i_help.set_data("text_help",
                        "Help file '" + filename + "' doesn't exist.");
      } else {
        i_help.set_data("text_help", slurp_file(filename));
      }
      i_help.set_data("text_help", 0);  // Scroll to top
    } else if (ch == 'q' || ch == 'Q' || ch == KEY_ESC) {
      done = true;
    } else {
      i_help.handle_keypress(ch);
    }
  }
}

void help_skill_desc(Skill_type skill)
{
  std::string filename = DATA_DIR + "/help/skills.txt";
  if (!file_exists(filename)) {
    debugmsg("File '%s' does not exist!", filename.c_str());
    return;
  }
  std::string sk_name = skill_type_name(skill);
  int line_num = find_line_starting_with(filename, sk_name);
  if (line_num == -1) {
    debugmsg("No help on '%s' in '%s'!", sk_name.c_str(), filename.c_str());
    return;
  }
  help_screen(filename, line_num);
}
