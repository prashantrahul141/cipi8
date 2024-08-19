#include <cstdint>

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
  uint16_t opcode{};

public:
  Chip8();

private:
  void load_rom();
};
