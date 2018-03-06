varying vec3 normal;
varying vec3 color;

void main() {
    float intensity = max(dot(vec3(0.577, 0.577, 0.577), normal), 0.0);
    if (intensity < 0.2) {
      if (mod(gl_FragCoord.x + floor(gl_FragCoord.y), 2.0) > 1.0) {
        gl_FragColor = vec4(color * 0.75, 1.0);
      } else {
        gl_FragColor = vec4(0.3, 0.3, 0.3, 1.0);
      }
    } else if (intensity < 0.5) {
      gl_FragColor = vec4(color * 0.75, 1.0);
    } else {
      gl_FragColor = vec4(color, 1.0);
    }
}
