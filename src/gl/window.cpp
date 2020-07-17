#include <string>

#include "glad.h"
#include <GLFW/glfw3.h>

#include "window.hpp"

GLFWwindow *createWindow(
  const int major, const int minor, const bool is_core,
  const int width, const int height, const std::string &title
) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
  if (is_core) {
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  }
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  return glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}
