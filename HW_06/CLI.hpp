//
// Created by tarasov on 05.06.2021.
//

#ifndef HW_06_CLI_HPP
#define HW_06_CLI_HPP

#include <string>

using namespace std;

class CLI
{
public:
  CLI(int argc, char **argv);
  void run();
private:
  enum
  {
    NONE,
    CREATE,
    EXTRACT,
  } mode = NONE;
  string archive = "";
  string directory = "./";

};


#endif //HW_06_CLI_HPP
