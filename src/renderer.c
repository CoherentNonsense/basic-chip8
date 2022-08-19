#include "renderer.h"

#include <string.h>
#include <stb_image.h>
// #include <stdio.h>


static struct render_data data;

const char *vertex_shader_src = "#version 330 core\n"
  "layout (location = 0) in vec2 a_pos;\n"
  "layout (location = 1) in vec2 a_uv;\n"
  "out vec2 uv;\n"
  "void main()\n"
  "{\n"
  "   gl_Position = vec4(a_pos.x, a_pos.y, 0.0, 1.0);\n"
  "   uv = a_uv;\n"
  "}\0";

const char *fragment_shader_src = "#version 330 core\n"
  "in vec2 uv;\n"
  "uniform sampler2D u_tex;\n"
  "out vec4 color;\n"
  "void main()\n"
  "{\n"
  "   color = texture(u_tex, uv);\n"
  "}\n\0";

float vertices[] = {
   (1.0f / 3.0f),  1.0f, 1.0f, 0.0f, // top-right
  -1.0f,            -1.0f, 0.0f, 1.0f, // bottom-left
   (1.0f / 3.0f), -1.0f, 1.0f, 1.0f, // bottom-right

  -1.0f,             1.0f, 0.0f, 0.0f,  // top-left
  -1.0f,            -1.0f, 0.0f, 1.0f, // bottom-left
   (1.0f / 3.0f),  1.0f, 1.0f, 0.0f, // top-right
};

int keys[] = {
  GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4,
  GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E, GLFW_KEY_R,
  GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_F,
  GLFW_KEY_Z, GLFW_KEY_X, GLFW_KEY_C, GLFW_KEY_V
};

void init_renderer()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  #endif

  data.window = glfwCreateWindow(64 * 18, 32 * 12, "Chip8 Emulator", NULL, NULL);
  glfwMakeContextCurrent(data.window);
  glfwSetWindowAspectRatio(data.window, 3, 1);
  gladLoadGL(glfwGetProcAddress);
  // glViewport(0, 0, 64 * 16, 32 * 16);

  // Create Shader Program
  int success = 0;
  char info[512];

  unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
  glCompileShader(vertex_shader);

  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
      glGetShaderInfoLog(data.shader, 512, NULL, info);
      printf("ERROR::SHADER::VERTEX::COMPILING_FAILED %s\n", info);
  }

  unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
  glCompileShader(fragment_shader);

  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
      glGetShaderInfoLog(data.shader, 512, NULL, info);
      printf("ERROR::SHADER::FRAGMENT::COMPILING_FAILED %s\n", info);
  }

  data.shader = glCreateProgram();
  glAttachShader(data.shader, vertex_shader);
  glAttachShader(data.shader, fragment_shader);
  glLinkProgram(data.shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);


  glGetProgramiv(data.shader, GL_LINK_STATUS, &success);
  if (!success) {
      glGetProgramInfoLog(data.shader, 512, NULL, info);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED %s\n", info);
  }

  glUseProgram(data.shader);


  // Create Texture
  for (int i = 0; i < CHIP8_SCREEN_WIDTH * CHIP8_SCREEN_HEIGHT * 3; ++i)
  {
    data.texture_data[i] = 0;
  }

  glGenTextures(1, &data.texture);
  glBindTexture(GL_TEXTURE_2D, data.texture);
  glEnable(GL_TEXTURE_2D);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CHIP8_SCREEN_WIDTH, CHIP8_SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)data.texture_data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


  // Quad Stuff
  glGenVertexArrays(1, &data.vao);
  glBindVertexArray(data.vao);
  glGenBuffers(1, &data.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

  unsigned int u_texture = glGetUniformLocation(data.shader, "u_tex");
  glUniform1i(u_texture, 0);


  // Debug
  int width;
  int height;
  int channels;
  unsigned char* texture_data = stbi_load("../resources/font.png", &width, &height, &channels, 4);
  if (texture_data == NULL)
  {
    printf("Failed to load font texture.\n");
  }

  glGenTextures(1, &data.debug_texture);
  glBindTexture(GL_TEXTURE_2D, data.debug_texture);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA,
    (unsigned int)width, (unsigned int)height,
    0, GL_RGBA, GL_UNSIGNED_BYTE,
    texture_data
  );

  stbi_image_free(texture_data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glGenVertexArrays(1, &data.debug_vao);
  glBindVertexArray(data.debug_vao);
  glGenBuffers(1, &data.debug_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, data.debug_vbo);

  for (int i = 0, j = 0; i < sizeof(data.debug_indices) / sizeof(unsigned int); i += 6, j += 4)
  {
    data.debug_indices[i + 0] = j + 0;
    data.debug_indices[i + 2] = j + 1;
    data.debug_indices[i + 1] = j + 2;

    data.debug_indices[i + 3] = j + 2;
    data.debug_indices[i + 4] = j + 3;
    data.debug_indices[i + 5] = j + 0;
  }

  glGenBuffers(1, &data.debug_ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.debug_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(data.debug_indices), data.debug_indices, GL_STATIC_DRAW);
}

int should_close()
{
  return glfwWindowShouldClose(data.window);
}

void poll_window(struct chip8_state* state)
{
  glfwPollEvents();
  
  for (int i = 0; i < sizeof(keys) / sizeof(int); ++i)
  {
    if (glfwGetKey(data.window, keys[i]) == GLFW_PRESS)
    {
      state->input[i] = 1;
    }
    if (glfwGetKey(data.window, keys[i]) == GLFW_RELEASE)
    {
      state->input[i] = 0;
    }
  }

}

void render_display(struct chip8_state* state)
{
  glClearColor(0.5f, 0.5f, 0.6f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  for (int y = 0; y < 32; ++y)
  {
    for (int x = 0; x < 64; ++x)
    {
      data.texture_data[((y * 64 + x) * 3) + 0] = state->display[y * 64 + x] * 255; // R
      data.texture_data[((y * 64 + x) * 3) + 1] = state->display[y * 64 + x] * 255; // G
      data.texture_data[((y * 64 + x) * 3) + 2] = state->display[y * 64 + x] * 255; // B
    }
  }

  glBindVertexArray(data.vao);
  glBindTexture(GL_TEXTURE_2D, data.texture);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, CHIP8_SCREEN_WIDTH, CHIP8_SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, (void*)data.texture_data);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glfwSwapBuffers(data.window);
}

void render_debug(struct chip8_state* state)
{

}