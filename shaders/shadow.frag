void main() {
  if (mod(gl_FragCoord.x + floor(gl_FragCoord.y), 2.0) > 1.0) {
    discard;
  } else {
    gl_FragColor = vec4(0.3, 0.3, 0.3, 1.0);
  }
}
