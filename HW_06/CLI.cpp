//
// Created by tarasov on 05.06.2021.
//

#include <stdexcept>
#include <zconf.h>

#include "CLI.hpp"
#include "Tar.hpp"

using namespace std;

CLI::CLI(int argc, char **argv)
{
  if (argc < 3 || argc > 5)
    throw runtime_error("Wrong number of arguments");

  archive = string(argv[argc - 1]);
  for (int i = 1; i < argc - 1; ++i)
  {
    string arg(argv[i]);
    if (arg == "-c" || arg == "--create")
      mode = CREATE;
    else if (arg == "-x" || arg == "--extract")
      mode = EXTRACT;
    else if (arg == "-d" || arg == "--directory")
    {
      if (i == argc - 2)
        throw runtime_error("Wrong number of arguments");
      directory = string(argv[++i]);
    }
    else
      throw runtime_error("Invalid argument");
  }
}

void CLI::run()
{
  if (mode == NONE || archive == "")
    throw runtime_error("Invalid arguments were passed");

  Tar tar;
  if (mode == CREATE)
    try
    {
      tar.create(directory, archive);
    }
    catch (exception e)
    {
      if (unlink(archive.c_str()) != 0)
        throw runtime_error("Everything is bad " + to_string(errno));
      throw e;
    }
  else
    tar.extract(archive, directory);
}
