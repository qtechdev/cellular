#version 330 core
layout (location = 0) in vec3 attr_pos;
layout (location = 1) in vec3 attr_colour;
layout (location = 2) in vec2 attr_tex_coords;

out vec3 _colour;
out vec2 _tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  gl_Position = projection * view * model * vec4(attr_pos, 1.0);
  _colour = attr_colour;
  _tex_coords = attr_tex_coords;
}
