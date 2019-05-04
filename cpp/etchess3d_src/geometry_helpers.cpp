#include <stdio.h>
#include <stdlib.h>
#include "geometry_helpers.h"
#include "math_helpers.h"
#include "mymath.h"
#include "opengl_helpers.h"
#include <lib3ds.h>

void createCubeGeometry( GLuint *cube_vbo, GLuint *cube_ibo, GLuint *cube_indices, float size )
{
	GLuint vbos[2];
	//Vertices
	myvec3 v[8] = { 
		myvec3( size/2, size/2, size/2 ),
		myvec3( -size/2, size/2, size/2 ),
		myvec3( -size/2, -size/2, size/2 ),
		myvec3( size/2, -size/2, size/2 ),
		myvec3( size/2, -size/2, -size/2 ),
		myvec3( size/2, size/2, -size/2 ),
		myvec3( -size/2, size/2, -size/2 ),
		myvec3( -size/2, -size/2, -size/2 )
	};
	//Face normals
	myvec3 n[6] = {
		myvec3( 0, 0, 1 ),
		myvec3( 1, 0, 0 ),
		myvec3( 0, 1, 0 ),
		myvec3( -1, 0, 0 ),
		myvec3( 0, -1, 0 ),
		myvec3( 0, 0, -1 )
	};

	//Interleave face normals with vertices. 
	//One face consists of two triangles, i.e. face0 consists of v0-v1-v2 and v2-v3-v0
	myvec3 normals_vertices_interleaved[36*2] =  {
		// n0-n1-n2, v0-v1-v2, n2-n3-n0, v2-v3-v0
		n[0], 		v[0], 
		n[0], 		v[1], 
		n[0], 		v[2], 
		n[0], 		v[2], 
		n[0], 		v[3], 
		n[0], 		v[0],        
		// n0-n3-n4, v0-v3-v4, n4-n5-n0, v4-v5-v0
		n[1], 		v[0], 
		n[1], 		v[3], 
		n[1], 		v[4], 
		n[1], 		v[4], 
		n[1], 		v[5], 
		n[1], 		v[0],         
		// n0-n5-n6, v0-v5-v6, n6-n1-n0, v6,v1,v0
		n[2], 		v[0], 
		n[2], 		v[5], 
		n[2], 		v[6], 
		n[2], 		v[6], 
		n[2], 		v[1], 
		n[2], 		v[0],    
		// n1-n6-n7, v1-v6-v7, n7,n2,n1, v7,v2,v1
		n[3],		v[1],
		n[3],		v[6], 
		n[3],		v[7], 
		n[3],		v[7], 
		n[3],		v[2], 
		n[3],		v[1],   
		// n7-n4-n3, v7-v4-v3, n3-n2-n7, v3-v2-v7
		n[4],		v[7], 
		n[4],		v[4], 
		n[4],		v[3], 
		n[4],		v[3], 
		n[4],		v[2], 
		n[4],		v[7], 
		// n4-n7-n6, v4-v7-v6, n6-n5-n4, v6-v5-v4
		n[5],		v[4], 
		n[5],		v[7], 
		n[5],		v[6], 
		n[5],		v[6], 
		n[5],		v[5], 
		n[5],		v[4]   
	};  
	GLushort indices[36];
	GLuint vns = 36*2*sizeof(myvec3);
	GLuint is = 36*sizeof(GLushort);

	for( unsigned int i = 0; i < 36; i++ ) indices[i] = i;

	//Generate vbo and ibo IDs. They are initially empty
	glGenBuffers( 2, vbos );   
	*cube_vbo = vbos[0];
	*cube_ibo = vbos[1];
	*cube_indices = 6*6;

	//Then bind the vbo and ibo and store the vertex data and index data in OpenGL
	glBindBuffer( GL_ARRAY_BUFFER, *cube_vbo );
	glBufferData( GL_ARRAY_BUFFER, vns, normals_vertices_interleaved, GL_DYNAMIC_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, *cube_ibo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, is, indices, GL_DYNAMIC_DRAW );

	CHECK_GL_ERROR;
}


