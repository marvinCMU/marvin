varying vec2 vTexCoord0;
void main(void){
  gl_Position = ftransform();
  vTexCoord0 = gl_MultiTexCoord0.st;
}
