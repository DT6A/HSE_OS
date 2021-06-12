#include <dirent.h>
#include <sys/stat.h>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>

#include "Tar.hpp"

// НЕ НАДО ЧИТАТЬ ЭТОТ КОД, ЕСЛИ НЕ ХОТИТЕ РАЗОЧАРОВАТЬСЯ В ЧЕЛОВЕЧЕСТВЕ


// АПИ УЖАСНОЕ, ДОКУМЕНТАЦИЯ КРИВАЯ, РАБОТАЕТ НЕПОНЯТНО КАК, НЕПОНЯТНО ПОЧЕМУ ЧТО-ТО НЕ РАБОТАЕТ
// ОТВРАТИТЕЛЬНОЕ ЗАДАНИЕ, ТОЛЬКО ЕСЛИ ЕГО ЦЕЛЬ НЕ ЗАСТАВИТЬ ЧУВСТОВАТЬ ЗАПАХ ГОРЯЩЕГО СТУЛА, БОЛЬ, ОТВРАЩЕНИЕ И ПРОЧИЕ РАДУЖНЫЕ ЭМОЦИИ

void Tar::create(string &dir, string &target)
{
  collectNames(dir, dir);
  saveToTarget(target, dir);
}

void Tar::extract(string &path, string &target)
{
  loadFromTarget(path, target);
  restoreFiles(target);
}

void Tar::collectNames(string &dir, string &rootDir, string prefix)
{
  DIR *d = opendir(dir.c_str());
  struct dirent *dd;
  while ((dd = readdir(d))) {
    string dn(dd->d_name);
    if (dn == "." || dn == "..") continue;
    string path = dir + "/" + dn;
    struct stat st;
    if (lstat(path.c_str(), &st) < 0) throw runtime_error("Failed to read stats about file" + to_string(errno));
    if (S_ISSOCK(st.st_mode))
      continue;
    names.emplace_back(prefix + dn);
    if (S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode))
    {
      collectNames(path, rootDir, prefix + dn + "/");
    }
  }
  closedir(d);
}

void Tar::saveToTarget(string &target, string &rootDir)
{
  int fileDescr = open(target.c_str(), O_RDWR | O_CREAT | O_TRUNC,
          S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if (fileDescr == -1) throw runtime_error("Failed to create output file " + to_string(errno));

  size_t size = names.size();

  if (write(fileDescr, reinterpret_cast<const char *>(&size), sizeof(size)) != sizeof(size))
    throw runtime_error("Failed to write to target file " + to_string(errno));
  for (string &name : names)
  {
    FileContainer fc(name, rootDir);
    if (ino2file.find(fc.getIno()) == ino2file.end())
    {
      fc.saveToFile(fileDescr);
      ino2file[fc.getIno()] = name;
    }
    else
      fc.saveToFile(fileDescr, false);
  }
  if (close(fileDescr) !=0) throw runtime_error("Failure while closing file " + to_string(errno));
}

void Tar::loadFromTarget(string &path, string &target)
{
  unordered_map<string, string> inos;
  int fileDescr = open(path.c_str(), O_RDONLY);
  if (fileDescr == -1) throw runtime_error("Failed to create output file " + to_string(errno));

  size_t size;
  read(fileDescr, reinterpret_cast<char *>(&size), sizeof(size));
  containers.resize(size);
  for (size_t i = 0; i < size; ++i)
  {
    containers[i].loadFromFile(fileDescr, inos);
  }

  close(fileDescr);
}

void Tar::restoreFiles(string &path)
{
  for (auto &container : containers)
  {
    if (S_ISDIR(container.stats.st_mode))
    {
      if (mkdir((path + container.name).c_str(), container.stats.st_mode) != 0)
        throw runtime_error("Failed to create directory " + to_string(errno));
      if (chown((path + container.name).c_str(), container.stats.st_uid, container.stats.st_gid) != 0)
        throw runtime_error("Failed to change directory's owners " + to_string(errno));
    }
    else if (S_ISLNK(container.stats.st_mode))
    {
      if (container.data != nullptr)
      {
        if (symlink(container.data, (path + container.name).c_str()) == -1)
          throw runtime_error("Failed to create sym link " + to_string(errno));
        ino2file[container.getIno()] = container.name;
      }
      else
      {
        if (link((path + ino2file[container.getIno()]).c_str(),
                 (path + container.name).c_str()) != 0)
          throw runtime_error("Failed to create hard link to sym link" + to_string(errno));
      }
      if (lchown((path + container.name).c_str(), container.stats.st_uid, container.stats.st_gid))
        throw runtime_error("Failed to change sym link's owners " + to_string(errno));
    }
    else if (S_ISFIFO(container.stats.st_mode))
    {
      if (mkfifo((path + container.name).c_str(), container.stats.st_mode) == -1)
        throw runtime_error("Failed to create FIFO " + to_string(errno));
      if (chown((path + container.name).c_str(), container.stats.st_uid, container.stats.st_gid) != 0)
        throw runtime_error("Failed to change FIFO's owners " + to_string(errno));
    }
    else
    {
      int fileDescr;
      if (ino2file.find(container.getIno()) == ino2file.end())
      {
        fileDescr = open((path + container.name).c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                         container.stats.st_mode);
        ino2file[container.getIno()] = container.name;
        if (write(fileDescr, container.data, container.stats.st_size) == -1)
          throw runtime_error("Error while writing content to file " + to_string(errno));
      }
      else
      {
        if (link((path + ino2file[container.getIno()]).c_str(),
                 (path + container.name).c_str()) != 0)
          throw runtime_error("Failed to create hard link " + to_string(errno));
        fileDescr = open((path + container.name).c_str(), O_RDWR);
      }
      if (fileDescr == -1)
        throw runtime_error("Failed to create output file " + to_string(errno));
      if (fchown(fileDescr, container.stats.st_uid, container.stats.st_gid) != 0)
        throw runtime_error("Failed to change file's owners " + to_string(errno));
      if (close(fileDescr) != 0) throw runtime_error("Failure while closing file" + to_string(errno));
    }
    if (!S_ISLNK(container.stats.st_mode))
      if (chmod((path + container.name).c_str(), container.stats.st_mode) != 0)
        throw runtime_error("Failed to change file's permissions " + to_string(errno));
  }
  for (auto i = containers.rbegin(); i != containers.rend(); ++i)
  {
    FileContainer &container = *i;
    struct timespec buf[2] = {container.stats.st_atim, container.stats.st_mtim};

    if (utimensat(AT_FDCWD, (path + container.name).c_str(), buf, AT_SYMLINK_NOFOLLOW) != 0)
      throw runtime_error("Failed to modify directory times " + to_string(errno));
  }
}
