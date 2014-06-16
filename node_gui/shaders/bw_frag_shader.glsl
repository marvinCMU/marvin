varying vec2 tex_coord;
uniform sampler2D stream_tex;

void main(void){
  vec4 color = texture2D( stream_tex, tex_coord );
  float gamma = 1.60;
  float L = 0.2126 * pow(color.x,gamma) + 0.7152 * pow(color.y,gamma) + 0.0722 * pow(color.z,gamma);
  gl_FragColor = vec4(L, L, L, 1.0 );
  //gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0 );
  
}
