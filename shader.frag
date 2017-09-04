//uniform sampler2D texture;

varying vec3 normal;

void main() {
  //vec4 color = texture2D(texture, gl_TexCoord[0].st);
  if (mod(gl_FragCoord.x + floor(gl_FragCoord.y), 2.0) > 1.0)
    gl_FragColor = vec4(1.0, 0.2, 0.3, 1.0) * max(dot(vec3(0.707, 0.707, 0), normal), 0.5);
  else
    gl_FragColor = vec4(0.3, 0.3, 0.3, 1.0);
}
