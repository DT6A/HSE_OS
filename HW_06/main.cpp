#include <iostream>
#include <fstream>

#include "FileContainer.hpp"
#include "Tar.hpp"
using namespace std;

int main()
{
  Tar t;
  string dir = "test-dir/";
  string target = "test-dir.tar";
  string out = "output-dir/";

  //t.create(dir, target);
  t.extract(target, out);

  return 0;
}
