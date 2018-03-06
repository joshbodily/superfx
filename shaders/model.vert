#version 120

uniform mat4 perspective;
uniform mat4 model;
uniform mat4 view;

varying vec3 normal;
varying vec3 color;

void main() {
  //normal = gl_Normal;
  normal = vec3(view * model * vec4(gl_Normal, 0.0));
  color = gl_Color.rgb;
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = perspective * view * model * gl_Vertex;
}
