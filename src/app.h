#include <cstdlib>
#include <iostream>

#define USAGE "USAGE:\n\tcipi8 [filename]"

class App {
public:
  char *filename;

public:
  App(int argc, char *argv[]);
  void run();
};
