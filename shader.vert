#version 120

uniform mat4 perspective;
uniform mat4 model;
uniform mat4 view;

varying vec3 normal;
varying vec4 color;

void main() {
  normal = gl_Normal;
  color = gl_Color;
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = perspective * view * model * gl_Vertex;
}
