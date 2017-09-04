uniform mat4 perspective;
uniform mat4 model;
uniform mat4 view;

void main() {
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = perspective * view * model * gl_Vertex;
}
