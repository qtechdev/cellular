#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <regex>
#include <sstream>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <qxdg/qxdg.hpp>
#include <qfio/qfio.hpp>

#include "elementary.hpp"

#include "gl/rect.hpp"
#include "gl/shader_program.hpp"
#include "gl/texture.hpp"
#include "gl/window.hpp"
#include "util/error.hpp"
#include "util/timer.hpp"

static constexpr int window_width = 400;
static constexpr int window_height = 200;
static constexpr int gl_major_version = 3;
static constexpr int gl_minor_version = 3;

static int wolfram_code = 0;

struct key {
  int key_code;
  std::string name;
  bool is_pressed = false;
  bool is_handled = false;
};

static key key_pause{GLFW_KEY_SPACE, "SPACEBAR"};
static key key_step{GLFW_KEY_PERIOD, ">"};
static key key_reset_single_0{GLFW_KEY_0, "0"};
static key key_reset_single_1{GLFW_KEY_1, "1"};
static key key_reset_alternate{GLFW_KEY_A, "A"};
static key key_reset_random{GLFW_KEY_R, "R"};
static key key_save{GLFW_KEY_S, "S"};
static key key_next{GLFW_KEY_RIGHT_BRACKET , "]"};
static key key_prev{GLFW_KEY_LEFT_BRACKET , "["};

constexpr timing::seconds loop_timestep(1.0/60.0);

void processInput(GLFWwindow *window);
std::array<glm::mat4, 3> fullscreen_rect_matrices(const int w, const int h);

