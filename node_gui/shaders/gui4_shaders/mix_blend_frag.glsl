varying vec2 vTexCoord0;
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform float blend;
uniform float blendBias;
void main(void){
  //vec4 color = mix(texture2D(tex0, vTexCoord0), texture2D(tex1, vTexCoord0 ), blendBias );
  //color.w = blend;
  //gl_FragColor = color;//texture2D( tex0, vTexCoord0 );
  vec4 color1 = texture2D( tex0, vTexCoord0 );
  vec4 color2 = texture2D( tex1, vTexCoord0 );
  color1.w = blend - blendBias;
  color2.w = blendBias;
  
  gl_FragColor = color1 + color2;
}
