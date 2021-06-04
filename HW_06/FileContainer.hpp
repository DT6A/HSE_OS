#ifndef HW_06_FILECONTAINER_HPP
#define HW_06_FILECONTAINER_HPP


#include <sys/stat.h>
#include <iostream>
#include <fstream>

using namespace std;

class FileContainer
{
public:
  string name;
  struct stat stats;
  char *data;

  FileContainer() = default;
  explicit FileContainer(string &path, string &rootDir);

  void saveToFile(int fd, bool saveContent=true);
  void loadFromFile(int fd);
};

#endif //HW_06_FILECONTAINER_HPP
