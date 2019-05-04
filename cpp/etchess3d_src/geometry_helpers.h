#ifndef _GEOMETRY_HELPERS_H_
#define _GEOMETRY_HELPERS_H_

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <gl/gl.h>
#include <gl/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif
#include "vertex_declaration_helpers.h"

// Geometry functions that will generate IDs for OpenGL vertex buffer object (vbo) and an index buffer object (ibo)
// The vbos and ibos are simply vertex arrays and index arrays stored in OpenGL and referenced via GLuints.
void createCubeGeometry( GLuint *cube_vbo, GLuint *cube_ibo, GLuint *cube_indices, float size );
void createHalfDomeGeometry( GLuint *dome_vbo, GLuint *dome_ibo, GLuint *dome_indices, int latitudes, int longtitudes, float radius );
void createSphereGeometry( GLuint *sphere_vbo, GLuint *sphere_ibo, GLuint *sphere_indices, int latitudes, int longtitudes, float radius );
void destroyGeometry( GLuint object_vbo, GLuint object_ibo );

#endif /*  _GEOMETRY_HELPERS_H_ */
