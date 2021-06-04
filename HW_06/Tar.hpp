#ifndef HW_06_TAR_HPP
#define HW_06_TAR_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include "FileContainer.hpp"


using namespace std;

class Tar
{
public:
  void create(string &path, string &target);
  void extract(string &path, string &target);

private:
  vector<string> names;
  vector<FileContainer> containers;
  unordered_map<__ino_t, string> ino2file;

  void collectNames(string &path, string &rootDir, string prefix="");
  void saveToTarget(string &target, string &rootDir);
  void loadFromTarget(string &path, string &target);
  void restoreFiles(string &path);
};


#endif //HW_06_TAR_HPP
