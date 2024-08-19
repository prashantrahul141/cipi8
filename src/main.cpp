#include "app.h"
#include <cstdlib>

int main(int argc, char *argv[]) {
  App app = App(argc, argv);
  app.run();
  return EXIT_SUCCESS;
}
