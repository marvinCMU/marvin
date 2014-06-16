varying vec2 tex_coord;
uniform sampler2D stream_tex;
const vec4 color = vec4(0.2, 0.4, 0.8, 1.0); 
uniform float sigma;
const float clearRadius = .2f;


void main(void){
  vec2 origin = vec2(.5f, .5f );
  float A = 4.0f;
  float val;
  float var = sigma;

  float distSq = pow( (abs(tex_coord.x - origin.x)) ,2 ) + pow( abs(tex_coord.y - origin.y) ,2 );
  //  if( distSq < clearRadius ){
  //val = 0.0f;
    //  }
  //else{
    val = A- A * exp( -( pow( (abs(tex_coord.x - origin.x)) ,2 )/(2.0 *pow(var,2)) +
			  pow( ( abs(tex_coord.y - origin.y)), 2) / ( 2.0 * pow(var,2) ) ) );
    //  }
  //val = 1.0;
  
  //gl_FragColor = vec4(texture2D( stream_tex, tex_coord).xyz, 0.0) + vec4(1.0, 0.0, 0.0, 1.0 );//+ color * val;
  //gl_FragColor = vec4(val, 0.0, 0.0, 1.0 );
  //gl_FragColor = mix(texture2D( stream_tex, tex_coord), vec4(val, 0.0, 0.0, 1.0), 0.0 );
  gl_FragColor = texture2D( stream_tex, tex_coord)  + vec4( 0.0, 0.0, val, 0.0 ) - vec4(val, val, 0.0, 0.0); 
  
}
