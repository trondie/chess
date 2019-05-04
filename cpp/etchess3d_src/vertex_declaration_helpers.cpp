#include <stdio.h>
#include <stdlib.h>
#include "vertex_declaration_helpers.h"
#include "opengl_helpers.h"
#include "TestGLSL.h"
#include "geometry_helpers.h"

extern const char *semantic_name[15] = {
	"POSITION",
	"COLOR",
	"NORMAL",
	"TANGENT",
	"BINORMAL",
	"TEXCOORD0",
	"TEXCOORD1",
	"TEXCOORD2",
	"TEXCOORD3",
	"TEXCOORD4",
	"TEXCOORD5",
	"TEXCOORD6",
	"TEXCOORD7",
	"TEXCOORD8",
	"TEXCOORD9"
};


void freeVertexDeclaration( MyVertexDeclaration *avdecl )
{
	GLuint vbos[2];
	free( avdecl->velems );

	vbos[0] = avdecl->vbo;
	vbos[1] = avdecl->ibo;
	glDeleteBuffers( 2, vbos );   
}

void setVertexDeclaration( MyVertexDeclaration *avdecl, MyShader *shader )
{
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, avdecl->ibo ); 
	CHECK_GL_ERROR;
	for ( int i = 0; i < avdecl->nvelems; i++ )
	{
		GLint attrib = glGetAttribLocation( shader->prog, semantic_name[ avdecl->velems[i].semantic ] );
		CHECK_GL_ERROR;
		
		if ( attrib == -1 ) 
		{
			continue;
		}
		glBindBuffer( GL_ARRAY_BUFFER, avdecl->vbo );
		CHECK_GL_ERROR;
		glEnableVertexAttribArray( attrib );
		CHECK_GL_ERROR;
		glVertexAttribPointer( attrib, avdecl->velems[i].components, avdecl->velems[i].type, GL_FALSE, avdecl->elem_size, (void*)avdecl->velems[i].offset );
		CHECK_GL_ERROR;
	}
}

void unbindDeclaration( MyVertexDeclaration *avdecl, MyShader *shader )
{
	for ( int i = 0; i < avdecl->nvelems; i++ )
	{
		GLint attrib = glGetAttribLocation( shader->prog, semantic_name[ avdecl->velems[i].semantic ] );
		CHECK_GL_ERROR;
		
		if ( attrib == -1 ) continue;
		glDisableVertexAttribArray( attrib );
		CHECK_GL_ERROR;
	}

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); 
	CHECK_GL_ERROR;
}


void setBuffer( MyVertexDeclaration *avdecl )
{
	glBindBuffer( GL_ARRAY_BUFFER, avdecl->vbo );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, avdecl->ibo );
}


void createQuadPositionTexcoordVertexDecl( MyVertexDeclaration *avdecl, 
				 float left, float right, float top, float bottom, 
				 float uvleft, float uvtop, float uvright, float uvbottom )
{
	GLuint vbos[2];
	vertextexdata vdata[4];
	unsigned short idata[6];

	MyVertexElement *elements = (MyVertexElement*)malloc( sizeof(MyVertexElement)*2 );
	elements[0].components = 3;
	elements[0].offset = 0;
	elements[0].semantic = SPOSITION;
	elements[0].stream = 0;
	elements[0].type = GL_FLOAT;
	elements[1].components = 2;
	elements[1].offset = 3*sizeof(GLfloat);
	elements[1].semantic = STEXCOORD0;
	elements[1].stream = 0;
	elements[1].type = GL_FLOAT;

	avdecl->velems = elements;
	avdecl->nvelems = 2;
	avdecl->nindices = 6;
	avdecl->elem_size = 0;
	for ( int i = 0; i < 2; i++ ) avdecl->elem_size += elements[i].components*sizeof(elements[i].type);

	vdata[0] = vertextexdata( left, bottom, 0, uvleft, uvbottom );
	vdata[1] = vertextexdata( right, bottom, 0, uvright, uvbottom );
	vdata[2] = vertextexdata( right, top, 0, uvright, uvtop );
	vdata[3] = vertextexdata( left, top, 0, uvleft, uvtop );

	idata[0] = 0;
	idata[1] = 1;
	idata[2] = 3;
	idata[3] = 1;
	idata[4] = 2;
	idata[5] = 3;

	glGenBuffers( 2, vbos );   
	avdecl->vbo = vbos[0];
	avdecl->ibo = vbos[1];

	glBindBuffer( GL_ARRAY_BUFFER, avdecl->vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vdata), vdata, GL_DYNAMIC_DRAW );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, avdecl->ibo );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(idata), idata, GL_DYNAMIC_DRAW );
}

