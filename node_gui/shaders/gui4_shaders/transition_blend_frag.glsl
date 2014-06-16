varying vec3 vTexCoord0;
uniform sampler2D texInitial;
uniform sampler2D texFinal;
uniform float blend;
uniform float rate;

void main(void){
  vec4 color1 = sampler2D( texInitial, cTexCoord0 );
  vec4 color2 = sampler2D( texFinal, cTexCoord0 );
  
}
