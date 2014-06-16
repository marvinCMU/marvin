varying vec2 tex_coord;
uniform vec2 lcPosition;


void main(void){
  tex_coord = gl_MultiTexCoord0.st;
  gl_Position = ftransform() + vec4(lcPosition, 0.0, 0.0 );
}
