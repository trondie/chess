#include <stdio.h>	
#include <stdlib.h>	
#include <string.h>	
#include "opengl_helpers.h"

char *textFileReadFragment(char *fragment, char *shader) {
	FILE *fp_fragment = NULL, *fp_shader = NULL;
	char *content = NULL;
	int count_shader=0, count_fragment=0;

	if (shader != NULL) {
		fp_shader = fopen(shader,"rt");
	}
	if (fragment != NULL) {
		fp_fragment = fopen(fragment,"rt");
	}

	if (fp_shader != NULL) {  
        fseek(fp_shader, 0, SEEK_END);
        count_shader = ftell(fp_shader);
        rewind(fp_shader);
	}
	else {
		Base::log( "Could not open file %s", shader );
	}

	if (fp_fragment != NULL) {  
        fseek(fp_fragment, 0, SEEK_END);
        count_fragment = ftell(fp_fragment);
        rewind(fp_fragment);
	}
	else {
		Base::log( "Could not open file %s", fragment );
	}

	if (count_shader + count_fragment > 0) {
		content = (char *)malloc(sizeof(char) * (count_shader+count_fragment+1));
		count_fragment = fread(content,sizeof(char),count_fragment,fp_fragment);
		count_shader = fread(&content[count_fragment],sizeof(char),count_shader,fp_shader);
		content[count_fragment+count_shader] = '\0';
	}
	
	if (fp_fragment != NULL) {
		fclose(fp_fragment);
	}
	if (fp_shader != NULL) {
		fclose(fp_fragment);
	}

//	Base::log( "Read %s", content );
	return content;
}

char *textFileRead(char *fn) 
{
	FILE *fp;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		
		fp = fopen(fn,"rt");

		if (fp != NULL) {
      
        		fseek(fp, 0, SEEK_END);
        		count = ftell(fp);
        		rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		
		}
		else
		{
			Base::log( "Could not open file %s", fn );
		}
	}

//	Base::log( "Read %s", content );
	return content;
}

int textFileWrite(char *fn, char *s) 
{
	FILE *fp;
	int status = 0;

	if (fn != NULL) {
		fp = fopen(fn,"w");

		if (fp != NULL) {
			
			if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
				status = 1;
			fclose(fp);
		}
	}
	return(status);
}

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		Base::log("printShaderInfoLog: %s\n",infoLog);
        free(infoLog);
	}else{
		Base::log("Shader Info Log: OK\n");
	}
}

void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		Base::log("printProgramInfoLog: %s\n",infoLog);
        free(infoLog);
    }else{
		Base::log("Program Info Log: OK\n");
	}
}

float get_run_time()
{
	static unsigned int start_clock = 0;
	if( 0 == start_clock ) start_clock = clock();
	return (clock() - start_clock)/float(CLOCKS_PER_SEC);
}

float get_gl_version()
{
	const GLubyte * str = glGetString( GL_VERSION );
	char version[10];
	memcpy( version, str, 10 );
	*strrchr( version, '.' ) = 0;

	CHECK_GL_ERROR;

	return atof( version );
}


bool gl_supports( const char *extension )
{
	char *search = " ";
	char *token = NULL;
	int len = 0;
	const char *gl_extensions = (const char *)glGetString(GL_EXTENSIONS);
	char *tokens;
	tokens = (char*)malloc(strlen( gl_extensions ) + 1);
	strcpy( tokens, gl_extensions );

	token = strtok(tokens, search);
	Base::log("GL_EXTENSIONS string: %s\n", gl_extensions );
	while (NULL != token)
	{
		len = strlen( token );
		Base::log("token: %s ", token );
		if ( 0 == strncmp( extension, token, len ) )
		{
			Base::log( "Using extension: %s\n",  extension );
			free( tokens );
			return true;
		}
		token = strtok( NULL, search );
	}
	Base::log( "GLES2 does not support %s\n", extension );
	free( tokens );
	return false;
}


void break_and_exit()
{
	//Put break here
	exit(0);
}

#if defined(PLATFORM_WINDOWS)

