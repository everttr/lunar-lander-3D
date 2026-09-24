#version 330 compatibility
// use 120 for the Mac

out  vec3  	vPOS;		// texture coords

void
main( )
{
	// Just send it through the pipeline... the interesting stuff is in the fragment shader!
	vec3 pos = gl_Vertex.xyz;
	vPOS = pos;
	gl_Position = gl_ModelViewProjectionMatrix * vec4( pos.xyz, 1. );
}
