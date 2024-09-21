#include "app.h"

// constructor.
App::App(int argc, char *argv[]) {
  // parse command line arguments.
  argparse::ArgumentParser program("cipi8", "1.0.0");
  program.add_argument("rom_file").help("The rom file to run.").required();
  program.add_argument("--scale")
      .help("Scale of the display")
      .default_value(10)
      .scan<'i', int>();

  program.add_argument("--delay")
      .help("Delay between CPU cycles.")
      .default_value(1)
      .scan<'i', int>();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  auto raw_filename = program.get<std::string>("rom_file");

  // if file doesnt exist
  if (!std::filesystem::exists(raw_filename)) {
    nhlog_error("Given file does not exist.");
    exit(EXIT_FAILURE);
  }

  this->filename = argv[1];
}

// public driver
void App::run() { Chip8 chip8 = Chip8(this->filename); }