//Here's all the shader functions being initialized. Nevermind this.
pfn_glBindBuffer glBindBuffer = NULL;
pfn_glDeleteBuffers glDeleteBuffers = NULL;
pfn_glGenBuffers glGenBuffers = NULL;
pfn_glIsBuffer glIsBuffer = NULL;
pfn_glBufferData glBufferData = NULL;
pfn_glBufferSubData glBufferSubData = NULL;
pfn_glGetBufferSubData glGetBufferSubData = NULL;
pfn_glMapBuffer glMapBuffer = NULL;
pfn_glUnmapBuffer glUnmapBuffer = NULL;
pfn_glGetBufferParameteriv glGetBufferParameteriv = NULL;
pfn_glGetBufferPointerv glGetBufferPointerv = NULL;

pfn_glVertexAttrib1f glVertexAttrib1f = NULL;
pfn_glVertexAttrib2f glVertexAttrib2f = NULL;
pfn_glVertexAttrib3f glVertexAttrib3f = NULL;
pfn_glVertexAttrib4f glVertexAttrib4f = NULL;
pfn_glVertexAttrib1fv glVertexAttrib1fv = NULL;
pfn_glVertexAttrib2fv glVertexAttrib2fv = NULL;
pfn_glVertexAttrib3fv glVertexAttrib3fv = NULL;
pfn_glVertexAttrib4fv glVertexAttrib4fv = NULL;
pfn_glVertexAttribPointer glVertexAttribPointer = NULL;
pfn_glEnableVertexAttribArray glEnableVertexAttribArray = NULL;
pfn_glDisableVertexAttribArray glDisableVertexAttribArray = NULL;
pfn_glBindAttribLocation glBindAttribLocation = NULL;
pfn_glGetActiveAttrib glGetActiveAttrib = NULL;
pfn_glGetAttribLocation glGetAttribLocation = NULL;
pfn_glGetVertexAttribfv glGetVertexAttribfv = NULL;
pfn_glGetVertexAttribiv glGetVertexAttribiv = NULL;
pfn_glGetVertexAttribPointerv glGetVertexAttribPointerv = NULL;
pfn_glUniform1i glUniform1i = NULL;
pfn_glUniform2i glUniform2i = NULL;
pfn_glUniform3i glUniform3i = NULL;
pfn_glUniform4i glUniform4i = NULL;
pfn_glUniform1f glUniform1f = NULL;
pfn_glUniform2f glUniform2f = NULL;
pfn_glUniform3f glUniform3f = NULL;
pfn_glUniform4f glUniform4f = NULL;
pfn_glUniform1iv glUniform1iv = NULL;
pfn_glUniform2iv glUniform2iv = NULL;
pfn_glUniform3iv glUniform3iv = NULL;
pfn_glUniform4iv glUniform4iv = NULL;
pfn_glUniform1fv glUniform1fv = NULL;
pfn_glUniform2fv glUniform2fv = NULL;
pfn_glUniform3fv glUniform3fv = NULL;
pfn_glUniform4fv glUniform4fv = NULL;
pfn_glUniformMatrix2fv glUniformMatrix2fv = NULL;
pfn_glUniformMatrix3fv glUniformMatrix3fv = NULL;
pfn_glUniformMatrix4fv glUniformMatrix4fv = NULL;
pfn_glGetUniformfv glGetUniformfv = NULL;
pfn_glGetUniformiv glGetUniformiv = NULL;
pfn_glGetUniformLocation glGetUniformLocation = NULL;
pfn_glGetActiveUniform glGetActiveUniform = NULL;
pfn_glLinkProgram glLinkProgram = NULL;
pfn_glValidateProgram glValidateProgram = NULL;
pfn_glGetShaderSource glGetShaderSource = NULL;
pfn_glShaderSource glShaderSource = NULL;
pfn_glCompileShader glCompileShader = NULL;
pfn_glUseProgram glUseProgram = NULL;
pfn_glCreateProgram glCreateProgram = NULL;
pfn_glCreateShader glCreateShader = NULL;
pfn_glAttachShader glAttachShader = NULL;
pfn_glDeleteShader glDeleteShader = NULL;
pfn_glDetachShader glDetachShader = NULL;
pfn_glGetAttachedShaders glGetAttachedShaders = NULL;
pfn_glGetProgramInfoLog glGetProgramInfoLog = NULL;
pfn_glIsShader glIsShader = NULL;
pfn_glGetShaderiv glGetShaderiv = NULL;
pfn_glGetShaderInfoLog glGetShaderInfoLog = NULL;
pfn_glDeleteProgram glDeleteProgram = NULL;
pfn_glActiveTexture glActiveTexture = NULL;
pfn_glGetProgramiv glGetProgramiv = NULL;
pfn_glIsProgram glIsProgram = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffers;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2D;
PFNGLDRAWBUFFERSPROC glDrawBuffers;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffers;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatus;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbuffer;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebuffer;

