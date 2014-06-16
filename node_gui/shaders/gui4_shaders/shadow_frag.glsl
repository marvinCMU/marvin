varying vec2 vTexCoord0;
uniform sampler2D tex;

void main(void){
  vec4 color = texture2D( tex, vTexCoord0 );
  color.w = 1.0 - color.x;
  gl_FragColor = color;
}
