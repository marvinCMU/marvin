varying vec2 tex_coord;
uniform sampler2D stream_tex;

void main(void){
  vec4 color = texture2D( stream_tex, tex_coord );
  gl_FragColor = color - vec4(.3, .3, .3, 0.0);
}
