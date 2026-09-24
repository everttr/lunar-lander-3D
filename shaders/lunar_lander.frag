#version 330 compatibility


in  vec3	vCol;		// vertex color
in  vec3	vN;			// normal vector
in  vec3	vL;			// vector from point to light
in  vec3	vE;			// vector from point to eye

const vec3 SPECULAR_COLOR 	= vec3( 1., 1., 1. );

const float SPECULAR	= 0.15;
const float AMBIENT		= 0.65;
const float DIFFUSE		= 1.;
const float SHININESS	= 0.125;

const float PI	= 3.14159265;
const float TAU	= PI * 2.;

void
main( )
{
	// Take in input from the vertex shader
	vec3 col		= vCol;
	vec3 Normal		= normalize(vN);
	vec3 Light		= normalize(vL);
	vec3 Eye		= normalize(vE);

	vec3 ambient = AMBIENT * col;

	float d = max( dot(Normal,Light), 0. );	// only do diffuse if the light can see the point
	vec3 diffuse = DIFFUSE * d * col;

	float s = 0.;
	if( d > 0. )	          // only do specular if the light can see the point
	{
		vec3 ref = normalize(  reflect( -Light, Normal )  );
		float cosphi = dot( Eye, ref );
		if( cosphi > 0. )
			s = pow( max( cosphi, 0. ), SHININESS );
	}
	vec3 specular = SPECULAR * s * SPECULAR_COLOR.rgb;
	gl_FragColor = vec4( ambient + diffuse + specular,  1. );
}
