#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct chip8_state* new_chip8()
{
  struct chip8_state* state = malloc(sizeof(struct chip8_state));

  state->I = 0;
  state->pc = 0x200;
  state->sp = 0;
  state->delay_timer = 0;
  state->sound_timer = 0;
  state->draw_flag = 0;

  for (int i = 0; i < 0xF; ++i)
  {
    state->V[i] = 0;
    state->input[i] = 0;
    state->waiting_input[i] = 0;
  }

  for (int i = 0; i < CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT; ++i)
  {
    state->display[i] = 0;
  }

  // Load fontset
  uint8_t fontset[] = {
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

  memcpy(state->memory, fontset, sizeof(fontset));

  time_t t;
  srand((unsigned) time(&t));

  return state;
}

void load_program(struct chip8_state* state, char* program_path)
{
  FILE* file = fopen(program_path, "rb");
  if (file == NULL)
  {
    perror("Error");
    return;
  }
  int i = 0x200;
  unsigned char c;
  while (1)
  {
    c = fgetc(file);
    if (feof(file))
    {
      break;
    }
    state->memory[i] = c;
    i += 1;
  }
}

void delete_chip8(struct chip8_state* state)
{
  free(state);
}

void chip8_cycle(struct chip8_state* state)
{
  uint16_t opcode = state->memory[state->pc] << 8 | state->memory[state->pc + 1];
  //printf("OPCODE: %04X PC: %04X\n", opcode, state->pc);
  state->pc += 2;

  // For full opcode definitions -> https://en.wikipedia.org/wiki/CHIP-8#Opcode_table
  switch (opcode & 0xF000)
  {
    case 0x0000:
      switch (opcode)
      {
        // 0x00E0 CLS - Clear the display.
        case 0x00E0:
          for (int i = 0; i < 64 * 32; ++i)
          {
            state->display[i] = 0;
          }
          state->draw_flag = 1;
          break;
        
        // 0x00EE RET - Return from a subroutine.
        case 0x00EE:
          state->sp -= 1;
          state->pc = state->stack[state->sp];
          break;
        
        default:
          printf("(ERROR) Unknown opcode: 0x%4X\n", opcode);
          break;
      }
      break;

    // 0x1nnn JP addr - Jump to location nnn
    case 0x1000:
      state->pc = opcode & 0x0FFF;
      break;

    // 0x2nnn CALL addr - Call subroutine at nnn.
    case 0x2000:
      state->stack[state->sp] = state->pc;
      state->sp += 1;
      state->pc = opcode & 0x0FFF;
      break;

    // 0x3xkk SE Vx, byte - Skip next instruction if Vx = kk.
    case 0x3000:
    {
      uint8_t x = (opcode & 0x0F00) >> 8;
      if (state->V[x] == (opcode & 0x00FF))
      {
        state->pc += 2;
      }
      break;
    }

    // 0x4xkk SNE Vx, byte - Skip next instruction if Vx != kk.
    case 0x4000:
    {
      uint8_t x = (opcode & 0x0F00) >> 8;
      if (state->V[x] != (opcode & 0x00FF))
      {
        state->pc += 2;
      }
      break;
    }

    // 0x5xy0 SE Vx, Vy - Skip next instruction if Vx = Vy.
    case 0x5000:
    {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;
      if (state->V[x] == state->V[y])
      {
        state->pc += 2;
      }
      break;
    }

    // 0x6xkk LD Vx, byte - Set Vx = kk
    case 0x6000:
    {
      uint8_t x = (opcode & 0x0F00) >> 8;
      state->V[x] = opcode & 0x00FF;
      break;
    }

    // 0x7xkk ADD Vx, byte - Set Vx = Vx + kk
    case 0x7000:
    {
      uint8_t x = (opcode & 0x0F00) >> 8;
      state->V[x] += opcode & 0x00FF;
      break;
    }

    case 0x8000:
    {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;

      switch (opcode & 0x000F)
      {
        // 0x8xy0 LD Vx, Vy - Stores the value of register Vy in register Vx.
        case 0x0000:
          state->V[x] = state->V[y];
          break;
        
        // 0x8xy1 OR Vx, Vy - Set Vx = Vx OR Vy
        case 0x0001:
          state->V[x] |= state->V[y];
          break;

        // 0x8xy2 AND Vx, Vy - Set Vx = Vx AND Vy.
        case 0x0002:
          state->V[x] &= state->V[y];
          break;

        // 0x8xy3 XOR Vx, Vy - Set Vx = Vx XOR Vy.
        case 0x0003:
          state->V[x] ^= state->V[y];
          break;

        // 0x8xy4 ADD Vx, Vy - Set Vx = Vx + Vy, set VF = carry
        case 0x0004:
        {
          uint16_t result = state->V[x] + state->V[y];
          state->V[0xF] = (result & 0x0100) >> 8;
          state->V[x] = result & 0x00FF;
          break;
        }

        // 0x8xy5 SUB Vx, Vy - Set Vx = Vx - Vy, set VF = NOT borrow.
        case 0x0005:
          if (state->V[x] > state->V[y])
          {
            state->V[0xF] = 1;
          }
          else
          {
            state->V[0xF] = 0;
          }
          state->V[x] -= state->V[y];
          break;

        // 0x8xy6 SHR Vx {, Vy} - Set Vx = Vx >> 1.
        case 0x0006:
          state->V[0xF] = state->V[x] & 0x01;
          state->V[x] >>= 1;
          break;

        // 0x8xy7 SUBN Vx, Vy - Set Vx = Vy - Vx, set VF = NOT borrow.
        case 0x0007:
          if (state->V[y] > state->V[x])
          {
            state->V[0xF] = 1;
          }
          else
          {
            state->V[0xF] = 0;
          }
          state->V[x] = state->V[y] - state->V[x];
          break;

        // 0x8xyE SHL Vx {, Vy} - Set Vx = Vx << 1.
        case 0x000E:
          state->V[0xF] = state->V[x] & 0x80;
          state->V[x] <<= 1;
          break;

        default:
          printf("(ERROR) Unknown opcode: 0x%4X\n", opcode);
      }
      break;
    }
    
    // 0x9xy0 SNE Vx, Vy - Skip next instruction if Vx != Vy.
    case 0x9000:
    {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;
      if (state->V[x] != state->V[y])
      {
        state->pc += 2;
      }
      break;
    }

    // 0xAnnn LD I, addr - Set I = nnn.
    case 0xA000:
      state->I = opcode & 0x0FFF;
      break;

    // 0xBnnn JP V0, addr - Jump to location nnn + V0.
    case 0xB000:
      state->pc = opcode & 0x0FFF + state->V[0];
      break;

    // 0xCxkk RND Vx, byte - Set Vx = random byte AND kk.
    case 0xC000:
    {
      uint8_t x = (opcode & 0x0F00) >> 8;
      state->V[x] = (rand() % 256) & (opcode & 0x00FF);
      break;
    }

    // 0xDxyn DRW Vx, Vy, nibble - Display n-byte sprite starting at memory location I at (Vx, Vy), set Vf = collision.
    case 0xD000:
    {
      uint8_t x = (opcode & 0x0F00) >> 8;
      uint8_t y = (opcode & 0x00F0) >> 4;
      state->V[0xF] = 0;
      for (int i = 0; i < (opcode & 0x000F); ++i)
      {
        for (int j = 0; j < 8; ++j)
        {
          int screen_x = state->V[x] + j;
          int screen_y = state->V[y] + i;

          screen_x %= 64;
          screen_y %= 32;

          uint8_t sprite_mask = 1 << (7 - j);
          uint8_t sprite_value = (state->memory[state->I + i] & sprite_mask) >> (7 - j);

          state->V[0xF] |= sprite_value & state->display[screen_y * 64 + screen_x];
          state->display[screen_y * 64 + screen_x] ^= sprite_value;
        }
      }
      state->draw_flag = 1;
      break;
    }

    case 0xE000:
      switch (opcode & 0x000F)
      {
        // 0xEx9E SKP Vx - Skip next instruction if key with the value of Vx is pressed.
        case 0x000E:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          state->pc += state->input[state->V[x]] * 2;
          break;
        }

        // 0xExA1 SKNP Vx - Skip next instruction if key with the value of Vx is not pressed. 
        case 0x0001:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          state->pc += (1 - state->input[state->V[x]]) * 2;
          break;
        }

        default:
          printf("(ERROR) Unknown opcode: 0x%4X\n", opcode);
      }
      break;

    case 0xF000:
      switch (opcode & 0x00FF)
      {
        // 0xFx07 LD Vx, DT - Set Vx = delay timer value.
        case 0x0007:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          state->V[x] = state->delay_timer;
          break;
        }

        // 0xFx0A - LD Vx, K - Wait for a key press, store the value of the key in Vx.
        case 0x000A:
        {
          state->pc -= 2;
          for (int i = 0; i < sizeof(state->waiting_input); ++i)
          {
            if (state->waiting_input[i] == 0 && (state->input[i] == 1))
            {
              uint8_t x = (opcode & 0x0F00) >> 8;
              state->V[x] = i;
              state->pc += 2;
              break;
            }
          }


          memcpy(state->waiting_input, state->input, sizeof(state->input));
          
          break;
        }

        // 0xFx15 - LD DT, Vx - Set delay timer = Vx.
        case 0x0015:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          state->delay_timer = state->V[x];
          break;
        }

        // 0xFx18 - LD ST, Vx - Set sound timer = Vx.
        case 0x0018:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          state->sound_timer = state->V[x];
          break;
        }

        // 0xFx1E ADD I, Vx - Set I = I + Vx.
        case 0x001E:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          state->I += state->V[x];
          break;
        }

        // 0xFx29 LD F, Vx - Set I = location of sprite for digit Vx. 
        case 0x0029:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          state->I = state->V[x] * 5;
          break;
        }

        // 0xFx33 LD B, Vx - Store BCD representation of Vx in memory location I, I+1 and I+2.
        case 0x0033:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          state->memory[state->I] = state->V[x] / 100;
          state->memory[state->I + 1] = (state->V[x] / 10) % 10;
          state->memory[state->I + 2] = state->V[x] % 10;
          break;
        }

        // 0xFx55 LD [I], Vx - Store registers V0 through Vx in memory starting at location I.
        case 0x0055:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          for (int i = 0; i <= x; ++i)
          {
            state->memory[state->I + i] = state->V[i];
          }
          break;
        }

        // 0xFx65 LD Vx, [I] - Read registers V0 through Vx from memory starting at location I.
        case 0x0065:
        {
          uint8_t x = (opcode & 0x0F00) >> 8;
          for (int i = 0; i <= x; ++i)
          {
            state->V[i] = state->memory[state->I + i];
          }
          break;
        }

        default:
          printf("(ERROR) Unknown opcode: 0x%4X\n", opcode);
      }
      break;

    default:
      printf("(ERROR) Unknown opcode: 0x%4X\n", opcode);
      break;
  }
}
