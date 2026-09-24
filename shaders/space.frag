#version 330 compatibility

uniform vec3	uNoiseOffset; // offset for the noise generation of the stars

in  vec3  vPOS;			// fragment local coords

const float VORONOI_SCALE	= .02f;
const float VORONOI_THRESH_MIN	= .125f;
const float VORONOI_THRESH_MAX	= .25f;
const float VORONOI_THRESH_DIFF	= VORONOI_THRESH_MAX - VORONOI_THRESH_MIN;
const float POS_INF			= 999999.f; // idk how to get IEEE float-represented positive infinity, so have this instead!

const vec3 SPACE_COLOR	= vec3( 57. / 255., 64. / 255., 124. / 255. );
const vec3 STAR_COLOR 	= vec3( 242. / 255., 233. / 255., 152. / 255. );

// VERY smart hashing & bitwise alorithms to get [0,1] float from random bit input
// modified from https://stackoverflow.com/a/17479300 (I wish I was this clever!)
int hash( int x ) {
    x += ( x << 10 );
    x ^= ( x >>  6 );
    x += ( x <<  3 );
    x ^= ( x >> 11 );
    x += ( x << 15 );
    return x;
}
int hash( int[3] v )
{
	return hash( v[0] ^ hash(v[1]) ^ hash(v[2]) );
}
float floatConstruct( int m ) {
    const int ieeeMantissa = 0x007FFFFF; // binary32 mantissa bitmask
    const int ieeeOne      = 0x3F800000; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = intBitsToFloat( m );        // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

// Gives a psuedo-random output based on the
// input point on the 3D integer grid
// (output is [0,1])
vec3
pRand3f(int[3] p)
{
	int baseHash = hash(p);

	return vec3(
		floatConstruct(baseHash),
		floatConstruct(hash(baseHash + 0x083CD816)),
		floatConstruct(hash(baseHash + 0x025B825F))
		);
}

// My own voronoi noise function!! yipee!!
// I didn't do much reading up on it honestly, I'm just assuming how they're written
// based upon my experience using them in blender. It just samples 27 nearby grid
// points offset randomly
float
voronoiDist(vec3 p)
{
	// this is the min distance from the edge
	float minDist = POS_INF;
	vec3 currDotPos;

	// Find the middle of 27 points to look outwards from
	int gridPos[3] = int[3](
		int(round(p.x)),
		int(round(p.y)),
		int(round(p.z)));
	int gridPosOffset[3] = int[3](
		gridPos[0],
		gridPos[1],
		gridPos[2]);

	for (int dx = -1; dx <= 1; dx++)
	{
		for (int dy = -1; dy <= 1; dy++)
		{
			for (int dz = -1; dz <= 1; dz++)
			{
				// Get the base dot position!
				gridPosOffset[0] = gridPos[0] + dx;
				gridPosOffset[1] = gridPos[1] + dy;
				gridPosOffset[2] = gridPos[2] + dz;
				currDotPos = vec3(
					gridPosOffset[0],
					gridPosOffset[1],
					gridPosOffset[2]);

				// Offset it randomly (at most one space over!)
				currDotPos += pRand3f(gridPosOffset) * 2. - vec3(1., 1., 1.);

				// Calculate the distance, and only use it if it's the smallest one yet encountered
				minDist = min(minDist, // choose old or new for which is smaller
						distance(currDotPos, p) // distance to center
						- (VORONOI_THRESH_MIN + VORONOI_THRESH_DIFF // offset by radius of dot
							// vvv which is randomly decided based on its grid position
							* floatConstruct(hash(gridPosOffset[0] + gridPosOffset[1] * -26452 + gridPosOffset[2] * 235))
						)
					);
			}
		}
	}

	return minDist;
}

// (this also clamps it btw)
float
remap(float t, float inMin, float inMax, float outMin, float outMax)
{
	return clamp(
		outMin + (t - inMin) / (inMax - inMin) * (outMax - outMin) 
		, outMin, outMax);
}

void
main( )
{
	// Output color is just voronoi distance with a threshold applied
	// (which is slightly different threshold per star)
	vec3 col;
	float t = voronoiDist(vPOS * VORONOI_SCALE + uNoiseOffset);
	t = remap(t, 0., -0.05, 0.05, 1.);
	t = t * t * t;
	col = STAR_COLOR * t + SPACE_COLOR * (1. - t);

	gl_FragColor = vec4(col, 1.);
}
