#include "chip8.h"
#include "renderer.h"


#include <stdio.h>
#include <stdint.h>
#include <time.h>

int main(int argc, char* argv[])
{
  init_renderer();

  struct chip8_state* state = new_chip8();
  load_program(state, "../c8games/tetris.ch8");

  struct timespec time;
  int64_t last_cycle = 0;
  int64_t last_timer = 0;
  int64_t clock_cycle;
  while (!should_close())
  {
    poll_window(state);

    timespec_get(&time, TIME_UTC);
    int64_t current_time = time.tv_sec * 1000 + time.tv_nsec * 0.000001;

    if (current_time > last_cycle + 2)
    {
      last_cycle = current_time;
      chip8_cycle(state);
    }

    if (current_time > last_timer + 17)
    {
      last_timer = current_time;
      if (state->sound_timer > 0)
      {
        state->sound_timer -= 1;
      }

      if (state->delay_timer > 0)
      {
        state->delay_timer -= 1;
      }
    }

    if (state->draw_flag != 0)
    {
      state->draw_flag = 0;
      render_display(state);
    }

    // getchar();
  }

  delete_chip8(state);

  return 0;
}
