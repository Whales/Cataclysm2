#include "files.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>

std::string DATA_DIR;
std::string CUSS_DIR;

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
}

bool set_dir(std::string &dir, std::string name)
{
  if (!directory_exists(name)) {
    return false;
  }
  dir = name;
  return true;
}
