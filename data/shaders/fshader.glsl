#version 330 core

in vec3 _colour;
in vec2 _tex_coords;

out vec4 FragmentColour;

uniform sampler2D texture_data;

void main() {
  FragmentColour = texture(texture_data, _tex_coords);
}
