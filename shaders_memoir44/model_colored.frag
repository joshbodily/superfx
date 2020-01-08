uniform sampler2D texture;
varying vec3 normal;
varying vec3 color;

void main() {
  float intensity = max(dot(vec3(0.577, 0.577, 0.577), normal), 0.6);
  gl_FragColor = vec4(color * intensity, 1.0);
}
