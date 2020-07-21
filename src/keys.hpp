#ifndef __KEY_BINDINGS_HPP__
#define __KEY_BINDINGS_HPP__
#include <functional>
#include <string_view>

#include "elementary.hpp"

struct game_state {
  bool is_paused = false;
  bool is_single_step = false;
  bool do_reset_texture = false;
  bool do_save_texture = false;
  bool do_update_rule = false;
  int gen_count = 0;
  int wolfram_code = 0;
  int new_code = 0;
};

using key_f = std::function<void(qca::elementary &ca, game_state &s)>;

struct key {
  const int key_code;
  const std::string_view name;
  const key_f f = [](qca::elementary &ca, game_state &s){};
  bool is_pressed = false;
  bool is_handled = false;
};

static key key_pause{
  GLFW_KEY_SPACE, "SPACEBAR",
  [](qca::elementary &ca, game_state &s){
    s.is_paused = !s.is_paused;
  }
};
static key key_step{
  GLFW_KEY_PERIOD, ">",
  [](qca::elementary &ca, game_state &s){
    s.is_paused = false;
    s.is_single_step = true;
  }
};
static key key_reset_single_1{
  GLFW_KEY_1, "1",
  [](qca::elementary &ca, game_state &s){
    ca.reset();
    ca.init_single_1();
    s.gen_count = 0;
    s.is_paused = false;
    s.do_reset_texture = true;
  }
};
static key key_reset_random{
  GLFW_KEY_R, "R",
  [](qca::elementary &ca, game_state &s){
    ca.reset();
    ca.init_random();
    s.gen_count = 0;
    s.is_paused = false;
    s.do_reset_texture = true;
  }
};
static key key_reset_alternate{
  GLFW_KEY_APOSTROPHE, "@",
  [](qca::elementary &ca, game_state &s){
    ca.reset();
    ca.init_alternate();
    s.gen_count = 0;
    s.is_paused = false;
    s.do_reset_texture = true;
  }
};
static key key_save{
  GLFW_KEY_S, "S",
  [](qca::elementary &ca, game_state &s){
    s.do_save_texture = true;
  }
};
static key key_next{
  GLFW_KEY_RIGHT_BRACKET , "]",
  [](qca::elementary &ca, game_state &s){
    s.new_code = s.wolfram_code + 1;
    if (s.new_code > 255) { s.new_code = 0; }
    s.do_update_rule = true;
  }
};
static key key_prev{
  GLFW_KEY_LEFT_BRACKET , "[",
  [](qca::elementary &ca, game_state &s){
    s.new_code = s.wolfram_code - 1;
    if (s.new_code < 0) { s.new_code = 255; }
    s.do_update_rule = true;
  }
};

static std::vector<key> key_bindings = {
  key_pause,
  key_step,
  key_reset_single_1,
  key_reset_alternate,
  key_reset_random,
  key_save,
  key_next,
  key_prev
};

#endif // __KEY_BINDINGS_HPP__
