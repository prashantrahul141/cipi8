#include "app.h"
#include "chip8.h"
#include <filesystem>

// constructor.
App::App(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "No rom file given.\n" << USAGE << std::endl;
    exit(1);
  }

  if (!std::filesystem::exists(argv[1])) {
    std::cout << "Given file does not exist." << std::endl;
    exit(1);
  }

  this->filename = argv[1];
}

// public driver
void App::run() {}
