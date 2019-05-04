#ifndef _OPENGL_HELPERS_H_
#define _OPENGL_HELPERS_H_

#if defined( PLATFORM_ARM_ANDROID )
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <windows.h>
#include <gl/gl.h>
#include <gl/glext.h>
#endif
#include <time.h>
#include "TestGLSL.h"

//Helper macro prints line and function of gl error
#ifndef CHECK_GL_ERROR
#ifdef NO_DEBUG
#define CHECK_GL_ERROR
#else
#define CHECK_GL_ERROR \
	do { \
		GLenum error = glGetError(); \
		if ( error != GL_NO_ERROR ) \
		{ \
			Base::log( "%s:%d glError() = 0x%x\n", __FUNCTION__, __LINE__, error ); \
			break_and_exit(); \
		} \
	} while( 0 )
#endif
#endif

//Helper macro for vertex attrib pointers
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
bool gl_supports(const char *extension);
/** Helper function for getting GL version */
float get_gl_version();
/** Helper function for GL error handling */
void break_and_exit();
/** Gets program run time */
float get_run_time();
/** Gets all shader extension functions.. messy stuff */
void init_shader_functions();
/* Used for reading a textfile, i.e. our GLSL shaders */
char *textFileRead(char *fn);
/* Used for reading a fragment plus a shader textfile, i.e. our GLSL shaders */
char *textFileReadFragment(char *fragment, char *shader);
int textFileWrite(char *fn, char *s);
/** Prints out shader info log. Shader compilation debugging */
void printShaderInfoLog(GLuint obj);
/** Prints out program info log. Program linkage debugging */
void printProgramInfoLog(GLuint obj);

