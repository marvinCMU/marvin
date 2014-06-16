varying vec2 vTexCoord0;
uniform sampler2D tex;
uniform sampler2D maskTex;

void main(void){
  vec4 color = texture2D( tex, vTexCoord0 );
  if( all( lessThanEqual( texture2D( maskTex, vTexCoord0 ), vec4(0.2, 0.2, 0.2, 1.0)))){
    color.w = 0.0;
  }
  else{
    float gamma = 1.60;
    float L = 0.2126 * pow(color.x,gamma) + 0.7152 * pow(color.y,gamma) + 0.0722 * pow(color.z,gamma);
    color = vec4(L, L, L, 1.0 );
  }
  
  gl_FragColor = color;
}
