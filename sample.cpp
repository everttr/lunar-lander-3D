/*
 * Program: Lunar Lander 3D: Lunar Lander Goes to Space!
 * Author: Reed Evertt
 * Email: reed@evertt.com
 * Date: 12/09/2024
 * Description: a physics-based spaceship landing game
 *   with all custom gameplay and shader code.
 *   Sorry about the mess of code, I was working on a
 *   tight deadline and off a term-long project template!
 */

#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <map>


#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#define DEG_2_RAD	((float)(F_PI/180.f))
#define RAD_2_DEG	((float)(180.f/F_PI))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glut.h"

// GLM stuff
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// title of these windows:

const char *WINDOWTITLE = "CS 450 FINAL Project - Lunar Lander";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};


// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0f, .0f, .0f, 1.f };
const GLenum  FOGMODE     = GL_LINEAR;
const GLfloat FOGDENSITY  = 0.5f;
const GLfloat FOGSTART    = 5.f;
const GLfloat FOGEND      = 40.f;

// scene parameters:

const float PLANE_SIZE = 6.5f;
const int PLANE_RES = 48;

// for lighting:

const float	WHITE[] = { 1.,1.,1.,1. };
const float BALL_COLOR[] = { 1.f, 0.525f, 0.3f, 1.f };
const float BALL_SHININESS = 0.725f;
const float SHADOW_COLOR[] = { 0.05f, 0.f, 0.1f, 1.f };
const float SHADOW_SHININESS = 0.f;
const float WALL_BACK_COLOR[] = { 0.3f, 1.f, 0.2f, 1.f };
const float WALL_SIDE_COLOR[] = { 0.3f, 0.2f, 1.f, 1.f };
const float WALL_SHININESS = 0.175f;
const float FLOOR_COLOR[] = { 1.f, 1.f, 1.f, 1.f };
const float FLOOR_SHININESS = 0.2f;

// for animation:

const int MS_PER_CYCLE = 50000;		// 10000 milliseconds = 10 seconds

// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
#define DEMO_Z_FIGHTING
#define DEMO_DEPTH_BUFFER
//#define USE_CUBE

#define MOON_RES 96
#define MOON_WIDTH 100.f
#define MOON_WIDTH_HALF (MOON_WIDTH * .5f)
#define MOON_NOISE_DEPTH 10

const float	LL_RADIUS = .5f;
const float	LL_Y_MAX = 30.f;
const float	LL_STARTING_Y_MIN = 20.f;
const float	LL_STARTING_Y_MAX = 30.f;
const float	LL_STARTING_SPEED_MIN = 1.5f;
const float	LL_STARTING_SPEED_MAX = 3.5f;
const float	LL_GRAVITY = -.8f; // in units/sec/sec
const float	LL_THRUST_UPWARDS = 0.75f; // in units/sec/sec
const float	LL_LINEAR_DRAG = 1.25f; // in units/sec/sec
const float	LL_ANGULAR_ACC = 30.f * DEG_2_RAD; // in degrees/sec/sec
const float	LL_ANGULAR_DRAG = 60.f * DEG_2_RAD; // in degrees/sec/sec
const float	LL_LANDING_ANGLE_ACCEPTANCE = 0.675f; // Approx. factor for how upright it must be to not crash
const float	LL_MAX_CRASHING_SPEED = 2.125f; // Max speed it must be travelling into the ground to crash

const float ARROW_FLICKER_PERIOD = 1.25f;
// (just disable the flicker, not visually helpful at all, really)
const float ARROW_FLICKER_PERIOD_VISIBLE = 2.0f;// ARROW_FLICKER_PERIOD * .5f;
const glm::vec3 ARROW_UNDEFINED_NORMAL = glm::vec3(2.f, 2.f, 2.f); // an impossible normal to represent it being undefined
const float ARROW_SCALE = .75f;

const bool	CAMERA_INHERENTS_LL_ROT = true;

const float	PHYSICS_STEP_MAX = 1.f / 15.f;

char* FLAME_OBJ_PATH = "objs/flame.obj";
char* LL_OBJ_PATH = "objs/lunar_lander.obj";
char* ARROW_OBJ_PATH = "objs/arrow.obj";
char* EXPLOSION_OBJ_PATH = "objs/explosion.obj";
char* MOON_VERT_PATH = "shaders/moon.vert";
char* MOON_FRAG_PATH = "shaders/moon.frag";
char* SPACE_VERT_PATH = "shaders/space.vert";
char* SPACE_FRAG_PATH = "shaders/space.frag";
char* LL_VERT_PATH = "shaders/lunar_lander.vert";
char* LL_FRAG_PATH = "shaders/lunar_lander.frag";

// I really forgot there was already an implementation of this... oh well.
//struct Vec3
//{
//	float x, y, z;
//	Vec3(const float x, const float y, const float z)
//		: x(x), y(y), z(z) { }
//	Vec3 operator+(const Vec3& other) const
//	{
//		return Vec3(this->x + other.x, this->y + other.y, this->z + other.z);
//	}
//	Vec3 operator-(const Vec3& other) const
//	{
//		return Vec3(this->x - other.x, this->y - other.y, this->z - other.z);
//	}
//	Vec3 operator*(const float& scale) const
//	{
//		return Vec3(this->x * scale, this->y * scale, this->z * scale);
//	}
//	Vec3 operator/(const float& divisor) const
//	{
//		return Vec3(this->x / divisor, this->y / divisor, this->z / divisor);
//	}
//	float dot(const Vec3& other) const
//	{
//		return this->x * other.x + this->y * other.y + this->z * other.z;
//	}
//	Vec3 cross(const Vec3& other) const
//	{
//		return Vec3(
//			this->y * other.z - this->z * other.y,
//			-this->x * other.z + this->z * other.x,
//			this->x * other.y - this->y * other.x);
//	}
//	float calcLength() const
//	{
//		return sqrt(x * x + y * y + z * z);
//	}
//	Vec3 calcNormalized() const
//	{
//		return *this / this->calcLength();
//	}
//};

