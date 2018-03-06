#version 120

uniform sampler2D texture;
uniform vec3 brightness;

void main() {
  gl_FragColor = texture2D(texture, gl_TexCoord[0].st);
}
