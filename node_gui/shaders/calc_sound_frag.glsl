varying vec2 tex_coord;
uniform float w;
uniform float lambda;
uniform float A;
uniform float t;

void main(void){
  float k = (2*3.1415926)/lambda;
  x = tex_coord.x;
  gl_FragColor = vec4(A*sin(wt + kx));
}
