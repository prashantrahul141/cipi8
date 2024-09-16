#include "chip8.h"
#include <cstdint>

Chip8::Chip8(const char *filename)
    : rand_generator(
          std::chrono::system_clock::now().time_since_epoch().count()) {

  // set pc to start of instructions.
  this->pc = 0x200;

  // load fonts into memory starting at 0x50.
  for (size_t i = 0; i < FONTSET_SIZE; i++) {
    memory[FONT_START_ADDR + i] = FONTSET[i];
  }

  // start loading rom into vm memory.
  this->load_rom(filename);

  // init rng
  this->rand_byte = std::uniform_int_distribution<uint8_t>(0, 255U);
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

// ======================================================
// ================= Instructions =======================
// ======================================================

/*
 * clears display
 */
inline void Chip8::OP_00E0() {
  std::memset(this->display, 0, sizeof(this->display));
}

/*
 * returns from a subroutine
 */
inline void Chip8::OP_00EE() {
  --this->sp;
  this->pc = this->stack[sp];
}

/*
 * jumps pc to nnn
 */
inline void Chip8::OP_1nnn() {
  uint16_t address = this->opcode & 0x0FFFu;
  this->pc = address;
}

/*
 * calls the subroutine at 2nnn
 */
inline void Chip8::OP_2nnn() {
  uint16_t address = this->opcode & 0x0FFFu;
  this->stack[sp] = this->pc;
  this->pc++;
  this->pc = address;
}

/*
 * skips next instruction if Vx = kk
 */
inline void Chip8::OP_3xkk() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t byte = this->opcode & 0x00FFu;
  if (this->registers[Vx] == byte) {
    this->pc += 2;
  }
}

/*
 * skips next instruction if Vx != kk
 */
inline void Chip8::OP_4xkk() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t byte = this->opcode & 0x00FFu;
  if (this->registers[Vx] != byte) {
    this->pc += 2;
  }
}

/*
 * skips next instruction if Vx = Vy
 */
inline void Chip8::OP_5xy0() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;
  if (this->registers[Vx] == this->registers[Vy]) {
    this->pc += 2;
  }
}

/*
 * sets Vx = kk
 */
inline void Chip8::OP_6xkk() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t byte = this->opcode & 0x00FFu;

  this->registers[Vx] = byte;
}

/*
 * sets Vx = Vx + kk
 */
inline void Chip8::OP_7xkk() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t byte = this->opcode & 0x00FFu;

  this->registers[Vx] += byte;
}

/*
 * sets Vx = Vy
 */
inline void Chip8::OP_8xy0() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;

  this->registers[Vx] = this->registers[Vy];
}

/*
 * sets Vx = Vx OR Vy
 */
inline void Chip8::OP_8xy1() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;

  this->registers[Vx] |= this->registers[Vy];
}

/*
 * sets Vx = Vx AND Vy
 */
inline void Chip8::OP_8xy2() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;

  this->registers[Vx] &= this->registers[Vy];
}

/*
 * sets Vx = Vx XOR Vy
 */
inline void Chip8::OP_8xy3() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;

  this->registers[Vx] ^= this->registers[Vy];
}

/*
 * set Vx = Vx + Vy, set VF = carry
 */
inline void Chip8::OP_8xy4() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;

  uint16_t sum = Vx + Vy;

  this->registers[0xF] = sum > 255 ? 1 : 0;
  this->registers[Vx] = sum & 0xFFu;
}

/*
 * set Vx = Vx - Vy, set VF = Not borrow
 */
inline void Chip8::OP_8xy5() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;

  this->registers[0xF] = this->registers[Vx] > this->registers[Vy] ? 1 : 0;
  this->registers[Vx] -= this->registers[Vy];
}
