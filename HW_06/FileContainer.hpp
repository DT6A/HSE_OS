#ifndef HW_06_FILECONTAINER_HPP
#define HW_06_FILECONTAINER_HPP


#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;

class FileContainer
{
public:
  string name;
  struct stat stats;
  char *data = nullptr;

  FileContainer() = default;
  explicit FileContainer(string &path, string &rootDir);

  string getIno();

  void saveToFile(int fd, bool saveContent=true);
  void loadFromFile(int fd, unordered_map<string, string> &inos);

  ~FileContainer();
};

#endif //HW_06_FILECONTAINER_HPP
