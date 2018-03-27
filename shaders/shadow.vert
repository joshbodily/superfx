#version 120

uniform mat4 perspective;
uniform mat4 model;
uniform mat4 flatten;
uniform mat4 view;
uniform float height;

void main() {
  //vec4 pos = view * model * gl_Vertex;
  //pos.y = height;
  //gl_Position = perspective * pos;
  gl_Position = perspective * view * flatten * model * gl_Vertex;
}
