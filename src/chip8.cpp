#include "chip8.h"

Chip8::Chip8(const char *filename) {
  // set pc to start of instructions.
  this->pc = 0x200;

  // load fonts into memory starting at 0x50.
  for (size_t i = 0; i < FONTSET_SIZE; i++) {
    memory[FONT_START_ADDR + i] = FONTSET[i];
  }

  // start loading rom into vm memory.
  this->load_rom(filename);
}

void Chip8::load_rom(const char *filename) {
  nhlog_trace("loading rom into memory.");
  // open file
  std::ifstream file(filename, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
    nhlog_error("Failed to open file exiting.");
  }

  // alloc a buffer size of the rom size.
  std::streampos size = file.tellg();
  std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);

  // seek to begin, read data from file to buffer.
  file.seekg(0, std::ios::beg);
  file.read(buffer.get(), size);
  file.close();

  // read data from buffer to vm's memory
  for (size_t i = 0; i < size; i++) {
    this->memory[ROM_START_ADDR + i] = buffer[i];
  }

  nhlog_trace("loaded rom into memory.");
}
