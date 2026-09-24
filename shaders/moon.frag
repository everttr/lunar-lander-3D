#version 330 compatibility

uniform vec3  uNoiseOffset; // offset for the noise texture on the landscape

uniform vec3  uLLPos;		// 3D position of the lunar lander
uniform float uLLRadius;	// shadow radius of lunar lander

in	vec3	vPOS;		// 3D position
in  vec2	vST;		// texture coords
in  vec3	vN;			// normal vector
in  vec3	vL;			// vector from point to light
in  vec3	vE;			// vector from point to eye

const float SHADOW_FEATHER_SCALE	= 2.0;
const float SHADOW_MAX_HEIGHT	= 30.0;

const int	NOISE_DEPTH				= 5;
const float NOISE_SCALE_MIN			= .175;
const float NOISE_SCALE_INCREASE	= .5;

const vec3 MOON_LIGHT_COLOR	= vec3( 252. / 255., 133. / 255., 88. / 255. );
const vec3 MOON_DARK_COLOR	= vec3( 191. / 255., 84. / 255., 52. / 255. );
const vec3 MOON_UNLIT_COLOR	= vec3( 127. / 255., 60. / 255., 42. / 255. );
const vec3 SPECULAR_COLOR 	= vec3( 1., 1., 1. );

const float SPECULAR	= 0.0;
const float AMBIENT		= 0.15;
const float DIFFUSE		= 1.;
const float SHININESS	= 0.025;

const float PI	= 3.14159265;
const float TAU	= PI * 2.;

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
// (output is unit vector in random direction)
vec3
pRandGradient(int[3] p)
{
	int baseHash = hash(p);

	// Find the 2D unit vector
	float angle = floatConstruct(baseHash) * TAU;
	vec3 v = vec3(sin(angle), 0., cos(angle));

	// Rotate it up or down
	angle = (floatConstruct(hash(baseHash + 0x0B27A824)) - .5) * PI;
	v = v * cos(angle);
	v.y = sin(angle);

	return v;
}

float
smoothStep(float t)
{
	return t*t*(1.-t) - ((t-1.)*(t-1.) - 1.)*t;
}

float
perlinNoise(vec3 p)
{
	int baseGridPos[3] = int[3](
		int(floor(p.x)),
		int(floor(p.y)),
		int(floor(p.z)));
	int gridPos[3] = int[3](
		baseGridPos[0],
		baseGridPos[1],
		baseGridPos[2]);
	int dx, dy, dz;
	vec3 gradient, offset, inflXYZ, fractional = fract(p);
	float output = 0.;

	inflXYZ.x = 1. - fractional.x; // (influence for lower X corners)
	for (dx = 0; dx <= 1; dx++)
	{
		inflXYZ.y = 1. - fractional.y; // (influence for lower Y corners)
		for (dy = 0; dy <= 1; dy++)
		{
			inflXYZ.z = 1. - fractional.z; // (influence for lower Z corners)
			for (dz = 0; dz <= 1; dz++)
			{
				// Find the gradient on this corner of the cube
				gridPos[0] = baseGridPos[0] + dx;
				gridPos[1] = baseGridPos[1] + dy;
				gridPos[2] = baseGridPos[2] + dz;
				gradient = pRandGradient(gridPos);
	
				// Get the delta between the texture coord and the corner's position
				offset = vec3(
					p.x - float(gridPos[0]),
					p.y - float(gridPos[1]),
					p.z - float(gridPos[2]));
	
				// Find the dot product and add it to the total (scaled by distance)
				output += dot(gradient, offset) * inflXYZ.x * inflXYZ.y * inflXYZ.z;

				inflXYZ.z = fractional.z; // (influence for upper Z corners)
			}
			inflXYZ.y = fractional.y; // (influence for upper Y corners)
		}
		inflXYZ.x = fractional.x; // (influence for upper X corners)
	}

	return output;
}

// (now unused noise function. Not very good...)
float
oldNoise(vec2 p)
{
	float t =
		cos(p.x * 16.123 + 2.9283)	// low-freq noise x
		+ sin(p.x * 37.536 + 3.673)	// medium-freq noise x
		+ cos(p.x * 85.267 + 1.9626) * 0.5	// high-freq noise x
		+ cos(p.y * 17.239 + 0.2356)	// low-freq noise y
		+ sin(p.y * 34.903 + 1.408)	// medium-freq noise y
		+ cos(p.y * 90.724 + 2.246) * 0.5;	// high-freq noise y
	t = clamp(t / 5. * 0.5 + 0.5, 0., 1.);
	return t;
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
	// Add some noise to make it look like the mooooon! it has divets & cavities and stuff!
	float t = 0.;
	float maxAmp = 0.;
	float currAmp;
	for (int i = 0; i < NOISE_DEPTH; i++)
	{
		currAmp = 1. / float(i + 1);
		maxAmp += currAmp;
		t += perlinNoise(vPOS * (NOISE_SCALE_MIN + NOISE_SCALE_INCREASE * i) + uNoiseOffset) * currAmp;
	}
	t = t / maxAmp + 0.5;
	vec3 col = MOON_LIGHT_COLOR * (1. - t) + MOON_DARK_COLOR * t;

	// Also make it a little brighter or darker based on its height
	// (craters should be darker? idk the rationale here)
	float tint = clamp((vPOS.y - 2.75f) / 3.6, -1., 1.);
	tint = (1. + tint * 0.165);

	// Shadow below the lunar lander
	// (first calculate 2D distance)
	float shadow = distance(vec2(vPOS.x, vPOS.z), vec2(uLLPos.x, uLLPos.z));
	// (feather it more the further away it is)
	float shadowFeather = clamp((uLLPos.y - vPOS.y) / SHADOW_MAX_HEIGHT, 0., 1.);
	shadowFeather = pow(shadowFeather, 0.65) * SHADOW_FEATHER_SCALE;
	// Calculate final shadow factor
	shadow = 1. - pow(remap(shadow, uLLRadius * (1. - shadowFeather), uLLRadius * (1. + shadowFeather), .25, 1.), 1.6);
	
	// Do a little bit of "lighting" at the end
	vec3 Normal    = normalize(vN);
	vec3 Eye       = normalize(vE);

	float lit = abs(dot(Normal, Eye));
	lit = remap(lit, 0., 1., 0.6, 1.);
	lit = lit * (1. - shadow);
	gl_FragColor = vec4( (col * lit + MOON_UNLIT_COLOR * (1. - lit)) * tint,  1. );

	// Old lighting model:
//	vec3 ambient = AMBIENT * col;
//
//	float d = max( dot(Normal,Light), 0. ) * (1. - shadow);	// only do diffuse if the light can see the point
//	vec3 diffuse = DIFFUSE * d * col;
//
//	float s = 0.;
//	if( d > 0. )	          // only do specular if the light can see the point
//	{
//		vec3 ref = normalize(  reflect( -Light, Normal )  );
//		float cosphi = dot( Eye, ref );
//		if( cosphi > 0. )
//			s = pow( max( cosphi, 0. ), SHININESS );
//	}
//	vec3 specular = SPECULAR * s * SPECULAR_COLOR.rgb * (1. - shadow);
//	gl_FragColor = vec4( ambient + diffuse + specular,  1. );
}
