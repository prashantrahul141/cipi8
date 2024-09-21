#pragma once

#include "chip8.h"
#include "external/argparse.hpp"
#include "external/nhlog.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>

#define USAGE "USAGE:\n\tcipi8 [filename]"

class App {
public:
  char *filename;

public:
  App(int argc, char *argv[]);
  void run();
};