// Optimizes a position on the line for being closest to the given point
// Returns true iff closest position along line was within line segment
static bool linePointOptimization(const glm::vec3& start, const glm::vec3& end, const glm::vec3& point, glm::vec3& closest)
{
	glm::vec3 dir{ (end - start) };
	float len = glm::length(dir);
	dir = dir / len;
	float distAlongLine = glm::dot(point - start, dir);
	if (distAlongLine < 0.f)
	{
		closest = start;
		return false;
	}
	if (distAlongLine > 1.f)
	{
		closest = end;
		return false;
	}
	closest = start + dir * distAlongLine;
	return true;
}

struct Tri
{
	glm::vec3 p1, p2, p3;
	Tri()
		: p1(glm::vec3(0.f)),
		p2(glm::vec3(0.f)),
		p3(glm::vec3(0.f))
	{ }
	Tri(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
		: p1(p1), p2(p2), p3(p3)
	{ }
	Tri(const float x1, const float y1, const float z1,
		const float x2, const float y2, const float z2, 
		const float x3, const float y3, const float z3)
		: p1(glm::vec3(x1, y1, z1)),
		p2(glm::vec3(x2, y2, z2)),
		p3(glm::vec3(x3, y3, z3))
	{ }

	glm::vec3 calcNormal() const
	{
		glm::vec3 normal = glm::cross(glm::normalize(p3 - p2), glm::normalize(p2 - p1));
		// This is a hack only for this specific game because the normals will always face upwards
		return normal.y < 0 ? -normal : normal;
	}

	glm::vec3 calcTangent() const
	{
		return glm::normalize(p2 - p1);
	}

	glm::vec3 calcMiddle() const
	{
		return (p1 + p2 + p3) / 3.f;
	}

	// Finds the height on the plane that the triangle represents on the given vertical line
	// (assumes triangle represents a plane and that the plane is angled straight up)
	// (may not be constrained to the dimensions of the triangle)
	float calcHeight(float x, float z)
	{
		// equation of a plane: d = ax + by + cz
		// where (a, b, c) is the normal (per-element slopes) of the plane
		glm::vec3 normal = calcNormal();
		float d = normal.x * p1.x + normal.y * p1.y + normal.z * p1.z;
		// now solve for y using the given x & z: y = (d - ax - cz) / b
		return (d - normal.x * p1.x - normal.z * p1.z) / normal.y;
	}
};

// Represents a moving object in the world (with simple physics kinda)
// (only represents spheres (sorry))
class GameObject
{
private:
	glm::vec3	pos, vel, rot, angVel;
	float	radius;
	int		defaultModel;
public:
	GameObject()
		: pos(glm::vec3(0.f, 0.f, 0.f)),
		vel(glm::vec3(0.f, 0.f, 0.f)),
		rot(glm::vec3(0.f, 0.f, 0.f)),
		angVel(glm::vec3(0.f, 0.f, 0.f)),
		radius(1.f),
		defaultModel(0)
	{ }
	GameObject(float radius, int defaultModel)
		: pos(glm::vec3(0.f, 0.f, 0.f)),
		vel(glm::vec3(0.f, 0.f, 0.f)),
		rot(glm::vec3(0.f, 0.f, 0.f)),
		angVel(glm::vec3(0.f, 0.f, 0.f)),
		radius(radius),
		defaultModel(defaultModel)
	{ }

	void reset(glm::vec3 startingPos, glm::vec3 startingVel)
	{
		pos = startingPos;
		vel = startingVel;
		angVel = glm::vec3(0.f, 0.f, 0.f);
		rot = glm::vec3(0.f, 0.f, 0.f);
	}

	glm::vec3 getPos() const
	{ return pos; }
	glm::vec3 getVel() const
	{ return vel; }
	float getSpeed() const
	{ return (float)glm::length(vel); }
	glm::vec3 getRot() const
	{ return rot; }
	float getRadius() const
	{ return radius; }
	int getDefaultModel() const
	{ return defaultModel; }

	void
	getTransformMatrices(glm::mat4& rotation, glm::mat4& translate) const
	{
		rotation = glm::mat4(1.f);
		rotation = glm::rotate(rotation, rot.x, glm::vec3(1.f, 0.f, 0.f));
		rotation = glm::rotate(rotation, rot.y, glm::vec3(0.f, 1.f, 0.f));
		rotation = glm::rotate(rotation, rot.z, glm::vec3(0.f, 0.f, 1.f));

		translate = glm::mat4(1.f);
		translate = glm::translate(translate, pos);
	}

	glm::mat4
	getTransformMatrix() const
	{
		glm::mat4 rotate, trans;
		getTransformMatrices(rotate, trans);
		return trans * rotate;
	}

	glm::vec3 getUpVector() const
	{
		return (glm::vec3)(this->getTransformMatrix() * glm::vec4(0.f, 1.f, 0.f, 0.f));
	}

	glm::vec3 getForwardVector() const
	{
		return (glm::vec3)(this->getTransformMatrix() * glm::vec4(0.f, 0.f, 1.f, 0.f));
	}

	void addVel(const glm::vec3& delta)
	{
		vel += delta;
	}
	void addAngVel(const glm::vec3& delta)
	{
		angVel += delta;
	}

