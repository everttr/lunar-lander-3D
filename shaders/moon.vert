#version 330 compatibility
// use 120 for the Mac

out  vec3  	vPOS;		// 3D position
out  vec2  	vST;		// texture coords
out  vec3  	vN;		// normal vector
out  vec3  	vL;		// vector from point to light
out  vec3  	vE;		// vector from point to eye

const vec3 	LIGHTPOS = vec3(  0., -50., 1. );
const float PI 		= 3.14159265;
const float	TWOPI 	= 2.*PI;

void
main( )
{ 
	// Take in/push out important input variables
	vST = gl_MultiTexCoord0.st;
	vec3 vert = gl_Vertex.xyz;
	vPOS = vert;

	// setup for per-fragment lighting:
	vec4 ECposition = gl_ModelViewMatrix * vec4( vert, 1. );
	vN = normalize( gl_NormalMatrix * gl_Normal );	// normal vector
	//vL = LIGHTPOS - ECposition.xyz;		// vector from the point
	vL = normalize( vec3( 0.1, -1., -0.1 ) );
						// to the light position
	vE = vec3( 0., 0., 0. ) - ECposition.xyz;	// vector from the point
							// to the eye position 

	// normal vertex position output
	gl_Position = gl_ModelViewProjectionMatrix * vec4( vert, 1. );
}
