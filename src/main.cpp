#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
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
#include "keys.hpp"

#include "gl/rect.hpp"
#include "gl/shader_program.hpp"
#include "gl/texture.hpp"
#include "gl/window.hpp"
#include "util/error.hpp"
#include "util/timer.hpp"

static constexpr int window_width = 800;
static constexpr int window_height = 200;
static constexpr int gl_major_version = 3;
static constexpr int gl_minor_version = 3;

#define BATCH_MODE

#ifndef BATCH_MODE
constexpr timing::seconds loop_timestep(1.0/60.0);
#else
constexpr timing::seconds loop_timestep(1.0/6000.0);
#endif


void processInput(GLFWwindow *window);
std::array<glm::mat4, 3> fullscreen_rect_matrices(const int w, const int h);
void reset_texture(
  const Texture &t, const int w, const int h, const std::vector<uint8_t> &d
);

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
  game_state state;
  qca::elementary ca(
    window_width, window_height, qca::wolfram(state.wolfram_code)
  );
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

  std::vector<uint8_t> blank_texture;
  blank_texture.resize(ca.field_width * ca.field_height * 3);

  std::vector<uint8_t> full_texture_data;
  full_texture_data.resize(ca.field_width * ca.field_height * 3);

  while (!glfwWindowShouldClose(window)) {
    loop_accumulator += loop_timer.getDelta();
    loop_timer.tick(clock.get());

    //process input
    glfwPollEvents();
    processInput(window);

    // handle user input
    for (key &k : key_bindings) {
      if (
        (glfwGetKey(window, k.key_code) == GLFW_PRESS) &&
        !k.is_handled
      ) {
        k.f(ca, state);
        k.is_pressed = true;
        k.is_handled = true;
      } else if (glfwGetKey(window, k.key_code) == GLFW_RELEASE) {
        k.is_pressed = false;
        k.is_handled = false;
      }
    }

    if (state.do_save_texture) {
      std::stringstream ss;
      ss << "out/" << state.wolfram_code << ".png";
      stbi_write_png(
        ss.str().c_str(), ca.field_width, ca.field_height, 3,
        full_texture_data.data(), ca.field_width * 3
      );
      state.do_save_texture = false;
    }

    if (state.do_reset_texture) {
      reset_texture(texture, ca.field_width, ca.field_height, blank_texture);
      state.do_reset_texture = false;
    }

    if (state.do_update_rule) {
      state.gen_count = 0;
      state.wolfram_code = state.new_code;
      std::cout << "Wolfram Code: " << state.wolfram_code << "\n";

      ca.set_rules(qca::wolfram(state.wolfram_code));
      ca.reset();
      ca.init_random();
      state.is_paused = false;

      state.do_update_rule = false;
    }

    // update loop
    while (loop_accumulator >= loop_timestep) {
      if (state.gen_count >= ca.field_height) {
        state.is_paused = true;

        #ifdef BATCH_MODE
        if (state.wolfram_code < 256) {
          state.do_save_texture = true;
          state.do_reset_texture = true;
          state.do_update_rule = true;
          state.new_code = state.wolfram_code + 1;
        } else {
          glfwSetWindowShouldClose(window, GLFW_TRUE);
          break;
        }
        #endif
      }

      if (state.is_paused) {
        loop_accumulator -= loop_timestep;
        continue;
      }

      if (state.is_single_step) {
        state.is_paused = true;
        state.is_single_step = false;
      }

      auto gen = ca.get();
      ca.next();

      std::vector<uint8_t> texture_data = qca::cells_to_colour(gen);
      for (int i = 0; i < texture_data.size(); ++i) {
        const int index = (state.gen_count * ca.field_width * 3) + i;

        if (index >= full_texture_data.size()) {
          std::cerr << "E: " << index << ", " << full_texture_data.size();
          std::cerr << ". gen " << state.gen_count << ", rule ";
          std::cerr << state.wolfram_code << "\n";
          break;
        }

        full_texture_data[index] = texture_data.at(i);
      }

      bindTexture(texture);
      glTexSubImage2D(
        GL_TEXTURE_2D, 0, 0, state.gen_count, ca.field_width, 1,
        GL_RGB, GL_UNSIGNED_BYTE, texture_data.data()
      );
      bindTexture({0});

      loop_accumulator -= loop_timestep;
      state.gen_count++;
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

void reset_texture(
  const Texture &t, const int w, const int h, const std::vector<uint8_t> &d
) {
  bindTexture(t);
  glTexSubImage2D(
    GL_TEXTURE_2D, 0, 0, 0, w, h,
    GL_RGB, GL_UNSIGNED_BYTE, d.data()
  );
  bindTexture({0});
}
