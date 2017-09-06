//uniform sampler2D texture;

varying vec3 normal;
varying vec4 color;

void main() {
  if (color.b > 0.7) {
    if (mod(gl_FragCoord.x + floor(gl_FragCoord.y), 2.0) > 1.0)
      gl_FragColor = color * max(dot(vec3(0.707, 0.707, 0), normal), 0.5);
    else
      gl_FragColor = vec4(0.5, 0.5, 0.5, 1);
  } else {
    gl_FragColor = color * max(dot(vec3(0.707, 0.707, 0), normal), 0.5);
  }
}
