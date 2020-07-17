#ifndef __RECT_HPP__
#define __RECT_HPP__

#include "glad.h"
#include <GLFW/glfw3.h>

static GLuint current_vao = 0;

struct Rect {
  GLuint vao = 0;
};

Rect createRect();
Rect createTexturedRect();

void drawRect(const Rect &r);
void drawRectOutline(const Rect &r);

#endif // __RECT_HPP__
