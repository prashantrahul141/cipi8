#include "chip8.h"
#include <cstdint>

Chip8::Chip8(std::string filename)
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

  // setup function pointers table for each type of instruction.
  this->table[0x0] = &Chip8::Tabel_0;
  this->table[0x1] = &Chip8::OP_1nnn;
  this->table[0x2] = &Chip8::OP_2nnn;
  this->table[0x3] = &Chip8::OP_3xkk;
  this->table[0x4] = &Chip8::OP_4xkk;
  this->table[0x5] = &Chip8::OP_5xy0;
  this->table[0x6] = &Chip8::OP_6xkk;
  this->table[0x7] = &Chip8::OP_7xkk;
  this->table[0x8] = &Chip8::Table_8;
  this->table[0x9] = &Chip8::OP_9xy0;
  this->table[0xA] = &Chip8::OP_Annn;
  this->table[0xB] = &Chip8::OP_Bnnn;
  this->table[0xC] = &Chip8::OP_Cxkk;
  this->table[0xD] = &Chip8::OP_Dxyn;
  this->table[0xE] = &Chip8::Table_E;
  this->table[0xF] = &Chip8::Table_F;

  // by default make all function pointers in the following
  // table point to OP_NULL.
  for (size_t i = 0; i <= 0xE; i++) {
    this->table_0[i] = &Chip8::OP_NULL;
    this->table_8[i] = &Chip8::OP_NULL;
    this->table_E[i] = &Chip8::OP_NULL;
  }

  // then overwrite the opcode indices we want to use.
  this->table_0[0x0] = &Chip8::OP_00E0;
  this->table_0[0xE] = &Chip8::OP_00EE;

  this->table_8[0x0] = &Chip8::OP_8xy0;
  this->table_8[0x1] = &Chip8::OP_8xy1;
  this->table_8[0x2] = &Chip8::OP_8xy2;
  this->table_8[0x3] = &Chip8::OP_8xy3;
  this->table_8[0x4] = &Chip8::OP_8xy4;
  this->table_8[0x5] = &Chip8::OP_8xy5;
  this->table_8[0x6] = &Chip8::OP_8xy6;
  this->table_8[0x7] = &Chip8::OP_8xy7;
  this->table_8[0xE] = &Chip8::OP_8xyE;

  this->table_E[0x1] = &Chip8::OP_ExA1;
  this->table_E[0xE] = &Chip8::OP_Ex9E;

  for (size_t i = 0; i <= 0x65; i++) {
    this->table_F[i] = &Chip8::OP_NULL;
  }

  this->table_F[0x07] = &Chip8::OP_Fx07;
  this->table_F[0x0A] = &Chip8::OP_Fx0A;
  this->table_F[0x15] = &Chip8::OP_Fx15;
  this->table_F[0x18] = &Chip8::OP_Fx18;
  this->table_F[0x1E] = &Chip8::OP_Fx1E;
  this->table_F[0x29] = &Chip8::OP_Fx29;
  this->table_F[0x33] = &Chip8::OP_Fx33;
  this->table_F[0x55] = &Chip8::OP_Fx55;
  this->table_F[0x65] = &Chip8::OP_Fx65;
}

/*
 * Fetch, Decode, Execute.
 */
void Chip8::Cycle() {
  // fetch instruction
  this->opcode = (this->memory[this->pc] << 8u) | this->memory[this->pc + 1];

  // increment this before executing
  this->pc += 2;

  // decode and execute
  ((*this).*(table[(this->opcode & 0xF000u) >> 12u]))();

  // decrement delay and sound timer
  if (this->delay_timer > 0) {
    --delay_timer;
  }

  if (this->sound_timer > 0) {
    --sound_timer;
  }
}

void Chip8::load_rom(std::string filename) {
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

/*
 * Functions corresponding to each instruction table
 */
inline void Chip8::Tabel_0() {
  // We deref `this`,
  //
  // Access the table_0 array and get the function
  // pointer using `opcode & 0x000Fu` as index.
  //
  // Deref that function pointer and then call it.
  ((*this).*(table_0[this->opcode & 0x000Fu]))();
}

inline void Chip8::Table_8() { ((*this).*(table_8[this->opcode & 0x000Fu]))(); }
inline void Chip8::Table_E() { ((*this).*(table_E[this->opcode & 0x000Fu]))(); }
inline void Chip8::Table_F() { ((*this).*(table_F[this->opcode & 0x00FFu]))(); }

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
  this->sp++;
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

  uint16_t sum = this->registers[Vx] + this->registers[Vy];

  this->registers[0xF] = sum > 255U ? 1 : 0;
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
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t Vy = (this->opcode & 0x00F0u) >> 4u;

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
  uint8_t yPos = this->registers[Vy] % VIDEO_HEIGHT;

  this->registers[0xF] = 0;

  for (size_t row = 0; row < height; ++row) {
    uint8_t sprite_byte = this->memory[index + row];
    for (size_t col = 0; col < 8; ++col) {
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
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
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

/*
 * Set delay timer = Vx.
 */
inline void Chip8::OP_Fx15() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  this->delay_timer = this->registers[Vx];
}

/*
 * Set sound timer = Vx.
 */
inline void Chip8::OP_Fx18() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  this->sound_timer = this->registers[Vx];
}

/*
 * Set I = I + Vx.
 */
inline void Chip8::OP_Fx1E() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  this->index += this->registers[Vx];
}

/*
 * Set I = location of sprite for digit Vx.
 */
inline void Chip8::OP_Fx29() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t digit = this->registers[Vx];
  this->index = FONT_START_ADDR + (5 * digit);
}

/*
 * Store BCD representation of Vx in memory locations I, I+1, and I+2.
 */
inline void Chip8::OP_Fx33() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  uint8_t value = this->registers[Vx];

  this->memory[index + 2] = value % 10;
  value /= 10;

  this->memory[index + 1] = value % 10;
  value /= 10;

  this->memory[index] = value % 10;
}

/*
 * Store registers V0 through Vx in memory starting at location I.
 */
inline void Chip8::OP_Fx55() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  for (uint8_t i = 0; i <= Vx; ++i) {
    this->memory[this->index + i] = this->registers[i];
  }
}

/*
 * Read registers V0 through Vx from memory starting at location I.
 */
inline void Chip8::OP_Fx65() {
  uint8_t Vx = (this->opcode & 0x0F00u) >> 8u;
  for (uint8_t i = 0; i <= Vx; ++i) {
    this->registers[i] = this->memory[this->index + i];
  }
}

/*
 * does nothing, for instructions which are not supported.
 */
inline void Chip8::OP_NULL() {}