void createHalfDomeGeometry( GLuint *dome_vbo, GLuint *dome_ibo, GLuint *dome_indices, int latitudes, int longtitudes, float r )
{
	GLfloat *vertices_and_texcoords;
	GLushort *indices;
	GLuint vbos[2];
	int theta, phy, index, iindex;

	//Generate vbo and ibo which are initially empty.
	glGenBuffers( 2, vbos );   
	*dome_vbo = vbos[0];
	*dome_ibo = vbos[1];
	*dome_indices = (latitudes+1)*(longtitudes+1)*6;

	//Generate the sphere geometry
	vertices_and_texcoords = new GLfloat[ *dome_indices ];
	indices = new GLushort[ *dome_indices ];
	index = 0;
	iindex = 0;
	float nx, ny, nz, u, v;

	for ( phy = 0; phy <= longtitudes; phy++ )
	{
		for ( theta = 0; theta <= latitudes; theta++ )
		{
			//Normals calculated from spherical vector
			nx = sin( (theta*M_PI)/(longtitudes/2.0) ) * cos( (phy*M_PI)/(latitudes/2.0) );
			ny = cos( (theta*M_PI)/(longtitudes/2.0) ); 
			nz = -sin( (theta*M_PI)/(longtitudes/2.0) ) * sin( (phy*M_PI)/(latitudes/2.0) );

			myvec3 n = myvec3(nx,ny,nz).normalized();
			//Vertices are just an extrusion of spherical normals
			vertices_and_texcoords[ index + 0 ] = r*n.x;
			vertices_and_texcoords[ index + 1 ] = r*n.y;
			vertices_and_texcoords[ index + 2 ] = r*n.z;

			u = asin( nx )/M_PI + 0.5;
			v = asin( ny )/M_PI + 0.5;
			vertices_and_texcoords[ index + 3 ] = CLAMP( 0.f, 1.f, u );
			vertices_and_texcoords[ index + 4 ] = CLAMP( 0.f, 1.f, v );

			//indices forms two triangles [/]
			indices[ iindex + 0 ] = theta*(latitudes+1) + phy;
			indices[ iindex + 1 ] = theta*(latitudes+1) + (phy+1)%(longtitudes+1);
			indices[ iindex + 2 ] = ((theta+1)%(latitudes+1))*(latitudes+1) + (phy+1)%(longtitudes+1);
			indices[ iindex + 3 ] = ((theta+1)%(latitudes+1))*(latitudes+1) + (phy+1)%(longtitudes+1);
			indices[ iindex + 4 ] = ((theta+1)%(latitudes+1))*(latitudes+1) + phy;
			indices[ iindex + 5 ] = theta*(latitudes+1) + phy;

			index += 5;
			iindex += 6;
		}
	}

	GLuint vns = index*sizeof(GLfloat);
	GLuint is = iindex*sizeof(GLushort);

	//Now bind vbo and ibo, then store the sphere vertex data and index data in OpenGL
	glBindBuffer( GL_ARRAY_BUFFER, *dome_vbo );
	glBufferData( GL_ARRAY_BUFFER, vns, vertices_and_texcoords, GL_DYNAMIC_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, *dome_ibo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, is, indices, GL_DYNAMIC_DRAW );

	//Data is in OpenGL so we may safeley delete local data.
	delete [] vertices_and_texcoords;
	delete [] indices;

	CHECK_GL_ERROR;
}

void createSphereGeometry( GLuint *sphere_vbo, GLuint *sphere_ibo, GLuint *sphere_indices, int latitudes, int longtitudes, float r )
{
	GLfloat *vertices_and_normals;
	GLushort *indices;
	GLuint vbos[2];
	int theta, phy, index;

	//Generate vbo and ibo which are initially empty.
	glGenBuffers( 2, vbos );   
	*sphere_vbo = vbos[0];
	*sphere_ibo = vbos[1];
	*sphere_indices = (latitudes+1)*(longtitudes+1)*6;

	//Generate the sphere geometry
	vertices_and_normals = new GLfloat[ *sphere_indices ];
	indices = new GLushort[ *sphere_indices ];
	index = 0;

	for ( theta = 0; theta <= latitudes; theta++ )
	{
		for ( phy = 0; phy <= longtitudes; phy++ )
		{
			//Normals calculated from spherical vector
			vertices_and_normals[ index + 0 ] = sin( (theta*M_PI)/(longtitudes/2.0) ) * cos( (phy*M_PI)/(latitudes/2.0) );
			vertices_and_normals[ index + 1 ] = cos( (theta*M_PI)/(longtitudes/2.0) ); 
			vertices_and_normals[ index + 2 ] = -sin( (theta*M_PI)/(longtitudes/2.0) ) * sin( (phy*M_PI)/(latitudes/2.0) );

			//Vertices are just an extrusion of spherical normals
			vertices_and_normals[ index + 3 ] = r*vertices_and_normals[ index + 0 ];
			vertices_and_normals[ index + 4 ] = r*vertices_and_normals[ index + 1 ];
			vertices_and_normals[ index + 5 ] = r*vertices_and_normals[ index + 2 ];

			//indices forms two triangles [/]
			indices[ index + 0 ] = theta*(latitudes+1) + phy;
			indices[ index + 1 ] = theta*(latitudes+1) + (phy+1)%(longtitudes+1);
			indices[ index + 2 ] = ((theta+1)%(latitudes+1))*(latitudes+1) + (phy+1)%(longtitudes+1);
			indices[ index + 3 ] = ((theta+1)%(latitudes+1))*(latitudes+1) + (phy+1)%(longtitudes+1);
			indices[ index + 4 ] = ((theta+1)%(latitudes+1))*(latitudes+1) + phy;
			indices[ index + 5 ] = theta*(latitudes+1) + phy;

			index += 6;
		}
	}

	GLuint vns = *sphere_indices*sizeof(GLfloat);
	GLuint is = *sphere_indices*sizeof(GLushort);

	//Now bind vbo and ibo, then store the sphere vertex data and index data in OpenGL
	glBindBuffer( GL_ARRAY_BUFFER, *sphere_vbo );
	glBufferData( GL_ARRAY_BUFFER, vns, vertices_and_normals, GL_DYNAMIC_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, *sphere_ibo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, is, indices, GL_DYNAMIC_DRAW );

	//Data is in OpenGL so we may safeley delete local data.
	delete [] vertices_and_normals;
	delete [] indices;

	CHECK_GL_ERROR;
}

void destroyGeometry( GLuint object_vbo, GLuint object_ibo )
{
	//Deletes the vbo and ibo so OpenGL may release vertex and index data
	GLuint vbos[2];
	vbos[0] = object_vbo;
	vbos[1] = object_ibo;
	glDeleteBuffers( 2, vbos );

	CHECK_GL_ERROR;
}
