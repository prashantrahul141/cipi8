#pragma once
#include <raylib.h>

#include "external/nhlog.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iosfwd>
#include <memory>
#include <random>

const size_t ROM_START_ADDR = 0x200;
const size_t FONT_START_ADDR = 0x50;

const size_t VIDEO_WIDTH = 64;
const size_t VIDEO_HEIGHT = 32;

const unsigned int FONTSET_SIZE = 80;
const uint8_t FONTSET[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class Chip8 {
public:
  uint8_t registers[16]{};
  uint8_t memory[4096]{};
  uint16_t index{};
  uint16_t pc{};
  uint16_t stack[16];
  uint8_t sp{};
  uint8_t delay_timer{};
  uint8_t sound_timer{};
  uint8_t keypad[16]{};
  uint32_t display[64][32]{};
  uint16_t opcode;

public:
  Chip8(const char *filename);

private:
  /*
   * Reads rom file into the vm's memory.
   */
  void load_rom(const char *filename);

  /*
   * Clears display
   */
  inline void OP_00E0();

  /*
   * Returns from a subroutine
   */
  inline void OP_00EE();

  /*
   * jumps pc to nnn
   */
  inline void OP_1nnn();

  /*
   * calls the subroutine at 2nnn
   */
  inline void OP_2nnn();

  /*
   * skips next instruction if Vx = kk
   */
  inline void OP_3xkk();

  /*
   * skips next instruction if Vx != kk
   */
  inline void OP_4xkk();

  /*
   * skips next instruction if Vx = Vy
   */
  inline void OP_5xy0();

  /*
   * sets Vx = kk
   */
  inline void OP_6xkk();

  /*
   * sets Vx = Vx + kk
   */
  inline void OP_7xkk();

  /*
   * sets Vx = Vy
   */
  inline void OP_8xy0();

  /*
   * sets Vx = Vx OR Vy
   */
  inline void OP_8xy1();

  /*
   * sets Vx = Vx AND Vy
   */
  inline void OP_8xy2();

  /*
   * sets Vx = Vx XOR Vy
   */
  inline void OP_8xy3();

  /*
   * set Vx = Vx + Vy, set VF = carry
   */
  inline void OP_8xy4();

  /*
   * Set Vx = Vx - Vy, set VF = NOT borrow.
   */
  inline void OP_8xy5();

  /*
   * Set Vx = Vx SHR 1
   */
  inline void OP_8xy6();

private:
  std::default_random_engine rand_generator;
  std::uniform_int_distribution<uint8_t> rand_byte;
};
