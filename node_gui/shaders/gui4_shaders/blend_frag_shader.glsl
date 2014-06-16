varying vec2 vTexCoord0;
uniform sampler2D tex0;
uniform float blend;

void main(void){
  vec4 color = texture2D(tex0, vTexCoord0 );
  color.w = blend;
  gl_FragColor = color;
}
