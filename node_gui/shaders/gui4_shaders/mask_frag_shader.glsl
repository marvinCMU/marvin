varying vec2 vTexCoord0;
uniform sampler2D tex;
uniform sampler2D maskTex;

void main(void){
  vec4 color = texture2D( tex, vTexCoord0 );
  if( all( lessThanEqual( texture2D( maskTex, vTexCoord0 ), vec4(0.2, 0.2, 0.2, 1.0)))){
    color.w = 0.0;
  }
  gl_FragColor = color;
}
