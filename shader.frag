uniform sampler2D texture;

void main() {
  vec4 color = texture2D(texture, gl_TexCoord[0].st);
  if (mod(gl_FragCoord.x + floor(gl_FragCoord.y), 2.0) > 1.0)
    gl_FragColor = color;
  else
    discard;
}
