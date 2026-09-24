#version 330 compatibility
// use 120 for the Mac

out	 vec3	vCol;	// vertex color
out  vec3  	vN;		// normal vector
out  vec3  	vL;		// vector from point to light
out  vec3  	vE;		// vector from point to eye

void
main( )
{ 
	// Take in/push out important input variables
	vec3 vert = gl_Vertex.xyz;
	vCol = vec3(gl_Color.r, gl_Color.g, gl_Color.b);

	// setup for per-fragment lighting:
	vec4 ECposition = gl_ModelViewMatrix * vec4( vert, 1. );
	vN = normalize( gl_NormalMatrix * gl_Normal );	// normal vector
	vL = normalize( vec3( 0.1, -1., -0.1 ) );
						// to the light position
	vE = vec3( 0., 0., 0. ) - ECposition.xyz;	// vector from the point
							// to the eye position 

	// normal vertex position output
	gl_Position = gl_ModelViewProjectionMatrix * vec4( vert, 1. );
}
