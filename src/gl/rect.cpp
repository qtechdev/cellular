#include <cstddef> // std::size_t

#include "glad.h"
#include <GLFW/glfw3.h>

#include "rect.hpp"

/*
  a___d
  |\ |
  |_\|
  b   c
*/
constexpr GLfloat vertex_data[12] = {
  // vertex/texture cooordinates
  0.0, 1.0, 0.0, // a
  0.0, 0.0, 0.0, // b
  1.0, 0.0, 0.0, // c
  1.0, 1.0, 0.0, // d
};

constexpr GLfloat colours[12] = {
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0,
  1.0, 1.0, 1.0
};

constexpr GLuint indices[6] = {
  0, 1, 2,
  0, 2, 3
};

Rect createRect() {
  GLuint vao;
  GLuint buffers[2];

  glGenVertexArrays(1, &vao);
  glGenBuffers(2, buffers);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(vertex_data) + sizeof(colours), nullptr,
    GL_STATIC_DRAW
  );
  glBufferSubData(
    GL_ARRAY_BUFFER, 0, sizeof(vertex_data), vertex_data
  );
  glBufferSubData(
    GL_ARRAY_BUFFER, sizeof(vertex_data), sizeof(colours), colours
  );
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void *>(0)
  );
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
    reinterpret_cast<void *>(sizeof(vertex_data))
  );

  GLuint ebo;
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW
  );

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(2, buffers);

  return {vao};
}

Rect createTexturedRect() {
  GLuint vao;
  GLuint buffers[2];

  glGenVertexArrays(1, &vao);
  glGenBuffers(2, buffers);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW
  );
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void *>(0)
  );
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
    2, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void *>(0)
  );

  GLuint ebo;
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW
  );

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glDeleteBuffers(2, buffers);

  return {vao};
}

void drawRect(const Rect &r) {
  if (current_vao != r.vao) {
    glBindVertexArray(r.vao);
    current_vao = r.vao;
  }

  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawRectOutline(const Rect &r) {
  if (current_vao != r.vao) {
    glBindVertexArray(r.vao);
    current_vao = r.vao;
  }

  glDrawArrays(GL_LINE_LOOP, 0, 4);
}
