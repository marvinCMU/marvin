varying vec2 tex_coord;
uniform sampler2D stream_tex;
uniform sampler2D border_tex;

void main(void){
  vec4 color = texture2D( border_tex, tex_coord);
  if ( any(lessThan( color, vec4(.1, .1, .1, 1.0 ) ))){
    gl_FragColor = texture2D( stream_tex, tex_coord );
  }else{
    gl_FragColor = color;
  }

}
