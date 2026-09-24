#ifndef VERTEX_BUFFER_OBJECT_H
#define VERTEX_BUFFER_OBJECT_H

#ifdef WIN32
#include "windows.h"
#endif

//#include "glew.h"
#include <GL/gl.h>

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <map>

#define BUFFER_OFFSET(bytes)	( (GLubyte *)NULL + (bytes) )
#define ELEMENT_OFFSET(a1,a2)	(  BUFFER_OFFSET( (char *)(a2) - (char *)(a1) )  )

bool	IsExtensionSupported( const char * );
#ifdef NOTDEF
void    glGenBuffers( GLsizei, GLuint * );
void    glBindBuffer( GLenum, GLuint );
void    glBufferData( GLenum, GLsizeiptr, const void *, GLenum );
void *  glMapBuffer( GLenum, GLenum );
void    glUnmapBuffer( GLenum );
void    glDeleteBuffers( GLsizei, GLuint * );
void	glPrimitiveRestartIndex( GLuint );
#endif




struct Point
{
	float x, y, z;
	float nx, ny, nz;
	float r, g, b;
	float s, t;
};


class Key
{
    public:
		float x, y, z;

		Key( float _x, float _y, float _z )
		{
			x = _x;
			y = _y;
			z = _z;
		};

		Key( float _v[3] )
		{
			x = _v[0];
			y = _v[1];
			z = _v[2];
		};

		Key( const Key& k )
		{
			x = k.x;
			y = k.y;
			z = k.z;
		};
};

typedef std::map< Key, int >	PMap;



class VertexBufferObject
{
    private:
	bool				hasVertices, hasNormals, hasColors, hasTexCoords;
	float				c_r, c_g, c_b;
	float				c_nx, c_ny, c_nz;
	float				c_s, c_t;

	GLenum				topology;
	bool				verbose;
	bool				collapseCommonVertices;
	bool				isFirstDraw;
	bool				glBeginWasCalled;
	bool				drawWasCalled;
	bool				restartFound;

	std::vector <struct Point>	PointVec;
	PMap				PointMap;
	std::vector <GLuint>		ElementVec;
	struct Point *			parray;
	GLuint *			earray;
	GLuint				pbuffer;
	GLuint				ebuffer;

	const static GLuint RESTART_INDEX = ~0;	// 0xffffffff
	const static int TWO_VALUES   = 2;
	const static int THREE_VALUES = 3;

	GLuint AddVertex( GLfloat, GLfloat, GLfloat );
	void Reset( );

    public:
	void CollapseCommonVertices( bool );
	void Draw( );
	void DrawInstanced( int );
	void glBegin( GLenum );
	void glColor3f( GLfloat, GLfloat, GLfloat );
	void glColor3fv( GLfloat * );
	void glEnd( );
	void glNormal3f( GLfloat, GLfloat, GLfloat );
	void glNormal3fv( GLfloat * );
	void glTexCoord2f( GLfloat, GLfloat );
	void glTexCoord2fv( GLfloat * );
	void glVertex3f( GLfloat, GLfloat, GLfloat );
	void glVertex3fv( GLfloat * );
	void Init( );
	void Print( char * = (char *)"", FILE * = stderr );
	void RestartPrimitive( );
	void SetVerbose( bool );

	VertexBufferObject( )
	{
		// everything that could have gone here is in the Init( ) method
	};

	~VertexBufferObject( )
	{
		// really should be sure all dynamic arrays are deleted and all buffers are destroyed:
		// (it's possible to cause memory leaks in both cpu and gpu memory)

		Reset( );
	};
};
#endif
