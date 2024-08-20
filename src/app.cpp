#include "app.h"

// constructor.
App::App(int argc, char *argv[]) {
  // no file given.
  if (argc < 2) {
    nhlog_error("No rom file given.");
    exit(1);
  }

  // file doesnt exist
  if (!std::filesystem::exists(argv[1])) {
    nhlog_error("Given file does not exist.");
    exit(1);
  }

  this->filename = argv[1];
}

// public driver
void App::run() { Chip8 chip8 = Chip8(this->filename); }
