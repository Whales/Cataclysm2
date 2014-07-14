#include "files.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>

std::string DATA_DIR;
std::string CUSS_DIR;
std::string SAVE_DIR;

bool directory_exists(std::string name)
{
  DIR *dir;
  dir = opendir(name.c_str());
  if (dir) {
    closedir(dir);
    return true;
  }
  return false;
}

bool create_directory(std::string name)
{
  if (directory_exists(name)) {
    return false; // Couldn't create it because it exists...
  }
#if (defined _WIN32 || defined __WIN32__)
  mkdir(name.c_str());
#else
  mkdir(name.c_str(), 0777);
#endif
  if (!directory_exists(name)) {  // Check to make sure we succeeded
    return false;
  }
  return true;
}

bool remove_directory(std::string name)
{
  if (!directory_exists(name)) {
    return false;
  }

  DIR *dir;
  struct dirent *entry;

  dir = opendir(name.c_str());
  if (dir == NULL) {
    return false;
  }

  while ((entry = readdir(dir)) != NULL) {
    std::string d_name = entry->d_name;
    std::string full_path = name + "/" + d_name;

    if (!d_name.empty() && d_name[0] != '.') {

      if (entry->d_type == DT_DIR) {  // remove other directories recursively
        remove_directory(full_path);
      }
      remove(full_path.c_str());
    }
  }

  remove(name.c_str());
  closedir(dir);

  return true;
}

bool file_exists(std::string name)
{
  std::ifstream fin;
  fin.open(name.c_str());
  if (fin.is_open()) {
    fin.close();
    return true;
  }
  return false;
}

bool remove_file(std::string name)
{
  if (!file_exists(name)) {
    return false;
  }

  remove(name.c_str());
  return true;
}

std::vector<std::string> files_in(std::string dir, std::string suffix)
{
  std::vector<std::string> ret;
  DIR *dp;
  dirent *dirp;

  if ( (dp = opendir(dir.c_str())) == NULL )
    return ret;

  while ( (dirp = readdir(dp)) != NULL ) {
    std::string filename(dirp->d_name);
    if (suffix == "" || filename.find(suffix) != std::string::npos) {
      ret.push_back( std::string(dirp->d_name) );
    }
  }

  closedir(dp);
  return ret;
}

std::vector<std::string> directories_in(std::string dir)
{
  std::vector<std::string> ret;
  DIR *dp;
  dirent *dirp;

  if ( (dp = opendir(dir.c_str())) == NULL ) {
    return ret;
  }

  while ( (dirp = readdir(dp)) != NULL ) {
#if (defined _WIN32 || defined WINDOWS)
    struct stat win_stat;
    stat(dirp->d_name, &win_stat);
    if (win_stat.st_mode & S_IFDIR) {
#else
    if (dirp->d_type == DT_DIR) {
#endif
      std::string dname = dirp->d_name;
      if (dname[0] != '.') {
        ret.push_back( std::string(dirp->d_name) );
      }
    }
  }
  return ret;
}

std::string slurp_file(const std::string &filename)
{
  std::string ret;
  std::ifstream fin;
  fin.open(filename.c_str());
  if (!fin.is_open()) {
    return ret;
  }

  ret.assign( (std::istreambuf_iterator<char>(fin) ),
              (std::istreambuf_iterator<char>()    ) );

  return ret;
}

void chomp(std::istream &data)
{
  if (data.peek() == '\n') {
    std::string junk;
    std::getline(data, junk);
  }
}

void set_default_dirs()
{
  if (DATA_DIR.empty()) {
    DATA_DIR = "./data";
  }
  if (CUSS_DIR.empty()) {
    CUSS_DIR = "./cuss";
  }
  if (SAVE_DIR.empty()) {
    SAVE_DIR = "./save";
  }
}

bool set_dir(std::string &dir, std::string name)
{
  if (!directory_exists(name)) {
    return false;
  }
  dir = name;
  return true;
}
