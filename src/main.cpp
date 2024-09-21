#include "app.h"
#include <cstdlib>

int main(int argc, char *argv[]) {
  App app = App(argc, argv);
  return app.run();
}
