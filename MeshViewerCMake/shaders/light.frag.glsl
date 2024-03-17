

varying vec3 mynormal;
varying vec4 myvertex;
varying vec4 mycolor;

vec4 mylight_position = vec4(0,0,4,1);
vec4 mylight_color = vec4(1,1,1,0);

void main (void) 
{        
	vec3 eyepos = vec3(0,0,0) ; 

	vec4 _mypos = gl_ModelViewMatrix * myvertex ; 
    vec3 mypos = _mypos.xyz / _mypos.w ;

	vec4 _lightpos = mylight_position;
   	vec3 lightpos = _lightpos.xyz / _lightpos.w;

	vec3 normal = normalize(gl_NormalMatrix*mynormal);

	vec3 eyedir = normalize(eyepos - mypos) ;
	vec3 lightdir = normalize (lightpos - mypos) ;

	//if (dot(normal, eyedir) < -0.1) normal = -normal;

	vec3 reflectdir = normalize( reflect(-lightdir, normal) );

    gl_FragColor =  vec4(0.1,0.1,0.1,0) + 
					mycolor * mylight_color * max( dot(lightdir, normal), 0.0) +
					mycolor * mylight_color * pow( max( dot(reflectdir, eyedir), 0.0 ), 60.0 );
    //gl_FragColor = mycolor;
}
