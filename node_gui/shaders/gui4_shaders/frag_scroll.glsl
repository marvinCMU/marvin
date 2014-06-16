varying vec2 vTexCoord0;
uniform sampler2D tex0;

void main(void){
  gl_FragColor = texture2D( tex0, (vTexCoord0 + vec2(0, .5)) );
}
