#pragma once

#include "chip8.h"
#include "external/argparse.hpp"
#include "external/nhlog.h"
#include "platform.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>

class App {
public:
  std::string filename;
  int scale;
  int delay;

public:
  App(int argc, char *argv[]);
  int run();
};
