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

/*
 * Set Vx = Vx SHR 1
 */
inline void Chip8::OP_8xy6() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  this->registers[0xF] = (this->registers[Vx] & 0x1u);
  this->registers[Vx] >>= 1;
}

/*
 * Set Vx = Vy - Vx, set VF = NOT borrow
 */
inline void Chip8::OP_8xy7() {
  uint8_t Vx = (opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (opcode & 0x00F0u) >> 4u;

  this->registers[0xF] = this->registers[Vy] > this->registers[Vx] ? 1 : 0;
  this->registers[Vx] = this->registers[Vy] - this->registers[Vx];
}

/*
 * Set Vx = Vx SHL 1
 */
inline void Chip8::OP_8xyE() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;

  // save msb in VF.
  this->registers[0xF] = (this->registers[Vx] & 0x80u) >> 7u;
  this->registers[Vx] <<= 1;
}

/*
 * Skip next instruction if Vx != Vy
 */
inline void Chip8::OP_9xy0() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;

  if (this->registers[Vx] != this->registers[Vy]) {
    this->pc += 2;
  }
}

/*
 * Set Index = nnn;
 */
inline void Chip8::OP_Annn() {
  uint16_t address = this->opcode & 0x0FFFu;
  this->index = address;
}

/*
 * Jump to location nnn + V0
 */
inline void Chip8::OP_Bnnn() {
  uint16_t address = this->opcode & 0x0FFFu;
  this->pc = this->registers[0] + address;
}

/*
 * Set Vx = random byte & KK.
 */
inline void Chip8::OP_Cxkk() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t byte = this->opcode & 0x00FFu;

  this->registers[Vx] = this->rand_byte(this->rand_generator) & byte;
}

/*
 * display n-byte sprite starting at memory location I at (Vx, Vy).
 * set VF = collision.
 */
inline void Chip8::OP_Dxyn() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;
  uint8_t height = this->opcode & 0x000Fu;

  uint8_t xPos = this->registers[Vx] % VIDEO_WIDTH;
  uint8_t yPos = this->registers[Vx] % VIDEO_HEIGHT;

  this->registers[0xF] = 0;

  for (size_t row = 0; row < height; ++row) {
    uint8_t sprite_byte = this->memory[index + row];
    for (unsigned int col = 0; col < 8; ++col) {
      uint8_t sprite_pixel = sprite_byte & (0x80u >> col);
      uint32_t *screen_pixel =
          &this->display[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

      // if pixel is on
      if (sprite_pixel) {
        // screen pixel also on - collision
        if (*screen_pixel == 0xFFFFFFFF) {
          registers[0xF] = 1;
        }

        // XOR with the sprite pixel
        *screen_pixel ^= 0xFFFFFFFF;
      }
    }
  }
}

/*
 * skip next instruction if key with the value of Vx is pressed.
 */
inline void Chip8::OP_Ex9E() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t key = this->registers[Vx];
  if (this->keypad[key]) {
    pc += 2;
  }
}

/*
 * skip next instruction if key with the value of Vx is not pressed.
 */
inline void Chip8::OP_ExA1() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t key = this->registers[Vx];
  if (!this->keypad[key]) {
    pc += 2;
  }
}

/*
 * Set Vx = delay timer value.
 */
inline void Chip8::OP_Fx07() {
  uint8_t Vx = (this->opcode * 0x0F00u) >> 8u;
  this->registers[Vx] = this->delay_timer;
}

/*
 * Wait for a key press, store the value of the key in Vx.
 */
inline void Chip8::OP_Fx0A() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  if (this->keypad[0]) {
    this->registers[Vx] = 0;
  } else if (this->keypad[1]) {
    this->registers[Vx] = 1;
  } else if (this->keypad[2]) {
    this->registers[Vx] = 2;
  } else if (this->keypad[3]) {
    this->registers[Vx] = 3;
  } else if (this->keypad[4]) {
    this->registers[Vx] = 4;
  } else if (this->keypad[5]) {
    this->registers[Vx] = 5;
  } else if (this->keypad[6]) {
    this->registers[Vx] = 6;
  } else if (this->keypad[7]) {
    this->registers[Vx] = 7;
  } else if (this->keypad[8]) {
    this->registers[Vx] = 8;
  } else if (this->keypad[9]) {
    this->registers[Vx] = 9;
  } else if (this->keypad[10]) {
    this->registers[Vx] = 10;
  } else if (this->keypad[11]) {
    this->registers[Vx] = 11;
  } else if (this->keypad[12]) {
    this->registers[Vx] = 12;
  } else if (this->keypad[13]) {
    this->registers[Vx] = 13;
  } else if (this->keypad[14]) {
    this->registers[Vx] = 14;
  } else if (this->keypad[15]) {
    this->registers[Vx] = 15;
  } else {
    this->pc -= 2;
  }
}
