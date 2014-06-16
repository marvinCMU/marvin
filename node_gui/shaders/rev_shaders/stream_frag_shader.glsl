varying vec2 tex_coord;
uniform sampler2D stream_tex;

void main(void){
  gl_FragColor = texture2D( stream_tex, tex_coord );
}