void createSpherePositionNormalVertexDecl( MyVertexDeclaration *avdecl, int latitudes, int longtitudes, float size )
{
	GLuint vbos[2];
	unsigned short idata[6];

	MyVertexElement *elements = (MyVertexElement*)malloc( sizeof(MyVertexElement)*2 );
	elements[0].components = 3;
	elements[0].offset = 0;
	elements[0].semantic = SPOSITION;
	elements[0].stream = 0;
	elements[0].type = GL_FLOAT;
	elements[1].components = 3;
	elements[1].offset = 12;
	elements[1].semantic = SNORMAL;
	elements[1].stream = 0;
	elements[1].type = GL_FLOAT;

	avdecl->velems = elements;
	avdecl->nvelems = 2;
	avdecl->elem_size = 0;
	for ( int i = 0; i < 2; i++ ) avdecl->elem_size += elements[i].components*sizeof(elements[i].type);

	createSphereGeometry( &avdecl->vbo, &avdecl->ibo, &avdecl->nindices, latitudes, longtitudes, size );

}

void createHalfDomePositionTexcoordVertexDecl( MyVertexDeclaration *avdecl, int latitudes, int longtitudes, float size )
{
	GLuint vbos[2];
	unsigned short idata[6];

	MyVertexElement *elements = (MyVertexElement*)malloc( sizeof(MyVertexElement)*2 );
	elements[0].components = 3;
	elements[0].offset = 0;
	elements[0].semantic = SPOSITION;
	elements[0].stream = 0;
	elements[0].type = GL_FLOAT;
	elements[1].components = 2;
	elements[1].offset = 12;
	elements[1].semantic = STEXCOORD0;
	elements[1].stream = 0;
	elements[1].type = GL_FLOAT;

	avdecl->velems = elements;
	avdecl->nvelems = 2;
	avdecl->elem_size = 0;
	for ( int i = 0; i < 2; i++ ) avdecl->elem_size += elements[i].components*sizeof(elements[i].type);

	createHalfDomeGeometry( &avdecl->vbo, &avdecl->ibo, &avdecl->nindices, latitudes, longtitudes, size );

}

void createBoxNormalPositionVertexDecl( MyVertexDeclaration *avdecl, float size )
{
	GLuint vbos[2];
	unsigned short idata[6];

	MyVertexElement *elements = (MyVertexElement*)malloc( sizeof(MyVertexElement)*2 );
	elements[0].components = 3;
	elements[0].offset = 0;
	elements[0].semantic = SNORMAL;
	elements[0].stream = 0;
	elements[0].type = GL_FLOAT;
	elements[1].components = 3;
	elements[1].offset = 12;
	elements[1].semantic = SPOSITION;
	elements[1].stream = 0;
	elements[1].type = GL_FLOAT;

	avdecl->velems = elements;
	avdecl->nvelems = 2;
	avdecl->elem_size = 0;
	for ( int i = 0; i < 2; i++ ) avdecl->elem_size += elements[i].components*sizeof(elements[i].type);

	createCubeGeometry( &avdecl->vbo, &avdecl->ibo, &avdecl->nindices, size );
}
