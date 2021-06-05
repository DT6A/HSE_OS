#include "CLI.hpp"

using namespace std;

int main(int argc, char **argv)
{
  CLI cli(argc, argv);
  cli.run();
  return 0;
}