int main(int argc, const char *argv[]) {
  // get base directories
  xdg::base base_dirs = xdg::get_base_directories();

  // create opengl window and context
  GLFWwindow *window = createWindow(
    gl_major_version, gl_minor_version, true, window_width, window_height,
    "Cellular Automata"
  );

  if (window == nullptr) {
    glfwDestroyWindow(window);
    return to_underlying(error_code_t::window_failed);
  }

  glfwMakeContextCurrent(window);

  // load opengl functions
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    return to_underlying(error_code_t::glad_failed);
  }

  glViewport(0, 0, window_width, window_height);
  glClearColor(0.1, 0.1, 0.2, 1.0);

  // load shaders
  auto v_shader_path = xdg::get_data_path(
    base_dirs, "cellular", "shaders/vshader.glsl"
  );
  auto v_shader_string = fio::read(*v_shader_path);

  auto f_shader_path = xdg::get_data_path(
    base_dirs, "cellular", "shaders/fshader.glsl"
  );
  auto f_shader_string = fio::read(*f_shader_path);

  GLuint v_shader = createShader(GL_VERTEX_SHADER, *v_shader_string);
  GLuint f_shader = createShader(GL_FRAGMENT_SHADER, *f_shader_string);
  GLuint shader_program = createProgram(v_shader, f_shader, true);

  // initialise automata
  qca::elementary ca(window_width, window_height, qca::wolfram(110));
  ca.init_random();

  // initialise texture
  static const std::size_t num_cols = ca.field_width;
  static const std::size_t num_rows = ca.field_height;
  static constexpr std::size_t num_channels = 3;
  static const std::size_t image_stride = num_cols * num_channels;
  static const std::size_t data_size = num_cols*num_rows*num_channels;
  unsigned char data[num_cols*num_rows*num_channels];

  for (int i = 0; i < data_size; i++) {
    data[i] = 0;
  }

  Texture texture = create_texture_from_data(
    num_cols, num_rows, num_channels, data
  );

  // create screen rect
  Rect rect = createTexturedRect();

  auto [projection, view, model] = fullscreen_rect_matrices(
    window_width, window_height
  );

  glUseProgram(shader_program);
  uniformMatrix4fv(shader_program, "projection", glm::value_ptr(projection));
  uniformMatrix4fv(shader_program, "view", glm::value_ptr(view));
  uniformMatrix4fv(shader_program, "model", glm::value_ptr(model));

  timing::Clock clock;
  timing::Timer loop_timer;
  timing::seconds loop_accumulator(0.0);

  bool is_paused = false;
  bool is_single_step = false;
  int gen_count = 0;
  std::vector<uint8_t> full_texture_data;
  full_texture_data.resize(ca.field_width * ca.field_height * 3);

  while (!glfwWindowShouldClose(window)) {
    loop_accumulator += loop_timer.getDelta();
    loop_timer.tick(clock.get());

    //process input
    glfwPollEvents();
    processInput(window);

    // handle user input
    // todo: reduce amount of code here?
    if(
      (glfwGetKey(window, key_pause.key_code) == GLFW_PRESS) &&
      !key_pause.is_handled
    ) {
      is_paused = !is_paused;
      key_pause.is_pressed = true;
      key_pause.is_handled = true;
    }

    if(glfwGetKey(window, key_pause.key_code) == GLFW_RELEASE) {
      key_pause.is_pressed = false;
      key_pause.is_handled = false;
    }

    if(
      (glfwGetKey(window, key_step.key_code) == GLFW_PRESS) &&
      !key_step.is_handled
    ) {
      is_paused = false;
      is_single_step = true;
      key_step.is_pressed = true;
      key_step.is_handled = true;
    }

    if(glfwGetKey(window, key_step.key_code) == GLFW_RELEASE) {
      key_step.is_pressed = false;
      key_step.is_handled = false;
    }

    if(
      (glfwGetKey(window, key_reset_single_0.key_code) == GLFW_PRESS) &&
      !key_reset_single_0.is_handled
    ) {
      ca.reset();
      ca.init_single_0();
      gen_count = 0;
      key_reset_single_0.is_pressed = true;
      key_reset_single_0.is_handled = true;
    }

    if(glfwGetKey(window, key_reset_single_0.key_code) == GLFW_RELEASE) {
      key_reset_single_0.is_pressed = false;
      key_reset_single_0.is_handled = false;
    }

    if(
      (glfwGetKey(window, key_reset_single_1.key_code) == GLFW_PRESS) &&
      !key_reset_single_1.is_handled
    ) {
      ca.reset();
      ca.init_single_1();
      gen_count = 0;
      key_reset_single_1.is_pressed = true;
      key_reset_single_1.is_handled = true;
    }

    if(glfwGetKey(window, key_reset_single_1.key_code) == GLFW_RELEASE) {
      key_reset_single_1.is_pressed = false;
      key_reset_single_1.is_handled = false;
    }

    if(
      (glfwGetKey(window, key_reset_alternate.key_code) == GLFW_PRESS) &&
      !key_reset_alternate.is_handled
    ) {
      ca.reset();
      ca.init_alternate();
      gen_count = 0;
      key_reset_alternate.is_pressed = true;
      key_reset_alternate.is_handled = true;
    }

    if(glfwGetKey(window, key_reset_alternate.key_code) == GLFW_RELEASE) {
      key_reset_alternate.is_pressed = false;
      key_reset_alternate.is_handled = false;
    }

    if(
      (glfwGetKey(window, key_reset_random.key_code) == GLFW_PRESS) &&
      !key_reset_random.is_handled
    ) {
      ca.reset();
      ca.init_random();
      gen_count = 0;
      key_reset_random.is_pressed = true;
      key_reset_random.is_handled = true;
    }

    if(glfwGetKey(window, key_reset_random.key_code) == GLFW_RELEASE) {
      key_reset_random.is_pressed = false;
      key_reset_random.is_handled = false;
    }

    if(
      (glfwGetKey(window, key_save.key_code) == GLFW_PRESS) &&
      !key_save.is_handled
    ) {
      stbi_write_png(
        "out.png", ca.field_width, ca.field_height, 3,
        full_texture_data.data(), ca.field_width * 3
      );
      key_save.is_pressed = true;
      key_save.is_handled = true;
    }

    if(glfwGetKey(window, key_save.key_code) == GLFW_RELEASE) {
      key_save.is_pressed = false;
      key_save.is_handled = false;
    }

    if(
      (glfwGetKey(window, key_next.key_code) == GLFW_PRESS) &&
      !key_next.is_handled
    ) {
      wolfram_code++;
      if (wolfram_code > 255) { wolfram_code = 0; }
      ca.set_rules(qca::wolfram(wolfram_code));
      std::cout << "Wolfram Code: " << wolfram_code << "\n";
      key_next.is_pressed = true;
      key_next.is_handled = true;
    }

    if(glfwGetKey(window, key_next.key_code) == GLFW_RELEASE) {
      key_next.is_pressed = false;
      key_next.is_handled = false;
    }

    if(
      (glfwGetKey(window, key_prev.key_code) == GLFW_PRESS) &&
      !key_prev.is_handled
    ) {
      wolfram_code--;
      if (wolfram_code < 0) { wolfram_code = 255; }
      ca.set_rules(qca::wolfram(wolfram_code));
      std::cout << "Wolfram Code: " << wolfram_code << "\n";
      key_prev.is_pressed = true;
      key_prev.is_handled = true;
    }

    if(glfwGetKey(window, key_prev.key_code) == GLFW_RELEASE) {
      key_prev.is_pressed = false;
      key_prev.is_handled = false;
    }

    // update loop
    while (loop_accumulator >= loop_timestep) {
      if (gen_count >= ca.field_height) {
        is_paused = true;
      }

      if (is_paused) {
        loop_accumulator -= loop_timestep;
        continue;
      }

      if (is_single_step) {
        is_paused = true;
        is_single_step = false;
      }

      auto gen = ca.get();
      ca.next();

      std::vector<uint8_t> texture_data = qca::cells_to_colour(gen);
      for (int i = 0; i < texture_data.size(); ++i) {
        const int index = (gen_count * ca.field_width * 3) + i;
        full_texture_data[index] = texture_data[i];
      }

      bindTexture(texture);
      glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, gen_count, ca.field_width, 1,
        GL_RGB, GL_UNSIGNED_BYTE, texture_data.data()
      );
      bindTexture({0});

      loop_accumulator -= loop_timestep;
      gen_count++;
    }

    // draw screen texture
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);
    bindTexture(texture);
    drawRect(rect);
    glfwSwapBuffers(window);
  }

  return 0;
}

void processInput(GLFWwindow *window) {
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

std::array<glm::mat4, 3> fullscreen_rect_matrices(const int w, const int h) {
  glm::mat4 projection = glm::ortho<double>(0, w, h, 0, 0.1, 100.0);

  glm::mat4 view = glm::mat4(1.0);
  view = glm::translate(view, glm::vec3(0.0, 0.0, -1.0));

  glm::mat4 model = glm::mat4(1.0);
  model = glm::scale(model, glm::vec3(w, h, 1));

  return {projection, view, model};
}
