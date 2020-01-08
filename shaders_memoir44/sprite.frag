uniform sampler2D texture;
varying vec3 normal;
varying vec3 color;

void main() {
  gl_FragColor = texture2D(texture, gl_TexCoord[0].st);
  /*if (mod(gl_FragCoord.x + floor(gl_FragCoord.y), 2.0) > 1.0) {
    gl_FragColor = vec4(color * 0.75, 1.0);
  } else {
    discard;
  }*/
}
