varying vec2 tex_coord;
uniform sampler2D stream_tex;
uniform float whiteout;

void main(void){
  float value = 1.0 - gTime * rate;
  if(value <= 0)
    gl_FragColor = texture2D( stream_tex, tex_coord );
  else{
    gl_FragColor = texture2D( stream_tex, tex_coord ) - vec4(1.0, 1.0, 1.0, 0.0) * value;
  }
}
