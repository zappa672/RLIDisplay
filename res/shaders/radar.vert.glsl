#version 120

attribute float pos;
attribute float amp;

uniform float clear;
uniform float pel_len;
uniform float square_side;

varying float norm_amp;


void main() {
  float y = mod(abs(pos), square_side) - pel_len + 1;
  float x = (abs(pos) / square_side) - pel_len + 1;
  vec2 pos2 = vec2(x, y);

  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(pos2, -amp/255, 1);

  /*if (clear == 0)
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(pos2, -amp/255, 1);
  else
    if (pos < 0)
      gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(pos2, 1, 1);
    else
      gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * vec4(0, 0, 0, 0);
  */

  norm_amp = int(amp / 16.0);
}
