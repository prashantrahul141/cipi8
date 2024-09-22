#include "app.h"

// constructor.
App::App(int argc, char *argv[]) {
#ifndef CIPI8_DEBUG_MODE
  nhlog_set_level(NHLOG_ERROR);
#else
  nhlog_set_level(NHLOG_TRACE);
#endif

  // parse command line arguments.
  argparse::ArgumentParser program("cipi8", "1.0.0");
  program.add_argument("rom_file").help("The rom file to run.").required();
  program.add_argument("--scale")
      .help("Scale of the display")
      .default_value(15)
      .scan<'i', int>();

  program.add_argument("--delay")
      .help("Delay between CPU cycles.")
      .default_value(6)
      .scan<'i', int>();

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << "Failed to parse arguments." << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  auto raw_filename = program.get<std::string>("rom_file");

  // if file doesnt exist
  if (!std::filesystem::exists(raw_filename) ||
      std::filesystem::is_directory(raw_filename)) {
    nhlog_error("Given file does not exist or is a directory.");
    exit(EXIT_FAILURE);
  }

  this->filename = raw_filename;
  this->delay = program.get<int>("--delay");
  this->scale = program.get<int>("--scale");
  nhlog_info("filename=%s, delay=%d, scale=%d", raw_filename, this->delay,
             this->scale);
}

// public driver
int App::run() {
  Chip8 chip8 = Chip8(this->filename);
  Platform platform = Platform("cipi8 - A Chip8 Emulator.", VIDEO_WIDTH * scale,
                               VIDEO_HEIGHT * scale, VIDEO_WIDTH, VIDEO_HEIGHT);

  int pitch = sizeof(chip8.display[0]) * VIDEO_WIDTH;
  auto last_cycle_time = std::chrono::high_resolution_clock::now();
  bool quit = false;

  while (!quit) {
    quit = platform.process_input(chip8.keypad);
    auto current_time = std::chrono::high_resolution_clock::now();
    float delta_time =
        std::chrono::duration<float, std::chrono::milliseconds::period>(
            current_time - last_cycle_time)
            .count();

    if (delta_time > this->delay) {
      last_cycle_time = current_time;
      chip8.Cycle();
      platform.update(chip8.display, pitch);
    }
  }

  return EXIT_SUCCESS;
}