#if defined( PLATFORM_WINDOWS )
//Next follows heaps of function pointers used for shader extensions. Nevermind this.
typedef void (APIENTRY * pfn_glBindBuffer) (GLenum target, GLuint buffer);
typedef void (APIENTRY * pfn_glDeleteBuffers) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * pfn_glGenBuffers) (GLsizei n, GLuint *buffers);
typedef GLboolean (APIENTRY * pfn_glIsBuffer)(GLuint GLbuffer);
typedef void (APIENTRY * pfn_glBufferData) (GLenum target, GLsizei size, const GLvoid *data, GLenum usage);
typedef void (APIENTRY * pfn_glBufferSubData) (GLenum target, GLint offset, GLsizei size, const GLvoid *data);
typedef void (APIENTRY * pfn_glGetBufferSubData) (GLenum target, GLint offset, GLsizei size, GLvoid *data);
typedef void* (APIENTRY * pfn_glMapBuffer) (GLenum target, GLenum access);
typedef GLboolean (APIENTRY * pfn_glUnmapBuffer) (GLenum target);
typedef void (APIENTRY * pfn_glGetBufferParameteriv) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRY * pfn_glGetBufferPointerv) (GLenum target, GLenum pname, GLvoid **params);
typedef void (APIENTRY * pfn_glActiveTexture) (GLenum texture);
typedef void (APIENTRY * pfn_glGetProgramiv) (GLuint program, GLenum pname, GLint *params);
typedef void (APIENTRY * pfn_glIsProgram) (GLuint program);
typedef void (APIENTRY * pfn_glVertexAttrib1f)(GLuint index, GLfloat v0);
typedef void (APIENTRY * pfn_glVertexAttrib2f)(GLuint index, GLfloat v0, GLfloat v1);
typedef void (APIENTRY * pfn_glVertexAttrib3f)(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY * pfn_glVertexAttrib4f)(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY * pfn_glVertexAttrib1fv)(GLuint index, const GLfloat *v);
typedef void (APIENTRY * pfn_glVertexAttrib2fv)(GLuint index, const GLfloat *v);
typedef void (APIENTRY * pfn_glVertexAttrib3fv)(GLuint index, const GLfloat *v);
typedef void (APIENTRY * pfn_glVertexAttrib4fv)(GLuint index, const GLfloat *v);
typedef void (APIENTRY * pfn_glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void (APIENTRY * pfn_glEnableVertexAttribArray)(GLuint index);
typedef void (APIENTRY * pfn_glDisableVertexAttribArray)(GLuint index);
typedef void (APIENTRY * pfn_glBindAttribLocation)(GLuint programObj, GLuint index, const GLuint *name);
typedef void (APIENTRY * pfn_glGetActiveAttrib)(GLuint programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLuint *name);
typedef GLint (APIENTRY * pfn_glGetAttribLocation)(GLuint programObj, const char* name);
typedef void (APIENTRY * pfn_glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params);
typedef void (APIENTRY * pfn_glGetVertexAttribiv)(GLuint index, GLenum pname, GLint *params);
typedef void (APIENTRY * pfn_glGetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid **pointer);
typedef void (APIENTRY * pfn_glUniform1i) (GLint location, GLint x);
typedef void (APIENTRY * pfn_glUniform2i) (GLint location, GLint x, GLint y);
typedef void (APIENTRY * pfn_glUniform3i) (GLint location, GLint x, GLint y, GLint z);
typedef void (APIENTRY * pfn_glUniform4i) (GLint location, GLint x, GLint y, GLint z, GLint w);
typedef void (APIENTRY * pfn_glUniform1f) (GLint location, GLfloat x);
typedef void (APIENTRY * pfn_glUniform2f) (GLint location, GLfloat x, GLfloat y);
typedef void (APIENTRY * pfn_glUniform3f) (GLint location, GLfloat x, GLfloat y, GLfloat z);
typedef void (APIENTRY * pfn_glUniform4f) (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void (APIENTRY * pfn_glUniform1iv) (GLint location, GLsizei count, const GLint *v);
typedef void (APIENTRY * pfn_glUniform2iv) (GLint location, GLsizei count, const GLint *v);
typedef void (APIENTRY * pfn_glUniform3iv) (GLint location, GLsizei count, const GLint *v);
typedef void (APIENTRY * pfn_glUniform4iv) (GLint location, GLsizei count, const GLint *v);
typedef void (APIENTRY * pfn_glUniform1fv) (GLint location, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * pfn_glUniform2fv) (GLint location, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * pfn_glUniform3fv) (GLint location, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * pfn_glUniform4fv) (GLint location, GLsizei count, const GLfloat *v);
typedef void (APIENTRY * pfn_glUniformMatrix2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * pfn_glUniformMatrix3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * pfn_glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRY * pfn_glGetUniformfv) (GLuint program, GLint location, GLfloat *params);
typedef void (APIENTRY * pfn_glGetUniformiv) (GLuint program, GLint location, GLint *params);
typedef int (APIENTRY * pfn_glGetUniformLocation) (GLuint program, const char *name);
typedef void (APIENTRY * pfn_glGetActiveUniform) (GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, char *name);
typedef void (APIENTRY * pfn_glLinkProgram) (GLuint program);
typedef void (APIENTRY * pfn_glValidateProgram) (GLuint program);
typedef void (APIENTRY * pfn_glGetShaderSource) (GLuint shader, GLsizei bufsize, GLsizei *length, char *source);
typedef void (APIENTRY * pfn_glShaderSource) (GLuint shader, GLsizei count, const char **string, const GLint *length);
typedef void (APIENTRY * pfn_glCompileShader) (GLuint shader);
typedef void (APIENTRY * pfn_glUseProgram) (GLuint program);
typedef GLuint (APIENTRY * pfn_glCreateProgram) (void);
typedef GLuint (APIENTRY * pfn_glCreateShader) (GLenum type);
typedef void (APIENTRY * pfn_glAttachShader) (GLuint program, GLuint shader);
typedef void (APIENTRY * pfn_glDeleteProgram) (GLuint program);
typedef void (APIENTRY * pfn_glDeleteShader) (GLuint shader);
typedef void (APIENTRY * pfn_glDetachShader) (GLuint program, GLuint shader);
typedef void (APIENTRY * pfn_glGetAttachedShaders) (GLuint program, GLsizei maxcount, GLsizei *count, GLuint *shaders);
typedef void (APIENTRY * pfn_glGetProgramInfoLog) (GLuint program, GLsizei bufsize, GLsizei *length, char *infolog);
typedef GLboolean (APIENTRY * pfn_glIsShader) (GLuint shader);
typedef GLboolean (APIENTRY * pfn_glGetShaderiv) (GLuint shader, GLenum pname, GLint *params);
typedef GLboolean (APIENTRY * pfn_glGetShaderInfoLog) (GLuint shader, GLsizei bufsize, GLsizei *length, char *infolog);

extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffers;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2D;
extern PFNGLDRAWBUFFERSPROC glDrawBuffers;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffers;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatus;
extern PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbuffer;
extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;

extern pfn_glGetBufferParameteriv glGetBufferParameteriv;
extern pfn_glGetBufferPointerv glGetBufferPointerv;
extern pfn_glBindBuffer glBindBuffer;
extern pfn_glDeleteBuffers glDeleteBuffers;
extern pfn_glGenBuffers glGenBuffers;
extern pfn_glIsBuffer glIsBuffer;
extern pfn_glBufferData glBufferData;
extern pfn_glBufferSubData glBufferSubData;
extern pfn_glGetBufferSubData glGetBufferSubData;
extern pfn_glUnmapBuffer glUnmapBuffer;
extern pfn_glMapBuffer glMapBuffer;

extern pfn_glActiveTexture glActiveTexture;
extern pfn_glGetProgramiv glGetProgramiv;
extern pfn_glIsProgram glIsProgram;
extern pfn_glVertexAttrib1f glVertexAttrib1f;
extern pfn_glVertexAttrib2f glVertexAttrib2f;
extern pfn_glVertexAttrib3f glVertexAttrib3f;
extern pfn_glVertexAttrib4f glVertexAttrib4f;
extern pfn_glVertexAttrib1fv glVertexAttrib1fv;
extern pfn_glVertexAttrib2fv glVertexAttrib2fv;
extern pfn_glVertexAttrib3fv glVertexAttrib3fv;
extern pfn_glVertexAttrib4fv glVertexAttrib4fv;
extern pfn_glVertexAttribPointer glVertexAttribPointer;
extern pfn_glEnableVertexAttribArray glEnableVertexAttribArray;
extern pfn_glDisableVertexAttribArray glDisableVertexAttribArray;
extern pfn_glBindAttribLocation glBindAttribLocation;
extern pfn_glGetActiveAttrib glGetActiveAttrib;
extern pfn_glGetAttribLocation glGetAttribLocation;
extern pfn_glGetVertexAttribfv glGetVertexAttribfv;
extern pfn_glGetVertexAttribiv glGetVertexAttribiv;
extern pfn_glGetVertexAttribPointerv glGetVertexAttribPointerv;
extern pfn_glUniform1i glUniform1i;
extern pfn_glUniform2i glUniform2i;
extern pfn_glUniform3i glUniform3i;
extern pfn_glUniform4i glUniform4i;
extern pfn_glUniform1f glUniform1f;
extern pfn_glUniform2f glUniform2f;
extern pfn_glUniform3f glUniform3f;
extern pfn_glUniform4f glUniform4f;
extern pfn_glUniform1iv glUniform1iv;
extern pfn_glUniform2iv glUniform2iv;
extern pfn_glUniform3iv glUniform3iv;
extern pfn_glUniform4iv glUniform4iv;
extern pfn_glUniform1fv glUniform1fv;
extern pfn_glUniform2fv glUniform2fv;
extern pfn_glUniform3fv glUniform3fv;
extern pfn_glUniform4fv glUniform4fv;
extern pfn_glUniformMatrix2fv glUniformMatrix2fv;
extern pfn_glUniformMatrix3fv glUniformMatrix3fv;
extern pfn_glUniformMatrix4fv glUniformMatrix4fv;
extern pfn_glGetUniformfv glGetUniformfv;
extern pfn_glGetUniformiv glGetUniformiv;
extern pfn_glGetUniformLocation glGetUniformLocation;
extern pfn_glGetActiveUniform glGetActiveUniform;
extern pfn_glLinkProgram glLinkProgram;
extern pfn_glValidateProgram glValidateProgram;
extern pfn_glGetShaderSource glGetShaderSource;
extern pfn_glShaderSource glShaderSource;
extern pfn_glCompileShader glCompileShader;
extern pfn_glUseProgram glUseProgram;
extern pfn_glCreateProgram glCreateProgram;
extern pfn_glCreateShader glCreateShader;
extern pfn_glAttachShader glAttachShader;
extern pfn_glDeleteProgram glDeleteProgram;
extern pfn_glDeleteShader glDeleteShader;
extern pfn_glDetachShader glDetachShader;
extern pfn_glGetAttachedShaders glGetAttachedShaders;
extern pfn_glGetProgramInfoLog glGetProgramInfoLog;
extern pfn_glIsShader glIsShader;
extern pfn_glGetShaderiv glGetShaderiv;
extern pfn_glGetShaderInfoLog glGetShaderInfoLog;
#endif /* PLATFORM_WINDOWS */

#endif /* _OPENGL_HELPERS_H_ */
