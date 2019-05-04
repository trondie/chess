#ifndef _VERTEX_DECLARATION_HELPERS_H_
#define _VERTEX_DECLARATION_HELPERS_H_

#include "shader_helper.h"

typedef enum 
{
	SPOSITION,
	SCOLOR,
	SNORMAL,
	STANGENT,
	SBINORMAL,
	STEXCOORD0,
	STEXCOORD1,
	STEXCOORD2,
	STEXCOORD3,
	STEXCOORD4,
	STEXCOORD5,
	STEXCOORD6,
	STEXCOORD7,
	STEXCOORD8,
	STEXCOORD9
} SemanticNames;

extern const char *semantic_name[15];

typedef struct 
{
	GLenum type;
	GLuint components;
	SemanticNames semantic;
	GLint offset;
	GLuint stream;
} MyVertexElement;

typedef struct
{
	MyVertexElement *velems;
	GLuint nvelems;
	GLuint vbo;
	GLuint ibo;
	GLuint nindices;
	GLuint elem_size;
} MyVertexDeclaration;

typedef struct vertextexdata_struct
{
	GLfloat x,y,z;
	GLfloat u,v;
	vertextexdata_struct( GLfloat ax, GLfloat ay, GLfloat az, GLfloat au, GLfloat av ) {x = (ax); y = (ay); z = (az); u = au; v = av; };
	vertextexdata_struct(){};
} vertextexdata;

typedef struct vertexdata_struct
{
	GLfloat x,y,z;
	vertexdata_struct( GLfloat ax, GLfloat ay, GLfloat az ) {x = (ax); y = (ay); z = (az); };
	vertexdata_struct(){};
} vertexdata;


void freeVertexDeclaration( MyVertexDeclaration *avdecl );

void setVertexDeclaration( MyVertexDeclaration *avdecl, MyShader *shader );
void unbindDeclaration( MyVertexDeclaration *avdecl, MyShader *shader );

void setBuffer( MyVertexDeclaration *avdecl );


// Functions that creates geometries and wraps them in Vertex Declarations 
void createBoxNormalPositionVertexDecl( MyVertexDeclaration *avdecl, float size );
void createHalfDomePositionTexcoordVertexDecl( MyVertexDeclaration *avdecl, int latitudes, int longtitudes, float size );
void createSpherePositionNormalVertexDecl( MyVertexDeclaration *avdecl, int latitudes, int longtitudes, float size );
void createQuadPositionTexcoordVertexDecl( MyVertexDeclaration *avdecl, 
				 float left, float right, float top, float bottom, 
				 float uvleft, float uvtop, float uvright, float uvbottom );

#endif /* _VERTEX_DECLARATION_HELPERS_H_ */
