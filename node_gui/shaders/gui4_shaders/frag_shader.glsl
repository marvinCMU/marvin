varying vec2 vTexCoord0;
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform float blend;
void main(void){
  if( all(lessThan( abs(texture2D( tex1, vTexCoord0 )-vec4(99.0/255.0, 124.0/255.0, 34.0/255.0, 1.0)),
		   vec4(.1, .1, .1, 1.0) ) ))
    gl_FragColor = texture2D(tex0, vTexCoord0);
  else
    gl_FragColor = mix( texture2D(tex0, vTexCoord0 ), texture2D( tex1, vTexCoord0 ) , .85 );
  
}
