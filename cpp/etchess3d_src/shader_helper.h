#ifndef _SHADER_HELPER_H_
#define _SHADER_HELPER_H_

#include "math_helpers.h"
#include "opengl_helpers.h"
#include "texture_helper.h"

class MyShader
{
public:
	MyShader() : prog(0), vs(0), fs(0) {}
	GLuint prog;
	GLuint vs;
	GLuint fs;
	enum 
	{
		Sky,
		Normal,
		SimpleColor,
		SimpleTexture,
		Color,
		Marble
	} type;
	void destroy() {
		glDeleteProgram(prog);
		prog = 0;
		glDeleteShader(vs);
		vs = 0;
		glDeleteShader(fs);
		fs = 0;
	}
};

class MyShaderSky : public MyShader
{
public:
	MyShaderSky() : MyShader() {}
	GLuint wvp;
	GLuint texture;
};

class MyShaderShadowGenerate : public MyShader
{
public:
	MyShaderShadowGenerate() : MyShader() {}
	GLuint wvp_light;
};

class MyShaderShadowFragment : public MyShader
{
public:
	MyShaderShadowFragment() : MyShader() {}
	GLuint wvp_light;
	GLuint shadow_texture;
	GLuint shadowmap_width;
	GLuint shadowmap_height;
	void reload_shadow_fragment() 
	{
		wvp_light = glGetUniformLocation( prog, "WORLD_VIEW_PROJECTION_LIGHT" );
		shadow_texture = glGetUniformLocation( prog, "shadow_texture" );
		shadowmap_width = glGetUniformLocation( prog, "shadowmap_width" );
		shadowmap_height = glGetUniformLocation( prog, "shadowmap_height" );
	}
};

class MyShaderShadowRender : public MyShaderShadowFragment
{
public:
	MyShaderShadowRender() : MyShaderShadowFragment() {}
	GLuint wvp;
};

class MyShaderNormal : public MyShaderShadowFragment
{
public:
	MyShaderNormal() : MyShaderShadowFragment() {}
	GLuint wvp;
	GLuint world;
	GLuint world_inverse_transpose;
	GLuint texture_ddn;
	GLuint texture_diff;
	GLuint lightW;
	GLuint camposW;
	MyTexture2D texture_data_ddn;
	MyTexture2D texture_data_diff;
	GLuint texture_scale;
	GLuint texture_offset;
};

class MyShaderMarble : public MyShaderShadowFragment
{
public:
	MyShaderMarble() : MyShaderShadowFragment() {type = MyShader::Marble; use_3d_texture = false;}
	GLuint wvp;
	GLuint world;
	GLuint world_inverse;
	GLuint world_inverse_transpose;
	GLuint texture_noise;
	GLuint lightW;
	GLuint camposW;
	GLuint color;
	bool use_3d_texture;
	MyTexture2D volume_texture_data;
};

class MyShaderSimpleColor : public MyShader
{
public:
	MyShaderSimpleColor() : MyShader() {type = MyShader::SimpleColor;}
	GLuint color;
	GLuint lightW;
	GLuint wvp;
	GLuint world;
	GLuint world_inverse_transpose;
	GLuint campos;
	GLuint fadeout;
};

class MyShaderTexture : public MyShaderShadowFragment
{
public:
	MyShaderTexture() : MyShaderShadowFragment() {}
	GLuint wvp;
	GLuint texture;
	GLuint texture_scale;
	GLuint texture_offset;
};

class MyShaderSimpleTexture : public MyShader
{
public:
	MyShaderSimpleTexture() : MyShader() { type = MyShader::SimpleTexture; }
	GLuint wvp;
	GLuint texture;
	GLuint alpha;
};

class MyShaderColor : public MyShader
{
public:
	MyShaderColor() : MyShader() {}
	GLuint wvp;
};

#endif 
