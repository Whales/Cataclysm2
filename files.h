#ifndef _FILES_H_
#define _FILES_H_

#include <vector>
#include <string>
#include <istream>

extern std::string DATA_DIR;
extern std::string CUSS_DIR;
extern std::string SAVE_DIR;
// TODO: extern std::string PROFILE_DIR;

bool directory_exists(std::string name);
bool create_directory(std::string name);
bool remove_directory(std::string name);
bool file_exists     (std::string name);
bool remove_file     (std::string name);
std::vector<std::string> files_in(std::string dir, std::string suffix = "");
std::vector<std::string> directories_in(std::string dir);

std::string slurp_file(const std::string &filename);
int find_line_starting_with(const std::string &filename, std::string term,
                            bool match_case = false);

void chomp(std::istream &data);

void set_default_dirs();
// set_dir() returns false if the directory named doesn't exist
bool set_dir(std::string &dir, std::string name);

#endif
