#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>


#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32

struct chip8_state
{
  uint8_t memory[4096];
  uint8_t display[64 * 32];
  uint16_t stack[16];
  uint8_t V[16];
  uint8_t input[16];
  uint8_t waiting_input[16];
  uint16_t I;
  uint16_t pc;
  uint8_t sp;
  uint8_t delay_timer;
  uint8_t sound_timer;
  uint8_t draw_flag;
};

struct chip8_state* new_chip8();
void load_program(struct chip8_state* state, char* program_path);
void delete_chip8(struct chip8_state* state);
void chip8_cycle();


#endif