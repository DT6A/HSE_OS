#include <zconf.h>
#include <sys/stat.h>
#include <stdexcept>
#include <fcntl.h>
#include <cstring>

#include "FileContainer.hpp"

FileContainer::FileContainer(string &path, string &rootDir)
{
  name = path;
  if (lstat((rootDir + "/" + path).c_str(), &stats) == -1)
  {
    throw runtime_error("Failed to read stats about file");
  }
  if (S_ISLNK(stats.st_mode))
  {
    size_t dataSize = stats.st_size + 1;
    if (stats.st_size == 0)
      dataSize = PATH_MAX;
    data = new char[dataSize];
    ssize_t nbytes = readlink((rootDir + "/" + path).c_str(), data, dataSize);
    if (nbytes == -1) throw runtime_error("Error while reading sym link content");
    data[nbytes] = 0;
  }
  else if (!(S_ISDIR(stats.st_mode) || S_ISFIFO(stats.st_mode)))
  {
    data = new char[stats.st_size];

    int fd = open((rootDir + "/" + path).c_str(), O_RDONLY);
    if (fd == -1)
      throw runtime_error("Failed to create output file");
    if (read(fd, data, stats.st_size) != stats.st_size) throw runtime_error("Failed to read file content");
    if (close(fd) != 0)
      throw runtime_error("Failure while closing file");
  }
}

void FileContainer::saveToFile(int fd, bool saveContent)
{
  std::size_t nameSize = name.size();

  if (write(fd, reinterpret_cast<const char *>(&nameSize), sizeof(nameSize)) == -1) throw runtime_error("Error while writing to file");
  if (write(fd, name.c_str(), name.size()) == -1) throw runtime_error("Error while writing to file");
  if (write(fd, reinterpret_cast<const char *>(&stats), sizeof(stats)) == -1) throw runtime_error("Error while writing to file");
  if (saveContent && !(S_ISDIR(stats.st_mode) || S_ISFIFO(stats.st_mode)))
    if (write(fd, data, stats.st_size) == -1) throw runtime_error("Error while writing to file");
}

void FileContainer::loadFromFile(int fd, unordered_map<string, string> &inos)
{
  std::size_t nameSize;

  if (read(fd, reinterpret_cast<char *>(&nameSize), sizeof(nameSize)) == -1) throw runtime_error("Error while reading from file");
  char *buf = new char[nameSize + 1];
  memset(buf, 0, nameSize + 1);
  int br = read(fd, buf, nameSize);
  if (br == -1) throw runtime_error("Error while reading from file");
  name = buf;
  //delete[] buf;
  if (read(fd, reinterpret_cast<char *>(&stats), sizeof(stats)) == -1) throw runtime_error("Error while reading from file");
  if (inos.find(getIno()) == inos.end())
  {
    inos[getIno()] = "";
    data = new char[stats.st_size + 1];
    memset(data, 0, stats.st_size + 1);
    if (!S_ISDIR(stats.st_mode))
      if (read(fd, data, stats.st_size) == -1) throw runtime_error("Error while reading from file");
  }
}

string FileContainer::getIno()
{
  return to_string(stats.st_rdev) + string(":") + to_string(stats.st_ino);
}

FileContainer::~FileContainer()
{
  delete[] data;
}