#endif

void init_shader_functions()
{
#if defined( PLATFORM_WINDOWS )
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSEXTPROC) wglGetProcAddress("glGenFramebuffersEXT");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) wglGetProcAddress("glFramebufferTexture2DEXT");
	glDrawBuffers = (PFNGLDRAWBUFFERSPROC) wglGetProcAddress("glDrawBuffers");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSEXTPROC) wglGetProcAddress("glDeleteFramebuffersEXT");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) wglGetProcAddress("glCheckFramebufferStatusEXT");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFEREXTPROC) wglGetProcAddress("glBindRenderbufferEXT");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFEREXTPROC) wglGetProcAddress("glBindFramebufferEXT");

	glBindBuffer = (pfn_glBindBuffer)wglGetProcAddress("glBindBuffer");
	glDeleteBuffers = (pfn_glDeleteBuffers)wglGetProcAddress("glDeleteBuffers");
	glGenBuffers = (pfn_glGenBuffers)wglGetProcAddress("glGenBuffers");
	glIsBuffer = (pfn_glIsBuffer)wglGetProcAddress("glIsBuffer");
	glMapBuffer = (pfn_glMapBuffer)wglGetProcAddress("glMapBuffer");
	glUnmapBuffer = (pfn_glUnmapBuffer)wglGetProcAddress("glUnmapBuffer");
	glBufferData = (pfn_glBufferData)wglGetProcAddress("glBufferData");
	glBufferSubData = (pfn_glBufferSubData)wglGetProcAddress("glBufferSubData");
	glGetBufferSubData = (pfn_glGetBufferSubData)wglGetProcAddress("glGetBufferSubData");
	glGetBufferParameteriv = (pfn_glGetBufferParameteriv)wglGetProcAddress("glGetBufferParameteriv");
	glGetBufferPointerv = (pfn_glGetBufferPointerv)wglGetProcAddress("glGetBufferPointerv");

	glVertexAttrib1f = (pfn_glVertexAttrib1f)wglGetProcAddress("glVertexAttrib1f");
	glVertexAttrib2f = (pfn_glVertexAttrib2f)wglGetProcAddress("glVertexAttrib2f");
	glVertexAttrib3f = (pfn_glVertexAttrib3f)wglGetProcAddress("glVertexAttrib3f");
	glVertexAttrib4f = (pfn_glVertexAttrib4f)wglGetProcAddress("glVertexAttrib4f");
	glVertexAttrib1fv = (pfn_glVertexAttrib1fv)wglGetProcAddress("glVertexAttrib1fv");
	glVertexAttrib2fv = (pfn_glVertexAttrib2fv)wglGetProcAddress("glVertexAttrib2fv");
	glVertexAttrib3fv = (pfn_glVertexAttrib3fv)wglGetProcAddress("glVertexAttrib3fv");
	glVertexAttrib4fv = (pfn_glVertexAttrib4fv)wglGetProcAddress("glVertexAttrib4fv");
	glVertexAttribPointer = (pfn_glVertexAttribPointer)wglGetProcAddress("glVertexAttribPointer");
	glEnableVertexAttribArray = (pfn_glEnableVertexAttribArray)wglGetProcAddress("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (pfn_glDisableVertexAttribArray)wglGetProcAddress("glDisableVertexAttribArray");
	glBindAttribLocation = (pfn_glBindAttribLocation)wglGetProcAddress("glBindAttribLocation");
	glGetActiveAttrib = (pfn_glGetActiveAttrib)wglGetProcAddress("glGetActiveAttrib");
	glGetAttribLocation = (pfn_glGetAttribLocation)wglGetProcAddress("glGetAttribLocation");
	glGetVertexAttribfv = (pfn_glGetVertexAttribfv)wglGetProcAddress("glGetVertexAttribfv");
	glGetVertexAttribiv = (pfn_glGetVertexAttribiv)wglGetProcAddress("glGetVertexAttribiv");
	glGetVertexAttribPointerv = (pfn_glGetVertexAttribPointerv)wglGetProcAddress("glGetVertexAttribPointerv");
	glCreateProgram = (pfn_glCreateProgram)wglGetProcAddress("glCreateProgram");
	glCreateShader = (pfn_glCreateShader)wglGetProcAddress("glCreateShader");
	glAttachShader = (pfn_glAttachShader)wglGetProcAddress("glAttachShader");
	glDeleteProgram = (pfn_glDeleteProgram)wglGetProcAddress("glDeleteProgram");
	glDeleteShader = (pfn_glDeleteShader)wglGetProcAddress("glDeleteShader");
	glDetachShader = (pfn_glDetachShader)wglGetProcAddress("glDetachShader");
	glGetAttachedShaders = (pfn_glGetAttachedShaders)wglGetProcAddress("glGetAttachedShaders");
	glGetProgramInfoLog = (pfn_glGetProgramInfoLog)wglGetProcAddress("glGetProgramInfoLog");
	glIsShader = (pfn_glIsShader)wglGetProcAddress("glIsShader");
	glGetShaderiv = (pfn_glGetShaderiv)wglGetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (pfn_glGetShaderInfoLog)wglGetProcAddress("glGetShaderInfoLog");
	glUniform1i = (pfn_glUniform1i)wglGetProcAddress("glUniform1i");
	glUniform2i = (pfn_glUniform2i)wglGetProcAddress("glUniform2i");
	glUniform3i = (pfn_glUniform3i)wglGetProcAddress("glUniform3i");
	glUniform4i = (pfn_glUniform4i)wglGetProcAddress("glUniform4i");
	glUniform1f = (pfn_glUniform1f)wglGetProcAddress("glUniform1f");
	glUniform2f = (pfn_glUniform2f)wglGetProcAddress("glUniform2f");
	glUniform3f = (pfn_glUniform3f)wglGetProcAddress("glUniform3f");
	glUniform4f = (pfn_glUniform4f)wglGetProcAddress("glUniform4f");
	glUniform1iv = (pfn_glUniform1iv)wglGetProcAddress("glUniform1iv");
	glUniform2iv = (pfn_glUniform2iv)wglGetProcAddress("glUniform2iv");
	glUniform3iv = (pfn_glUniform3iv)wglGetProcAddress("glUniform3iv");
	glUniform4iv = (pfn_glUniform4iv)wglGetProcAddress("glUniform4iv");
	glUniform1fv = (pfn_glUniform1fv)wglGetProcAddress("glUniform1fv");
	glUniform2fv = (pfn_glUniform2fv)wglGetProcAddress("glUniform2fv");
	glUniform3fv = (pfn_glUniform3fv)wglGetProcAddress("glUniform3fv");
	glUniform4fv = (pfn_glUniform4fv)wglGetProcAddress("glUniform4fv");
	glUniformMatrix2fv = (pfn_glUniformMatrix2fv)wglGetProcAddress("glUniformMatrix2fv");
	glUniformMatrix3fv = (pfn_glUniformMatrix3fv)wglGetProcAddress("glUniformMatrix3fv");
	glUniformMatrix4fv = (pfn_glUniformMatrix4fv)wglGetProcAddress("glUniformMatrix4fv");
	glGetUniformfv = (pfn_glGetUniformfv)wglGetProcAddress("glGetUniformfv");
	glGetUniformiv = (pfn_glGetUniformiv)wglGetProcAddress("glGetUniformiv");
	glGetUniformLocation = (pfn_glGetUniformLocation)wglGetProcAddress("glGetUniformLocation");
	glGetActiveUniform = (pfn_glGetActiveUniform)wglGetProcAddress("glGetActiveUniform");
	glLinkProgram = (pfn_glLinkProgram)wglGetProcAddress("glLinkProgram");
	glValidateProgram = (pfn_glValidateProgram)wglGetProcAddress("glValidateProgram");
	glGetShaderSource = (pfn_glGetShaderSource)wglGetProcAddress("glGetShaderSource");
	glShaderSource = (pfn_glShaderSource)wglGetProcAddress("glShaderSource");
	glCompileShader = (pfn_glCompileShader)wglGetProcAddress("glCompileShader");
	glUseProgram = (pfn_glUseProgram)wglGetProcAddress("glUseProgram");
	glGetProgramiv = (pfn_glGetProgramiv)wglGetProcAddress("glGetProgramiv");
	glActiveTexture = (pfn_glActiveTexture)wglGetProcAddress("glActiveTexture");
	glIsProgram = (pfn_glIsProgram)wglGetProcAddress("glIsProgram");
#endif
}
