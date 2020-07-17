#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__
#include <string>

#include "glad.h"
#include <GLFW/glfw3.h>

GLFWwindow *createWindow(
  const int major, const int minor, const bool is_core,
  const int width, const int height, const std::string &title
);

#endif // __WINDOW_HPP__