	void physicsStep(const float delta, const bool linearDrag = false, const bool angularDrag = false)
	{
		// Apply velocity
		pos += vel * delta;
		rot += angVel * delta;
		// (clamp positions & bounce)
		if (pos.y < 0.f)
		{
			pos.y = 0.f;
			vel.y = -vel.y;
		}
		if (pos.y > LL_Y_MAX)
		{
			pos.y = LL_Y_MAX;
			vel.y = -vel.y;
		}
		if (pos.x < 0.f)
		{
			pos.x = 0.f;
			vel.x = -vel.x;
		}
		if (pos.x > MOON_WIDTH)
		{
			pos.x = MOON_WIDTH;
			vel.x = -vel.x;
		}
		if (pos.z < 0.f)
		{
			pos.z = 0.f;
			vel.z = -vel.z;
		}
		if (pos.z > MOON_WIDTH)
		{
			pos.z = MOON_WIDTH;
			vel.z = -vel.z;
		}
		// (wrap angle)
		rot.x = rot.x >= 0.f ? fmod(rot.x, F_2_PI) : (F_2_PI - abs(fmod(rot.x, F_2_PI)));
		rot.y = rot.y >= 0.f ? fmod(rot.y, F_2_PI) : (F_2_PI - abs(fmod(rot.y, F_2_PI)));
		rot.z = rot.z >= 0.f ? fmod(rot.z, F_2_PI) : (F_2_PI - abs(fmod(rot.z, F_2_PI)));

		// Apply acceleration (just gravity)
		vel.y += LL_GRAVITY * delta;

		// Apply drag because HOLY it starts to spin & move fast
		if (linearDrag)
		{
			glm::vec3 linDrag = vel;
			linDrag.y = 0.f;
			if (abs(linDrag.x) > 0.00001f && abs(linDrag.x) > 0.00001f)
			{
				linDrag = glm::normalize(linDrag) * LL_LINEAR_DRAG * delta;
				glm::vec3 newVel = vel - linDrag;
				// Prevent it from overcorrecting/clamp it
				if (signbit(newVel.x) != signbit(vel.x))
					newVel.x = 0.f;
				if (signbit(newVel.y) != signbit(vel.y))
					newVel.y = 0.f;
				if (signbit(newVel.z) != signbit(vel.z))
					newVel.z = 0.f;
				vel = newVel;
			}
		}
		if (angularDrag && abs(angVel.x) > 0.00001f && abs(angVel.y) > 0.00001f && abs(angVel.z) > 0.00001f)
		{
			glm::vec3 angDrag = glm::normalize(angVel) * LL_ANGULAR_DRAG * delta;
			glm::vec3 newAngVel = angVel - angDrag;
			// Prevent it from overcorrecting/clamp it
			if (signbit(newAngVel.x) != signbit(angVel.x))
				newAngVel.x = 0.f;
			if (signbit(newAngVel.y) != signbit(angVel.y))
				newAngVel.y = 0.f;
			if (signbit(newAngVel.z) != signbit(angVel.z))
				newAngVel.z = 0.f;
			angVel = newAngVel;
		}
	}

	glm::vec3 optimizePointOnTri(const Tri& tri) const
	{
		// Calc 3 simple optimization problems to find closest point on triangle 
		glm::vec3 p1, p2;
		linePointOptimization(tri.p1, tri.p2, this->pos, p1);
		linePointOptimization(tri.p2, tri.p3, this->pos, p2);
		linePointOptimization(p1, p2, this->pos, p1);
		return p1;
	}

	// Triangle-sphere collision (well, really just an intersection check...)
	bool intersectionCheckTri(const Tri& tri) const
	{
		glm::vec3 closest = optimizePointOnTri(tri);

		// Then just do a distance check!
		return glm::length(closest - this->pos) <= this->radius;
	}
};

struct MoonWave
{
	float freq;
	float offset;
	float amplitude;
	glm::vec3 center;
	MoonWave()
		: freq(1.f),
		offset(0.f),
		amplitude(1.f),
		center(glm::vec3(0.f, 0.f, 0.f))
	{}
	float getHeight(float x, float z) const
	{
		// (where V is the coordinate on the flat plane)
		// a * (sin(f * |V - c| + o) * .5 + .5)
		return
			(sin(freq
				* glm::distance(
					glm::vec3((float)x, 0.f, (float)z),
					center)
				+ offset)
				* .5f + .5f) * amplitude;
	}
	// vvvv unimplemented because I forgot how to do calculus lol! Also unsure of how collision would work
	//// pretty much the first order derivative
	//glm::vec3 getNormal(float x, float z) const
	//{
	//	// (where V is the coordinate on the flat plane)
	//	// a * (sin(f * |V - c| + o) * .5 + .5)
	//	return
	//		(sin(freq
	//			* glm::distance(
	//				glm::vec3((float)x, 0.f, (float)z),
	//				center)
	//			+ offset)
	//			* .5f + .5f) * amplitude;
	//}
};

enum CameraView : short
{
	AttachedView,
	OverheadView, // unused because it's incredibly unhelpful
	SideView,
	SideView2,
};

enum GameState : short
{
	PlayingState,
	PausedState,
	GameEndState,
};

// non-constant global variables:

int		LastTime;					// last time in ms
GameObject	ll;					// Lunar lander object
CameraView	cameraMode;			// Where the camera is positioned
GameState	gameState;
std::map<char, bool>	inputMap;
float	moonHeight[MOON_RES][MOON_RES];	// the height of the moon at each grid point
MoonWave moonWaves[MOON_NOISE_DEPTH];	// sine waves used to generate the height of the moon
int		SpaceList;				// the enclosing space sphere
int		MoonList;				// moon itself
int		FlameList;				// default little flame (doesn't make physical sense (oh well)
int		LLList;					// lunar lander module
int		ArrowList;				// helper arrow for orientation
int		ExplosionList;			// explosion for when you lose
glm::vec3 LastArrowNormal;		// the last ground normal the arrow was pretending to be
bool	DidWin;
float	RealTime;				// (sec) Real time spent in game, not counting pausing/resetting
float	TimeElapsed;			// (sec) Time spent in this current game
float	LastTimeDelta;			// (sec) the time delta from the last physics/game logic update
float	RecordTime;				// (sec) High (low?) score in the current game
int		MainWindow;				// window id for main graphics window
int		DebugOn;				// Logs some extra stuff to the console
int		ActiveButton;			// current button that is down
int		Xmouse, Ymouse;			// mouse values

// function prototypes:

void	Animate( );
void	Display( );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
void			HsvRgb( float[3], float [3] );
void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);
float			Unit(float [3]);


// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}


float
Ranf( float low, float high )
{
        float r = (float) rand();               // 0 - RAND_MAX
        float t = r  /  (float) RAND_MAX;       // 0. - 1.

        return   low  +  t * ( high - low );
}

// call this if you want to force your program to use
// a different random number sequence every time you run it:
void
TimeOfDaySeed( )
{
	struct tm y2k;
	y2k.tm_hour = 0;    y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 2000; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time_t  now;
	time( &now );
	double seconds = difftime( now, mktime(&y2k) );
	unsigned int seed = (unsigned int)( 1000.*seconds );    // milliseconds
	srand( seed );
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
//#include "setlight.cpp"
#include "osusphere.cpp"
//#include "osucone.cpp"
//#include "osutorus.cpp"
//#include "bmptotexture.cpp"
#include "loadobjfile.cpp"
//#include "keytime.cpp"
#include "glslprogram.cpp"
#include <stdexcept>
//#include "vertexbufferobject.cpp"

// helper functions for a bunch of stuff:

float
clamp(const float& t, const float& min, const float& max)
{
	return t < min ? min : t > max ? max : t;
}

int
clamp(const int& t, const int& min, const int& max)
{
	return t < min ? min : t > max ? max : t;
}

float
min(const float& num1, const float& num2)
{
	return num1 < num2 ? num1 : num2;
}

float
max(const float& num1, const float& num2)
{
	return num1 > num2 ? num1 : num2;
}

GLSLProgram LLShaders;
GLSLProgram SpaceShaders;
GLSLProgram MoonShaders;

void
loadShaders()
{
	// Compile the shaders
	LLShaders.Init();
	if (!LLShaders.Create(LL_VERT_PATH, LL_FRAG_PATH))
		fprintf(stderr, "ERROR: Lunar Lander shaders didn't compile\n");
	SpaceShaders.Init();
	if (!SpaceShaders.Create(SPACE_VERT_PATH, SPACE_FRAG_PATH))
		fprintf(stderr, "ERROR: Space shaders didn't compile\n");
	MoonShaders.Init();
	if (!MoonShaders.Create(MOON_VERT_PATH, MOON_FRAG_PATH))
		fprintf(stderr, "ERROR: Moon shaders didn't compile\n");

	// Set the globals
	float llPosArr[3] = { 0.f, 1000.f, 0.f };
	MoonShaders.SetUniformVariable("uLLPos", llPosArr);
	MoonShaders.SetUniformVariable("uLLRadius", ll.getRadius());
}

float
calcExactMoonHeightGrid(float gridPosX, float gridPosZ)
{
	float height = 0.f;
	for (int i = 0; i < MOON_NOISE_DEPTH; i++)
	{
		height += moonWaves[i].getHeight(gridPosX, gridPosZ);
	}
	return height;
}

float
calcExactMoonHeightWorld(float worldX, float worldZ)
{
	return calcExactMoonHeightGrid(
		worldX / MOON_WIDTH * (float)MOON_RES,
		worldZ / MOON_WIDTH * (float)MOON_RES);
}

glm::vec3
getMoonPos(int x, int z)
{
	return glm::vec3(
		((float)x / MOON_RES) * MOON_WIDTH,
		moonHeight[x][z],
		((float)z / MOON_RES) * MOON_WIDTH);
}

void
GenerateMoonscape()
{
	// Generate height map
	float t;
	for (int i = 0; i < MOON_NOISE_DEPTH; i++)
	{
		t = Ranf(0.f, 1.f); // to make freq inv. proportional to amplitude
		moonWaves[i].freq = F_2_PI / (3.5f + t * 20.f); // (divisor is the period)
		moonWaves[i].offset = Ranf(0.f, F_2_PI);
		moonWaves[i].amplitude =  .175f + t * 2.0f;
		moonWaves[i].center = glm::vec3(
			Ranf(0.f, MOON_WIDTH),
			Ranf(-2.f, 2.f),
			Ranf(0.f, MOON_WIDTH));
	}
	int x, z;
	for (x = 0; x < MOON_RES; x++)
	{
		for (z = 0; z < MOON_RES; z++)
		{
			moonHeight[x][z] = calcExactMoonHeightGrid((float)x, (float)z);
		}
	}

	// Generate the offset mesh
	glNewList(MoonList, GL_COMPILE);
	glm::vec3 p;
	int dz, realZ;
	for (z = 0; z < MOON_RES - 1; z++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (x = 0; x < MOON_RES; x++)
		{
			for (dz = 0; dz <= 1; dz++)
			{
				realZ = z + dz;
				glTexCoord2f((float)x / (float)MOON_RES, (float)realZ / (float)MOON_RES);
				p = getMoonPos(x, realZ);
				glVertex3f(p.x, p.y, p.z);
			}
		}
		glEnd();
	}
	glEndList();
}

void
ResetGameState()
{
	// Reset game variables
	cameraMode = SideView2;
	gameState = PausedState;
	TimeElapsed = 0.f;
	LastArrowNormal = ARROW_UNDEFINED_NORMAL;
	
	// Reset lunar lander
	float startingSpeed = Ranf(LL_STARTING_SPEED_MIN, LL_STARTING_SPEED_MAX);
	float theta = Ranf(0.f, F_2_PI);
	ll.reset(
		glm::vec3(
			MOON_WIDTH_HALF,
			Ranf(LL_STARTING_Y_MIN, LL_STARTING_Y_MAX),
			MOON_WIDTH_HALF),
		glm::vec3(
			sin(theta) * startingSpeed,
			0.f,
			cos(theta) * startingSpeed));
	
	// Re-initialize shaders with new seeds
	float shaderNoiseOffset[3] = { Ranf(-1000.f, 1000.f), Ranf(-1000.f, 1000.f), Ranf(-1000.f, 1000.f) };
	SpaceShaders.SetUniformVariable("uNoiseOffset", shaderNoiseOffset);
	for (int i = 0; i < 3; i++)
		shaderNoiseOffset[i] = Ranf(-1000.f, 1000.f);
	MoonShaders.SetUniformVariable("uNoiseOffset", shaderNoiseOffset);

	// Do the most important part: generate a new level!
	GenerateMoonscape();
}

void
EndTheGame(const bool succeeded)
{
	gameState = GameEndState;

	DidWin = succeeded;

	if (succeeded && (TimeElapsed < RecordTime || RecordTime < 0.f))
		RecordTime = TimeElapsed;

	// that's it!
}

// Returns if ll has hit that triangle
// (crashed is defined if it was a hit, and is true iff landing wasn't safe)
bool
DidLunarLanderLand(const GameObject& ll, const Tri& tri, bool& crashed)
{
	// Check if it was a hit
	if (ll.intersectionCheckTri(tri))
	{
		crashed = true; // default to true, only false if passes all the checks
		glm::vec3 normal = tri.calcNormal();
		glm::vec3 upVec = ll.getUpVector();
		glm::vec3 vel = ll.getVel();

		fprintf(stdout, "Hit the ground! ground normal: (%.2f, %.2f, %.2f), ll up: (%.2f, %.2f, %.2f), ll vel: (%.2f, %.2f, %.2f)\n",
			normal.x, normal.y, normal.z,
			upVec.x, upVec.y, upVec.z,
			vel.x, vel.y, vel.z);

		// if so, first check that it was upright enough
		if (glm::dot(upVec, normal) >= LL_LANDING_ANGLE_ACCEPTANCE)
		{
			// Second check for if it's going slow enough
			if (glm::dot(vel, -normal) < LL_MAX_CRASHING_SPEED)
				crashed = false;
		}
		return true;
	}
	return false;
}

bool
TestForAndResetInput(char c)
{
	// Tests for the input
	bool wasThere;
	try
	{
		wasThere = inputMap.at(c);
	}
	catch (std::out_of_range e)
	{
		wasThere = false;
	}

	// Resets the input (or puts it there in the first place)
	inputMap.insert_or_assign(c, false);

	return wasThere;
}

bool
TestForAndResetInput(char c1, char c2)
{
	// Tests for the input (in the first character)
	bool wasThere;
	try
	{
		wasThere = inputMap.at(c1);
	}
	catch (std::out_of_range e)
	{
		wasThere = false;
	}

	// Then in the second character
	if (!wasThere)
	{
		try
		{
			wasThere = inputMap.at(c2);
		}
		catch (std::out_of_range e)
		{
			wasThere = false;
		}
	}

	// Resets the input (or puts it there in the first place)
	inputMap.insert_or_assign(c1, false);
	inputMap.insert_or_assign(c2, false);

	return wasThere;
}

// main program:

int
main(int argc, char* argv[])
{
	// Initialize the randomization
	TimeOfDaySeed();

	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit(&argc, argv);

	// setup all the graphics stuff:

	InitGraphics();

	// create the display lists that **will not change**:

	InitLists();

	// Initialize the stuff!

	inputMap = std::map<char, bool>();
	ll = GameObject(LL_RADIUS, LLList);
	loadShaders();
	LastTime = 0;
	RealTime = 0.f;
	RecordTime = -1.f;
	LastTimeDelta = 0.f;
	LastArrowNormal = ARROW_UNDEFINED_NORMAL;

	// Starts the game!

	ResetGameState( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate()
{
	// Calculate time
	int newTime = glutGet(GLUT_ELAPSED_TIME);
	RealTime = newTime / 1000.f;
	float timeDelta = (newTime - LastTime) / 1000.f;
	if (timeDelta > PHYSICS_STEP_MAX)
		timeDelta = PHYSICS_STEP_MAX;
	LastTime = newTime;
	LastTimeDelta = timeDelta;

	float upThrust{ 0.f };
	float pitchAcc{ 0.f }, yawAcc{ 0.f }, rollAcc{ 0.f };
	bool angularDrag{ false }, linearDrag{ false };
	glm::vec3 llPos;

	// change camera modes
	if (TestForAndResetInput('1', '!') && cameraMode != AttachedView)
	{
		cameraMode = AttachedView;
		fprintf(stdout, "New camera mode: attached\n");
	}
	if (TestForAndResetInput('2', '@') && cameraMode != SideView)
	{
		cameraMode = SideView;
		fprintf(stdout, "New camera mode: side\n");
	}
	if (TestForAndResetInput('3', '#') && cameraMode != SideView2)
	{ 
		cameraMode = SideView2;
		fprintf(stdout, "New camera mode: side 2\n");
	}

	switch (gameState)
	{
		case PausedState:
			// Just unpause if button is pressed again
			if (!TestForAndResetInput('\t'))
				break;
			else
				gameState = PlayingState;
		case PlayingState:
			// Consider User Input (resets flags in function)
			if (TestForAndResetInput('\t'))
			{ // pause
				gameState = PausedState;
				break;
			}
			if (TestForAndResetInput('q', 'Q'))
			{ // pitch inc.
				pitchAcc += LL_ANGULAR_ACC * timeDelta;
			}
			if (TestForAndResetInput('a', 'A'))
			{ // pitch dec.
				pitchAcc -= LL_ANGULAR_ACC * timeDelta;
			}
			if (TestForAndResetInput('w', 'W'))
			{ // yaw inc.
				yawAcc += LL_ANGULAR_ACC * timeDelta;
			}
			if (TestForAndResetInput('s', 'S'))
			{ // yaw dec.
				yawAcc -= LL_ANGULAR_ACC * timeDelta;
			}
			if (TestForAndResetInput('e', 'E'))
			{ // roll inc.
				rollAcc += LL_ANGULAR_ACC * timeDelta;
			}
			if (TestForAndResetInput('d', 'D'))
			{ // roll dec.
				rollAcc -= LL_ANGULAR_ACC * timeDelta;
			}
			if (TestForAndResetInput('f', 'F'))
			{ // activate angular drag (anti-rotation thrusters?)
				pitchAcc = 0.f;
				yawAcc = 0.f;
				rollAcc = 0.f;
				angularDrag = true;
			}
			if (TestForAndResetInput(' '))
			{ // thrust upwards
				upThrust += LL_THRUST_UPWARDS;
			}
			if (TestForAndResetInput('z', 'Z'))
			{ // thrust downwards
				upThrust -= LL_THRUST_UPWARDS;
			}
			if (TestForAndResetInput('r', 'R'))
			{ // activate linear drag (anti-movement thrusters?)
				linearDrag = true;
			}


			// Physics step
			ll.addAngVel(glm::vec3(pitchAcc, yawAcc, rollAcc));
			ll.addVel(ll.getUpVector() * upThrust);
			ll.physicsStep(timeDelta, linearDrag, angularDrag);
			// Collision check
			{
				// Find max # of triangles it could collide with
				// (using 2D overhead quick culling)
				llPos = ll.getPos();
				int minX = clamp((int)floor((llPos.x - ll.getRadius()) / MOON_WIDTH * (float)MOON_RES), 0, MOON_RES - 2);
				int maxX = clamp((int)ceil((llPos.x + ll.getRadius()) / MOON_WIDTH * (float)MOON_RES), 0, MOON_RES - 1);
				int minZ = clamp((int)floor((llPos.z - ll.getRadius()) / MOON_WIDTH * (float)MOON_RES), 0, MOON_RES - 2);
				int maxZ = clamp((int)ceil((llPos.z + ll.getRadius()) / MOON_WIDTH * (float)MOON_RES), 0, MOON_RES - 1);
				Tri tri;
				bool hit{ false }, crashed{ false }, temp{ true };
				for (int z = minZ; z < maxZ; z++) // max is exclusive
				{
					for (int x = minX; x < maxX; x++) // max is exclusive
					{
						// "lower-left" triangle
						tri.p1 = getMoonPos(x	 , z	);
						tri.p2 = getMoonPos(x + 1, z	);
						tri.p3 = getMoonPos(x	 , z + 1);
						if (DidLunarLanderLand(ll, tri, temp))
						{
							hit = true;
							// 'or' it, because if any collision was a crash, then it crashed!
							crashed |= temp;
						}
						// "upper-right" triangle
						tri.p1 = getMoonPos(x + 1, z	);
						tri.p2 = getMoonPos(x + 1, z + 1);
						tri.p3 = getMoonPos(x	 , z + 1);
						if (DidLunarLanderLand(ll, tri, temp))
						{
							hit = true;
							crashed |= temp; 
						}
					}
				}
				// Deal with the aftermath
				if (hit)
				{
					// Either way, game is over
					EndTheGame(!crashed);
				}
				else // Only add to your time if the game didn't end
				{
					TimeElapsed += timeDelta;
				}
			}
			break;
		case GameEndState:
			// Restart game if player presses "start"
			if (TestForAndResetInput('\t'))
			{
				ResetGameState();
			}
			break;
	}

	// Update tracked position in shaders
	llPos = ll.getPos();
	float llPosArr[3] = { llPos.x, llPos.y, llPos.z };
	MoonShaders.SetUniformVariable("uLLPos", llPosArr);

	// Reset all input flags
	// (actually that's really annoying to do with a map in this version so maybe don't...
	//		this leads to bugs, but I don't care that much if there's a frame of queued input after unpausing)

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );

	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( 70.f, 1.f,	0.025f, 5000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// Get base transforms for the lunar lander
	glm::mat4 llRotation, llTranslation;
	ll.getTransformMatrices(llRotation, llTranslation);
	glm::mat4 llFullTransform = llTranslation * llRotation;
	glm::vec3 llPos = ll.getPos();
	glm::vec3 llRot = ll.getRot();

	// Position the camera
	glm::mat4 cameraTransformRot = glm::mat4(1.);
	glm::mat4 cameraTransformTrans = glm::mat4(1.);
	{
		// Make some variables that will be modified based on various conditions
		// (these are all relative to the ll)
		glm::vec3 cameraPos = glm::vec3(0., 0., 0.);
		glm::vec3 lookPos = glm::vec3(0., 0., 0.);
		glm::vec3 upVec = glm::vec3(0., 1., 0.);

		// Special camera mode for when the game ended
		if (gameState == GameEndState)
		{
			// Make it spin around the exploded (or possibly landed...) LL
			float theta = fmod(RealTime * 55.f, 360.f) * DEG_2_RAD;
			cameraTransformRot = glm::rotate(cameraTransformRot, theta, glm::vec3(0.f, 1.f, 0.f));
			cameraTransformRot = glm::rotate(cameraTransformRot, -32.5f * DEG_2_RAD, glm::vec3(1.f, 0.f, 0.f));
			cameraTransformTrans = glm::translate(cameraTransformTrans, glm::vec3(sin(theta), .8f, cos(theta)) * 5.25f);
			llFullTransform = llTranslation;
		}
		// Offset it slightly differently based on specified camera mode
		else
		{
			switch (cameraMode)
			{
				case AttachedView:
					cameraTransformRot = glm::rotate(cameraTransformRot, 105.f * DEG_2_RAD, glm::vec3(0.f, 1.f, 0.f));
					cameraTransformRot = glm::rotate(cameraTransformRot, -38.5f * DEG_2_RAD, glm::vec3(1.f, 0.f, 0.f));
					cameraTransformRot = glm::rotate(cameraTransformRot, 15.f * DEG_2_RAD, glm::vec3(0.f, 0.f, 1.f));
					cameraTransformTrans = glm::translate(cameraTransformTrans, glm::vec3(.35f, .375f, -.475f));
					// Add a little noise if it's going fast enough
					if (gameState == PlayingState)
					{
						float noise = clamp(ll.getSpeed() / 8.f, 0.f, 1.f);
						noise = noise * noise * noise;
						noise = noise * 0.4f * DEG_2_RAD;
						cameraTransformTrans = glm::rotate(cameraTransformTrans, Ranf(-noise, noise), glm::vec3(1.f, 0.f, 0.f));
						cameraTransformTrans = glm::rotate(cameraTransformTrans, Ranf(-noise, noise), glm::vec3(0.f, 1.f, 0.f));
						cameraTransformTrans = glm::rotate(cameraTransformTrans, Ranf(-noise, noise), glm::vec3(0.f, 0.f, 1.f));
					}
					break;
				case OverheadView:
					cameraTransformRot = glm::rotate(cameraTransformRot, -90.f * DEG_2_RAD, glm::vec3(1.f, 0.f, 0.f));
					cameraTransformTrans = glm::translate(cameraTransformTrans, glm::vec3(0.f, 4.f, 0.f));
					break;
				case SideView:
					cameraTransformRot = glm::rotate(cameraTransformRot, 90.f * DEG_2_RAD, glm::vec3(0.f, 1.f, 0.f));
					cameraTransformTrans = glm::translate(cameraTransformTrans, glm::vec3(4.f, 2.f, 0.f));
					cameraTransformRot = glm::rotate(cameraTransformRot, -35.f * DEG_2_RAD, glm::vec3(1.f, 0.f, 0.f));
					break;
				case SideView2:
					cameraTransformRot = glm::rotate(cameraTransformRot, 180.f * DEG_2_RAD, glm::vec3(0.f, 1.f, 0.f));
					cameraTransformTrans = glm::translate(cameraTransformTrans, glm::vec3(0.f, 2.f, -4.f));
					cameraTransformRot = glm::rotate(cameraTransformRot, -35.f * DEG_2_RAD, glm::vec3(1.f, 0.f, 0.f));
					break;
			}
		}
	}

	// set the fog parameters:

	glFogi(GL_FOG_MODE, FOGMODE);
	glFogfv(GL_FOG_COLOR, FOGCOLOR);
	glFogf(GL_FOG_DENSITY, FOGDENSITY);
	glFogf(GL_FOG_START, FOGSTART);
	glFogf(GL_FOG_END, FOGEND);
	glEnable(GL_FOG);

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable(GL_NORMALIZE);

	// specify shading & lighting:
	glDisable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);

	// Set the base projection for the rest of everything
	// (from now on everything will be relative to world coordinates)
	glm::mat4 fullProjectionTransform = glm::inverse((llFullTransform * cameraTransformTrans) * cameraTransformRot);
	glLoadMatrixf(glm::value_ptr(fullProjectionTransform));
	
	// Draw the moon!!!
	MoonShaders.Use();
	glCallList(MoonList);
	MoonShaders.UnUse();

	// Draw space!!!
	SpaceShaders.Use();
	glCallList(SpaceList);
	SpaceShaders.UnUse();

	// Draw the flames (maybe)
	// (implement this maybe???)

	// Draw the helper arrow (sometimes), yeah!
	if (gameState != GameEndState && cameraMode != AttachedView && fmod(RealTime, ARROW_FLICKER_PERIOD) <= ARROW_FLICKER_PERIOD_VISIBLE)
	{
		Tri tri;
		int gridX{ (int)floor(llPos.x / MOON_WIDTH * (float)MOON_RES) };
		int gridZ{ (int)floor(llPos.z / MOON_WIDTH * (float)MOON_RES) };
		// decide whether we try the lower-left (x-, z-) or upper-right (x+, z+) triangle
		if ((llPos.x - gridX * MOON_WIDTH / (float)MOON_RES)
			>= (llPos.z - gridZ * MOON_WIDTH / (float)MOON_RES)) // lower-left
		{
			tri.p1 = getMoonPos(gridX	 , gridZ	);
			tri.p2 = getMoonPos(gridX + 1, gridZ	);
			tri.p3 = getMoonPos(gridX	 , gridZ + 1);
		}
		else // upper-right
		{
			tri.p1 = getMoonPos(gridX + 1, gridZ	);
			tri.p2 = getMoonPos(gridX + 1, gridZ + 1);
			tri.p3 = getMoonPos(gridX	 , gridZ + 1);
		}

		// Calculate the position & normal
		glm::vec3 arrowPos(llPos);
		arrowPos.y = calcExactMoonHeightWorld(llPos.x, llPos.z);
		glm::vec3 normal{ tri.calcNormal() }, up{ 0.f,1.f,0.f };

		// Animate the drawn normal to make it look a little less jagged
		if (LastArrowNormal == ARROW_UNDEFINED_NORMAL)
			LastArrowNormal = normal;
		else
		{
			// do some linear interpolation
			float t = min(10.0f * LastTimeDelta, 1.f);
			glm::vec3 temp = normal * t + LastArrowNormal * (1.f - t);
			normal = glm::normalize(temp);
			LastArrowNormal = normal;
		}

		// Find the rotation for the vector to point upwards (axis and angle along it)
		glm::vec3 axis = glm::cross(up, normal);
		float angle = glm::acos(glm::dot(up, normal)) * RAD_2_DEG;

		glPushMatrix();
		glTranslatef(arrowPos.x, arrowPos.y, arrowPos.z);
		glRotatef((GLfloat)angle, (GLfloat)axis.x, (GLfloat)axis.y, (GLfloat)axis.z);
		glScalef(ARROW_SCALE, ARROW_SCALE, ARROW_SCALE);
		glColor3f(.05f, 1.f, .1f);
		glCallList(ArrowList);
		glPopMatrix();
	}

	// Draw the lunar module!
	glLoadIdentity();
	glLoadMatrixf(glm::value_ptr(glm::inverse(cameraTransformTrans * cameraTransformRot)));
	// draw explosion if you lost
	if (gameState == GameEndState && !DidWin)
	{
		glPushMatrix();
		glScalef((GLfloat)1.675f, (GLfloat)1.675f, (GLfloat)1.675f);
		glCallList(ExplosionList);
		glPopMatrix();
	}
	// otherwise just draw the ship normally
	else
	{
		LLShaders.Use();
		glCallList(ll.getDefaultModel());
		LLShaders.UnUse();
	}

	// Draw some text maybe depending on the game state!
	glDisable( GL_DEPTH_TEST );
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.f, 100.f, 0.f, 100.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	char buffer[256] = { '\0' };
	sprintf(buffer, "Time: %.2f", TimeElapsed);
	switch (gameState)
	{
		case PausedState:
			// Put the paused text in one corner
			glColor3f(0.6f, 0.7f, 1.f);
			DoRasterString(0.f, 95.f, 1.f, (char*)"Game Paused (tab to resume)");
			// Also put the time in the other corner
			// (but only if the game has started
			if (TimeElapsed > 0.f)
			{
				glColor3f(1.f, 1.f, 1.f);
				DoRasterString(80.f, 95.f, 1.f, buffer);
			}
			break;
		case GameEndState:
			glColor3f(1.f, 1.f, 1.f);
			DoRasterString(80.f, 95.f, 1.f, buffer);
			if (DidWin)
			{
				glColor3f(0.15f, 1.0f, 0.225f);
				DoRasterString(30.f, 65.f, 1.f, (char*)"You Landed! (tab to reset)");
				// Also put the high score in the other corner!
				if (TimeElapsed <= RecordTime)
				{
					glColor3f(0.25f, 1.f, 1.f);
					sprintf(buffer, "Record Time (New High!): %.2f", TimeElapsed);
				}
				else
				{
					glColor3f(1.f, .5f, .5f);
					sprintf(buffer, "Record Time: %.2f", RecordTime);
				}
				DoRasterString(1.5f, 95.f, 1.f, buffer);
			}
			else
			{
				glColor3f(1.f, 0.3f, 0.2f);
				DoRasterString(30.f, 65.f, 1.f, (char*)"You Crashed! (tab to reset)");
				// Draw the high score. To taunt the player, mostly
				if (RecordTime >= 0.f)
				{
					glColor3f(1.f, .5f, .5f);
					sprintf(buffer, "Record Time: %.2f", RecordTime);
					DoRasterString(1.5f, 95.f, 1.f, buffer);
				}
			}
			break;
		default:
			// Just put the current time up in the corner
			glColor3f(1.f, 1.f, 1.f);
			DoRasterString(80.f, 95.f, 1.f, buffer);
			break;
	}

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}

void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	glutSetWindow( MainWindow );

	// Create some lists!!!

	// Create the sky/space-box/sphere!!!
	SpaceList = glGenLists(1);
	glNewList(SpaceList, GL_COMPILE);
	OsuSphere(MOON_WIDTH * 10.f, 48, 48);
	glEndList();

	// Load the flame!
	FlameList = glGenLists(1);
	glNewList(FlameList, GL_COMPILE);
	//LoadObjFile(FLAME_OBJ_PATH);
	OsuSphere(.15f, 8, 8);
	glEndList();

	// Load the lunar lander!
	LLList = glGenLists(1);
	glNewList(LLList, GL_COMPILE);
	LoadObjFile(LL_OBJ_PATH);
	//OsuSphere(.5f, 16, 16); // (test object)
	glEndList();

	// Load the helper arrow!
	ArrowList = glGenLists(1);
	glNewList(ArrowList, GL_COMPILE);
	LoadObjFile(ARROW_OBJ_PATH);
	glEndList();

	// Load the explosion effect!
	ExplosionList = glGenLists(1);
	glNewList(ExplosionList, GL_COMPILE);
	LoadObjFile(EXPLOSION_OBJ_PATH);
	glEndList();

	// (don't create the moon because that's done
	//	on game state initialization)
	MoonList = glGenLists(1);
}


// initialize the glui window:

void
InitMenus()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow(MainWindow);

	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	glutAddMenuEntry("Reset", RESET);
	glutAddSubMenu("Debug", debugmenu);
	glutAddMenuEntry("Quit", QUIT);

	// attach the pop-up menu to the right mouse button:

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	// Sets input flags for the ones that have been pressed
	inputMap.insert_or_assign(c, true);
	fprintf(stdout, "+ key pressed: %c\n", c);
	
	//switch( c )
	//{
	//	case 'o':
	//	case 'O':
	//		NowProjection = ORTHO;
	//		break;

	//	case 'p':
	//	case 'P':
	//		NowProjection = PERSP;
	//		break;

	//	case 'q':
	//	case 'Q':
	//	case ESCAPE:
	//		DoMainMenu( QUIT );	// will not return here
	//		break;				// happy compiler

	//	default:
	//		fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	//}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}
// called when the mouse button transitions down or up:

void
MouseButton(int button, int state, int x, int y)
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion(int x, int y)
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset()
{
	ActiveButton = 0;
	DebugOn = 0;
}


// called when user resizes the window:

void
Resize(int width, int height)
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// handle a change to the window's visibility:

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}


float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
