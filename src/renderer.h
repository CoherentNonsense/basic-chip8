#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdint.h>
#include "chip8.h"

struct debug_vertex
{
  float pos_x;
  float pos_y;
  float uv_x;
  float uv_y;
};

struct render_data
{
  GLFWwindow* window;
  uint8_t texture_data[CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT * 3];
  unsigned int shader;
  unsigned int vbo;
  unsigned int vao;
  unsigned int texture;

  struct debug_vertex debug_vertices[4 * 5 * 21];
  unsigned int debug_indices[(4 + 5 + 21) / 4 * 6];
  unsigned int debug_vbo;
  unsigned int debug_vao;
  unsigned int debug_ebo;
  unsigned int debug_texture;
};

void init_renderer();
int should_close();
void poll_window(struct chip8_state* state);
void render_display(struct chip8_state* state);
void render_debug(struct chip8_state* state);

#endif