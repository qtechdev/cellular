#include <optional>
#include <string>

#include "glad.h"
#include <GLFW/glfw3.h>

#include "shader_program.hpp"

GLuint createShader(
  const GLenum shader_type, const std::string &shader_string
) {
  const GLchar *shader_str = shader_string.c_str();

  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &shader_str, nullptr);
  glCompileShader(shader);

  return shader;
}

GLuint createProgram(
  const GLuint v_shader, const GLuint f_shader, const bool delete_shaders
) {
  GLuint program = glCreateProgram();
  glAttachShader(program, v_shader);
  glAttachShader(program, f_shader);
  glLinkProgram(program);
  glDetachShader(program, v_shader);
  glDetachShader(program, f_shader);

  if (delete_shaders) {
    glDeleteShader(v_shader);
    glDeleteShader(f_shader);
  }

  return program;
}


std::optional<std::string> getCompileStatus(const GLuint shader) {
  int  success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if(!success) {
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    return infoLog;
  }

  return {};
}

std::optional<std::string> getLinkStatus(const GLuint program) {
  int  success;
  char infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);

  if(!success) {
    glGetProgramInfoLog(program, 512, nullptr, infoLog);
    return infoLog;
  }

  return {};
}

void uniform3f(
  const GLuint program, const char *name,
  const GLfloat x, const GLfloat y, const GLfloat z
) {
  GLuint loc = glGetUniformLocation(program, name);
  glUniform3f(loc, x, y, z);
}

void uniformMatrix4fv(
  const GLuint program, const char *name, const GLfloat *matrix
) {
  GLuint loc = glGetUniformLocation(program, name);
  glUniformMatrix4fv(loc, 1, GL_FALSE, matrix);
}
