#include <string.h>
#include <assert.h>
#include <lib3ds.h>
#include <tinyxml2.h>
#include <stdio.h>
#include <stdlib.h>
#include <Board.h>
#include <Engine.h>

#include "TestGLSL.h"
#include "math_helpers.h"
#include "opengl_helpers.h"
#include "geometry_helpers.h"
#include "vertex_declaration_helpers.h"
#include "texture_helper.h"
#include "mymath.h"
#include "shader_helper.h"
#include "ee_scene_gl.h"
#include "3ds_helpers.h"
#include "simple_vector.h"
#include "mythreading.h"
#include "os_helpers.h"
#include "chess_helpers.h"
#include "ee_camera.h"
#include "../native-lib.h"

#if PLATFORM_WINDOWS
#include <GL/glut.h>
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif


#ifdef NO_DEBUG
#define OPTIMIZATIONS 1

#else
#define OPTIMIZATIONS 0 
#endif

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))
#define SIZE_OF_ARRAY(x) sizeof((x))/sizeof((x[0]))

#if defined( PLATFORM_ARM_ANDROID )
EGLContext context;
EGLDisplay display;
EGLConfig *configs = (EGLConfig * )NULL;
EGLSurface surface;
#endif

mutex_handle mtxRender;
static bool terminating = false;

/* Window state */
static TestGLSL tglsl;
static int windowWidth = 800;
static int windowHeight = 600;

/* Shadow map state */
#if 1 == OPTIMIZATIONS
static int shadowmapWidth = windowWidth/2;
static int shadowmapHeight = windowHeight/2;
#else
static int shadowmapWidth = windowWidth;
static int shadowmapHeight = windowHeight;
#endif

/* Picking state */
static bool leftMouseDown = false;
static float mousePrevX = 0;
static float mousePrevY = 0;
static myvec3 pickedInstanceStartPos;
static myvec3 pickedInstanceMovePos;
static EE_GeometryInstance *pickedInstance = NULL;

/* FPS state */
unsigned long long prevTimestamp;
unsigned int frameCount = 0;
static float currentTime = 0.f;

/* Scene state */
EE_Scene *mainScene = NULL;

/* Depth state */
typedef struct
{
	GLint format;
	GLint type;
} my_depth_format;

static my_depth_format depth_preferences[] = {
	{ GL_DEPTH_COMPONENT, GL_FLOAT },
/*	{ GL_DEPTH_COMPONENT, GL_UNSIGNED_INT }, */
	{ GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT },
	{ GL_DEPTH_COMPONENT16, GL_UNSIGNED_SHORT },
	{ GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE },
};
static my_depth_format depth_preferred = {GL_NONE, GL_NONE};

/* Blend modes */
GLenum blend_modes[] = {
	GL_ZERO, 
	GL_ONE, 
	GL_SRC_COLOR, 
	GL_ONE_MINUS_SRC_COLOR, 
	GL_DST_COLOR, 
	GL_ONE_MINUS_DST_COLOR, 
	GL_SRC_ALPHA, 
	GL_ONE_MINUS_SRC_ALPHA, 
	GL_DST_ALPHA, 
	GL_ONE_MINUS_DST_ALPHA, 
	GL_CONSTANT_COLOR, 
	GL_ONE_MINUS_CONSTANT_COLOR, 
	GL_CONSTANT_ALPHA, 
	GL_ONE_MINUS_CONSTANT_ALPHA, 
	GL_SRC_ALPHA_SATURATE
};
GLenum blend_src = blend_modes[6];
GLenum blend_dst = blend_modes[7];

/* Vertex declarations */
MyVertexDeclaration vdIconResetGame = {};
MyVertexDeclaration vdIconCheckmate = {};
MyVertexDeclaration vdIconCheck = {};
MyVertexDeclaration vdIconChessBusy = {};
MyVertexDeclaration vdSphere = {};
MyVertexDeclaration vdBox = {};
MyVertexDeclaration vdHalfDome = {};

/* Textures */
MyTexture2D textureIcons = {};
MyTexture2D textureGalaxy = {};
MyTexture2D textureBricksDiff = {};
MyTexture2D textureBricksDdn = {};
MyTexture2D textureNoise = {};

/* Shaders */
MyShaderMarble shaderMarble = MyShaderMarble();
MyShaderNormal shaderNormal = MyShaderNormal();
MyShaderSky shaderSky = MyShaderSky();
MyShaderSimpleColor shaderSimpleColor = MyShaderSimpleColor();
MyShaderSimpleTexture shaderIcon = MyShaderSimpleTexture();
MyShaderTexture shaderTexture = MyShaderTexture();
MyShaderShadowGenerate shaderShadowGenerate = MyShaderShadowGenerate();
MyShaderShadowRender shaderShadowRender = MyShaderShadowRender();

/* xml state */
static tinyxml2::XMLDocument *xmldocument = NULL;

/* Chess state */
static bool chessThreadDone = false;

/* Forward declarations */
void initApp( bool use_save );
void destroyApp( bool save_the_game );
void cbResize( int width, int height );

EE_Scene *get_scene()
{
	return mainScene;
}

/* Scene rendering helpers */
void renderIconResetGame()
{
	mymat4 wvpMatrix_icons = (*get_camera()->viewProjQuad())*mymat4::translation( -windowWidth/2.f, (windowHeight)/2.f-50.f, 0 );
	
	glUseProgram( shaderIcon.prog );
	glUniformMatrix4fv( shaderIcon.wvp, 1, GL_FALSE, wvpMatrix_icons );

	setTexture( &textureIcons, 0 );
	glUniform1i( shaderIcon.texture, 0 );
	glUniform1f( shaderIcon.alpha, 0.5f);

	setVertexDeclaration( &vdIconResetGame, &shaderIcon );
	setBuffer( &vdIconResetGame );
	glDrawElements( GL_TRIANGLES, vdIconResetGame.nindices, GL_UNSIGNED_SHORT, (void*)0 );
	unbindDeclaration( &vdIconResetGame, &shaderIcon );
}

void renderIconChessBusy()
{
	mymat4 wvpMatrix_icons = (*get_camera()->viewProjQuad())*mymat4::translation( windowWidth/2.f-50.f, (windowHeight)/2.f-50.f, 0 );
	
	glUseProgram( shaderIcon.prog );
	glUniformMatrix4fv( shaderIcon.wvp, 1, GL_FALSE, wvpMatrix_icons );

	setTexture( &textureIcons, 0 );
	glUniform1i( shaderIcon.texture, 0 );
	glUniform1f( shaderIcon.alpha, 0.7f);

	setVertexDeclaration( &vdIconChessBusy, &shaderIcon );
	setBuffer( &vdIconChessBusy );
	glDrawElements( GL_TRIANGLES, vdIconChessBusy.nindices, GL_UNSIGNED_SHORT, (void*)0 );
	unbindDeclaration( &vdIconChessBusy, &shaderIcon );
}

void renderIconCheck()
{
	mymat4 wvpMatrix_icons = (*get_camera()->viewProjQuad())*mymat4::translation( windowWidth/2.f-50.f, (windowHeight)/2.f-50.f, 0 );
	
	glUseProgram( shaderIcon.prog );
	glUniformMatrix4fv( shaderIcon.wvp, 1, GL_FALSE, wvpMatrix_icons );

	setTexture( &textureIcons, 0 );
	glUniform1i( shaderIcon.texture, 0 );
	glUniform1f( shaderIcon.alpha, 0.7f);

	setVertexDeclaration( &vdIconCheck, &shaderIcon );
	setBuffer( &vdIconCheck );
	glDrawElements( GL_TRIANGLES, vdIconCheck.nindices, GL_UNSIGNED_SHORT, (void*)0 );
	unbindDeclaration( &vdIconCheck, &shaderIcon );
}

void renderIconCheckmate()
{
	mymat4 wvpMatrix_icons = (*get_camera()->viewProjQuad())*mymat4::translation( windowWidth/2.f-50.f, (windowHeight)/2.f-50.f, 0 );
	
	glUseProgram( shaderIcon.prog );
	glUniformMatrix4fv( shaderIcon.wvp, 1, GL_FALSE, wvpMatrix_icons );
	glUniform1f( shaderIcon.alpha, 0.7f);

	setTexture( &textureIcons, 0 );
	glUniform1i( shaderIcon.texture, 0 );

	setVertexDeclaration( &vdIconCheckmate, &shaderIcon );
	setBuffer( &vdIconCheckmate );
	glDrawElements( GL_TRIANGLES, vdIconCheckmate.nindices, GL_UNSIGNED_SHORT, (void*)0 );
	unbindDeclaration( &vdIconCheckmate, &shaderIcon );
}

void renderSky()
{
	const myvec3 *p = get_camera()->pos();
	glCullFace( GL_BACK );
	glDisable( GL_DEPTH_TEST );

	glUseProgram( shaderSky.prog );
	glUniformMatrix4fv( shaderSky.wvp, 1, GL_FALSE, (*get_camera()->viewProj())*mymat4::translation( p->x, p->y, p->z ) );
	setTexture( &textureGalaxy, 0 );
	CHECK_GL_ERROR;
	glUniform1i( shaderSky.texture, 0 );
	CHECK_GL_ERROR;
	setVertexDeclaration( &vdHalfDome, &shaderSky );
	glDrawElements( GL_TRIANGLES, vdHalfDome.nindices, GL_UNSIGNED_SHORT, (void*)0 );
	unbindDeclaration( &vdHalfDome, &shaderSky );
	glEnable( GL_DEPTH_TEST );
}

void render_sphere( EE_GeometryInstance *instance )
{
       if ( instance == NULL || instance->geometry == NULL ) return;
       float color[4] = {1,0,0,1};
       mymat4 wvp_sphere = (*get_camera()->viewProj()) * (instance->matrix*mymat4::scale( instance->geometry->size/2, instance->geometry->size/2, instance->geometry->size/2 ));
       myvec4 lightPosW = myvec4( mainScene->getLight(), 1 );

       glUseProgram( shaderSimpleColor.prog );
       glUniform4fv( shaderSimpleColor.color, 1, color );
       glUniform4fv( shaderSimpleColor.lightW, 1, lightPosW );
       glUniform3fv( shaderSimpleColor.campos, 1, (GLfloat*)get_camera()->pos() );
       glUniformMatrix4fv( shaderSimpleColor.wvp, 1, GL_FALSE, wvp_sphere );
       glUniformMatrix4fv( shaderSimpleColor.world, 1, GL_FALSE, instance->matrix );
       setVertexDeclaration( &vdSphere, &shaderSimpleColor );
       glDrawElements( GL_LINES, vdSphere.nindices, GL_UNSIGNED_SHORT, (void*)0 );
       unbindDeclaration( &vdSphere, &shaderSimpleColor );
}

void render_box( EE_GeometryInstance *instance )
{
	if ( instance == NULL || instance->geometry == NULL ) return;
	float color[4] = {1,0,0,0.5};
	mymat4 world;
	mymat4 world_inverse;
	world.invert( world_inverse );
	world = instance->ctm*mymat4::translation( instance->geometry->bbc)*mymat4::scale( instance->geometry->bbs );
	mymat4 wvp_box = (*get_camera()->viewProj()) * world;
	myvec4 lightPosW = myvec4( mainScene->getLight(), 1 );
	GLint src_blend;
	GLint dst_blend;
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram( shaderSimpleColor.prog );
	glUniform4fv( shaderSimpleColor.color, 1, color );
	glUniform4fv( shaderSimpleColor.lightW, 1, lightPosW );
	glUniform3fv( shaderSimpleColor.campos, 1, (GLfloat*)get_camera()->pos() );
	glUniformMatrix4fv( shaderSimpleColor.wvp, 1, GL_FALSE, wvp_box );
	glUniformMatrix4fv( shaderSimpleColor.world, 1, GL_FALSE, world );
	glUniformMatrix4fv( shaderSimpleColor.world_inverse_transpose, 1, GL_FALSE, world_inverse.transposed() );
	setVertexDeclaration( &vdBox, &shaderSimpleColor );
	glDrawElements( GL_TRIANGLES, vdBox.nindices, GL_UNSIGNED_SHORT, (void*)0 );
	unbindDeclaration( &vdBox, &shaderSimpleColor );
}

void resetBoxRendering()
{
	for ( int i = 0; i < mainScene->geometryinstances.size(); i++ )
	{
		mainScene->geometryinstances[i]->do_render_box = false;
	}
}

/* Scene picking helpers */
void TestGLSL::onSecondPointerDown( int first_x, int first_y, int other_x, int other_y )
{
	if(pickedInstance)
	{
		onPointerUp( 0, first_x, first_y, false);
	}
	myvec2 first = myvec2(first_x,first_y);
	myvec2 other = myvec2(other_x,other_y);
	get_camera()->orbitScale(first,other);
}

void TestGLSL::onSecondPointerUp( int other_x, int other_y )
{
	myvec2 other = myvec2(0,0);
	myvec2 first = myvec2(0,0);
	get_camera()->orbitScale(first,other );
}

void TestGLSL::onSecondPointerMove( int first_x, int first_y, int other_x, int other_y )
{
	//Scaled multigesture is dependent on distance from first other position 
	myvec2 first = myvec2(first_x, first_y);
	myvec2 other = myvec2(other_x, other_y);	
	get_camera()->orbitScale(first,other );
}

void TestGLSL::onMove( int x, int y )
{
	if ( ! (leftMouseDown) ) return;
	if ( pickedInstance )
	{
		//picking, move object
		float deltaX = x - mousePrevX;
		float deltaY = mousePrevY - y;
		get_scene()->moveObjectAlongCameraPlane( pickedInstance, deltaX, deltaY, pickedInstanceMovePos );
		mousePrevX = x;
		mousePrevY = y;
	}
	else
	{
		//nothing picked, assume camera orbit move
		get_camera()->orbitMove( x, y );
	}
}

void TestGLSL::onPointerMove( int id, int x, int y )
{
	onMove( x, y );
}

void TestGLSL::onPointerUp(int id, int x, int y, bool right_click) 
{
	get_camera()->pointerUp( x, y );

	if ( leftMouseDown == true ) 
	{
		leftMouseDown = false;
		if ( pickedInstance != NULL ) 
		{
			/* Thumbs up, check the chess move */
			resetBoxRendering();
			bool failedMove = user_moved_chess_piece( pickedInstance );
			if ( failedMove )
			{
				pickedInstance->matrix[12] = pickedInstanceStartPos.x;
				pickedInstance->matrix[13] = pickedInstanceStartPos.y;
				pickedInstance->matrix[14] = pickedInstanceStartPos.z;
			}
			pickedInstance = NULL;
		}
	}
}

vector<EE_GeometryInstance *> *get_all_instances()
{
	return &mainScene->geometryinstances;
}

vector<EE_Geometry *> *get_all_geometries()
{
	return &mainScene->geometries;
}

void *get_xml_document()
{
	return xmldocument;
}

void reset_game()
{
	grab_mutex( mtxRender );
	terminating = true;
	release_mutex( mtxRender );
	reset_chess_board();
	initApp(false);
	cbResize(windowWidth, windowHeight);
}

void TestGLSL::onPointerDown(int id, int x, int y, bool right_click) 
{
	leftMouseDown = !right_click;
	get_camera()->pointerDown( x, y );
#if !PLATFORM_ARM_ANDROID
	if ( y <= 50.f && x <= 50.f ) 
	{
		/* Reset icon clicked */
		reset_game();
	}
	else
#endif
	{
		/* otherwise, picking */
		pickedInstance = mainScene->pick( x, y );
		if ( pickedInstance != NULL )
		{
			pickedInstance->do_render_box = true;
			//Make a copy of start pos in case of reverting a failed move
			pickedInstanceStartPos = myvec3( pickedInstance->matrix[12], pickedInstance->matrix[13], pickedInstance->matrix[14]);
			//Make a copy of start pos that is delta-updated on Move.
			pickedInstanceMovePos = pickedInstanceStartPos;
			Base::log( "Closest instance %s (%.2f)/50.f=%.2f (%.2f)/50.f=%.2f\n", pickedInstance->getName(), pickedInstance->matrix[12],pickedInstance->matrix[12] / 50.f,pickedInstance->matrix[13],pickedInstance->matrix[13] / 50.f);
			pick_chess_piece( pickedInstance );
		}
	}

	mousePrevX = x;
	mousePrevY = y;
}

/* Main render loop */
void cbRender( )
{
	grab_mutex( mtxRender );
	if (terminating)
	{
		release_mutex( mtxRender );
		return;
	}
	CHECK_GL_ERROR;
	chessThreadDone = check_chess_engine_done( mainScene->geometryinstances, mainScene->geometries, xmldocument );
	CHECK_GL_ERROR;

	unsigned long long curTimestamp = timer_get_time();
	double deltatime = double( curTimestamp - prevTimestamp ) / double(get_timer_tick_resolution());
	prevTimestamp = curTimestamp;
	currentTime += deltatime;

	get_camera()->update( deltatime );

	mymat4 identity = mymat4::identity();
	
	/* First rendershadow map */
	CHECK_GL_ERROR;
	glDisable( GL_BLEND );
	CHECK_GL_ERROR;
	glEnable( GL_CULL_FACE );
	CHECK_GL_ERROR;
	glEnable( GL_DEPTH_TEST );
	CHECK_GL_ERROR;
	glDepthMask( GL_TRUE );
	CHECK_GL_ERROR;
	glCullFace( GL_FRONT );
	CHECK_GL_ERROR;
	glViewport(0,0,shadowmapWidth,shadowmapHeight);
	CHECK_GL_ERROR;
	glDepthFunc( GL_LESS );
	CHECK_GL_ERROR;
	glBindFramebuffer(GL_FRAMEBUFFER, mainScene->FramebufferName);
	CHECK_GL_ERROR;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mainScene->depthTexture, 0);
	CHECK_GL_ERROR;
#if PLATFORM_WINDOWS
	glDrawBuffer(GL_NONE);
	CHECK_GL_ERROR;
#endif
	glClear( GL_DEPTH_BUFFER_BIT );
	CHECK_GL_ERROR;
 
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
		return;
	}
	CHECK_GL_ERROR;

	/* Render shadow occluders only */
	mainScene->shadow_generate = true;
	mainScene->shadow_render = false;
	mainScene->render( deltatime, &identity );

	/*writePNGFromDepthTexture( "depth.png", mainScene->depthTexture, shadowmapWidth, shadowmapHeight );*/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CHECK_GL_ERROR;
	glViewport(0,0,windowWidth, windowHeight);
	glEnable( GL_BLEND );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	/* Then render scene normally */
	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	renderSky();
	CHECK_GL_ERROR;

	glEnable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	mainScene->shadow_generate = false;
#if 1
	mainScene->shadow_render = true;
	mainScene->render( deltatime, &identity );
	CHECK_GL_ERROR;
	if ( mainScene->selected )
	{
		render_box( mainScene->selected );
		mainScene->selected = NULL;
	}
#else
	mainScene->shadow_render = false;
	mainScene->render( deltatime, &identity );
	CHECK_GL_ERROR;

	/* Then render shadow receivers only */
	glEnable( GL_BLEND );
	glEnable(GL_CULL_FACE);
	glCullFace( GL_BACK );
	glEnable( GL_DEPTH_TEST );
	glClear( GL_DEPTH_BUFFER_BIT );
	GLint tmp_src_blend;
	GLint tmp_dst_blend;
	glGetIntegerv( GL_BLEND_SRC, &tmp_src_blend );
	glGetIntegerv( GL_BLEND_DST, &tmp_dst_blend );
	glBlendFunc(blend_src, blend_dst);

	mainScene->shadow_render = true;
	mainScene->shadow_generate = false;
	mainScene->render( deltatime, &identity );
	glBlendFunc( tmp_src_blend, tmp_dst_blend );
	glEnable( GL_CULL_FACE );
	glCullFace(GL_BACK);
#endif
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

#if !PLATFORM_ARM_ANDROID
	//icons for chess 
	renderIconResetGame();
	CHECK_GL_ERROR;
#endif

	if ( !player_can_move() )
	{
		renderIconChessBusy();
		CHECK_GL_ERROR;
	}
	if ( check_player_in_check() )
	{
		renderIconCheck();
		CHECK_GL_ERROR;
	}
	if ( check_player_in_checkmate() || check_opponent_in_checkmate() )
	{
		renderIconCheckmate();
		CHECK_GL_ERROR;
	}

	frameCount++;
	if ( frameCount % 60 == 0 ) 
	{
		Base::log( "FPS: %.6f Frames: %u Time: %.6f\n", frameCount / currentTime , frameCount, currentTime);
	}
	CHECK_GL_ERROR;

#if defined( PLATFORM_ARM_ANDROID )
	eglSwapBuffers( display, surface );
#else
	glutSwapBuffers();
#endif
	CHECK_GL_ERROR;
	release_mutex( mtxRender );
}

void TestGLSL::onNewFrame()
{
	if ( animating ) cbRender();
}

/* Application init, reload and destruction */

void reloadShaders()
{
	//If one loaded assume all loaded
	if (shaderSky.prog)
	{
		return;
	}

	//Better assert above assumption
	assert(!shaderSky.prog && !shaderIcon.prog && !shaderMarble.prog &&
	 	!shaderNormal.prog && !shaderShadowGenerate.prog && !shaderShadowRender.prog && 
		!shaderSimpleColor.prog && !shaderTexture.prog);
	
	char *simple_vsdata = textFileRead(PATHSTR("shaders", "simple.vert"));
	char *simple_fsdata = textFileRead(PATHSTR("shaders", "simple.frag"));
	char *icons_vsdata = textFileRead(PATHSTR("shaders","icons.vert"));
	char *icons_fsdata = textFileRead(PATHSTR("shaders", "icons.frag"));
	char *texture_vsdata = textFileReadFragment(PATHSTR("shaders","shadow_fragment.vert"),PATHSTR("shaders","tex.vert"));
	char *texture_fsdata = textFileReadFragment(PATHSTR("shaders","shadow_fragment.frag"),PATHSTR("shaders", "tex.frag"));
	char *sky_vsdata = textFileRead(PATHSTR("shaders","sky.vert"));
	char *sky_fsdata = textFileRead(PATHSTR("shaders","sky.frag"));
	char *normal_vsdata = textFileReadFragment(PATHSTR("shaders","shadow_fragment.vert"),PATHSTR("shaders","normal.vert"));
	char *normal_fsdata = textFileReadFragment(PATHSTR("shaders","shadow_fragment.frag"),PATHSTR("shaders","normal.frag"));
	char *shadow_generate_vsdata = textFileRead(PATHSTR("shaders", "shadow_generate.vert"));
	char *shadow_generate_fsdata = textFileRead(PATHSTR("shaders", "shadow_generate.frag"));
	char *shadow_render_vsdata = textFileRead(PATHSTR("shaders", "shadow_render.vert"));
	char *shadow_render_fsdata = textFileRead(PATHSTR("shaders", "shadow_render.frag"));
	char *marble_vsdata;
	char *marble_fsdata;

	if ( shaderMarble.use_3d_texture )
	{
		marble_vsdata = textFileReadFragment(PATHSTR("shaders","shadow_fragment.vert"),PATHSTR("shaders", "marble_tex3d.vert"));
		marble_fsdata = textFileRead(PATHSTR("shaders", "marble_tex3d.frag"));
	}
	else
	{
		marble_vsdata = textFileReadFragment(PATHSTR("shaders","shadow_fragment.vert"),PATHSTR("shaders", "marble.vert"));
		marble_fsdata = textFileReadFragment(PATHSTR("shaders","shadow_fragment.frag"),PATHSTR("shaders", "marble.frag"));
	}

	assert( simple_vsdata != NULL && simple_fsdata != NULL && icons_vsdata != NULL && icons_fsdata != NULL 
		&& marble_vsdata != NULL && marble_fsdata != NULL
		&& sky_vsdata != NULL && sky_fsdata != NULL && normal_vsdata != NULL && normal_fsdata != NULL 
		&& shadow_generate_vsdata != NULL && shadow_generate_fsdata != NULL && shadow_render_vsdata != NULL
		&& shadow_render_fsdata != NULL && icons_vsdata != NULL && icons_fsdata != NULL );

	grab_mutex( mtxRender );

	{
		shaderMarble.vs = glCreateShader(GL_VERTEX_SHADER);
		shaderMarble.fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource( shaderMarble.vs, 1, ((const char**)&marble_vsdata), NULL );
		glShaderSource( shaderMarble.fs, 1, ((const char**)&marble_fsdata), NULL );
		glCompileShader( shaderMarble.vs );
		glCompileShader( shaderMarble.fs );
		shaderMarble.prog = glCreateProgram();
		glAttachShader(shaderMarble.prog, shaderMarble.fs);
		glAttachShader(shaderMarble.prog, shaderMarble.vs);
		glLinkProgram(shaderMarble.prog);
		printShaderInfoLog( shaderMarble.vs );
		printShaderInfoLog( shaderMarble.fs );
		printProgramInfoLog( shaderMarble.prog );
		shaderMarble.texture_noise = glGetUniformLocation( shaderMarble.prog, "NoiseTex" );
		shaderMarble.wvp = glGetUniformLocation( shaderMarble.prog, "WORLD_VIEW_PROJECTION" );
		shaderMarble.world = glGetUniformLocation( shaderMarble.prog, "WORLD" );
		shaderMarble.world_inverse = glGetUniformLocation( shaderMarble.prog, "WORLD_INVERSE" );
		shaderMarble.world_inverse_transpose = glGetUniformLocation( shaderMarble.prog, "WORLD_INVERSE_TRANSPOSE" );
		shaderMarble.camposW = glGetUniformLocation( shaderMarble.prog, "wEye" );
		shaderMarble.lightW  = glGetUniformLocation( shaderMarble.prog, "wLightPos" );
		shaderMarble.color = glGetUniformLocation( shaderMarble.prog, "color" );
		//grab shadow uniforms
		shaderMarble.reload_shadow_fragment();
		Base::log( "shaderMarble.prog = %d\n", shaderMarble.prog );
	}
	CHECK_GL_ERROR;

	{
		shaderNormal.vs = glCreateShader(GL_VERTEX_SHADER);
		shaderNormal.fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource( shaderNormal.vs, 1, ((const char**)&normal_vsdata), NULL );
		glShaderSource( shaderNormal.fs, 1, ((const char**)&normal_fsdata), NULL );
		glCompileShader( shaderNormal.vs );
		glCompileShader( shaderNormal.fs );
		shaderNormal.prog = glCreateProgram();
		glAttachShader(shaderNormal.prog, shaderNormal.fs);
		glAttachShader(shaderNormal.prog, shaderNormal.vs);
		glLinkProgram(shaderNormal.prog);
		printShaderInfoLog( shaderNormal.vs );
		printShaderInfoLog( shaderNormal.fs );
		printProgramInfoLog( shaderNormal.prog );
		shaderNormal.texture_ddn = glGetUniformLocation( shaderNormal.prog, "texNorm" );
		shaderNormal.texture_diff = glGetUniformLocation( shaderNormal.prog, "tex" );
		shaderNormal.wvp = glGetUniformLocation( shaderNormal.prog, "WORLD_VIEW_PROJECTION" );
		shaderNormal.world = glGetUniformLocation( shaderNormal.prog, "WORLD" );
		shaderNormal.camposW = glGetUniformLocation( shaderNormal.prog, "wEye" );
		shaderNormal.lightW  = glGetUniformLocation( shaderNormal.prog, "lightW" );
		shaderNormal.texture_scale = glGetUniformLocation( shaderNormal.prog, "texture_scale" );
		shaderNormal.texture_offset = glGetUniformLocation( shaderNormal.prog, "texture_offset" );
		shaderNormal.world_inverse_transpose = glGetUniformLocation( shaderNormal.prog, "WORLD_INVERSE_TRANSPOSE" );
		shaderNormal.reload_shadow_fragment();
		Base::log( "shaderNormal.prog = %d\n", shaderNormal.prog );
	}
	CHECK_GL_ERROR;

	{
		shaderSky.vs = glCreateShader(GL_VERTEX_SHADER);
		shaderSky.fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource( shaderSky.vs, 1, ((const char**)&sky_vsdata), NULL );
		glShaderSource( shaderSky.fs, 1, ((const char**)&sky_fsdata), NULL );
		glCompileShader( shaderSky.vs );
		glCompileShader( shaderSky.fs );
		shaderSky.prog = glCreateProgram();
		glAttachShader(shaderSky.prog, shaderSky.fs);
		glAttachShader(shaderSky.prog, shaderSky.vs);
		glLinkProgram(shaderSky.prog);
		printShaderInfoLog( shaderSky.vs );
		printShaderInfoLog( shaderSky.fs );
		printProgramInfoLog( shaderSky.prog );
		shaderSky.texture = glGetUniformLocation( shaderSky.prog, "sky_texture" );
		shaderSky.wvp = glGetUniformLocation( shaderSky.prog, "WORLD_VIEW_PROJECTION" );
		Base::log( "shaderSky.prog = %d\n", shaderSky.prog );
	}
	CHECK_GL_ERROR;

	{
		shaderSimpleColor.vs = glCreateShader(GL_VERTEX_SHADER);
		shaderSimpleColor.fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource( shaderSimpleColor.vs, 1, ((const char**)&simple_vsdata), NULL );
		glShaderSource( shaderSimpleColor.fs, 1, ((const char**)&simple_fsdata), NULL );
		glCompileShader( shaderSimpleColor.vs );
		glCompileShader( shaderSimpleColor.fs );
		shaderSimpleColor.prog = glCreateProgram();
		glAttachShader(shaderSimpleColor.prog, shaderSimpleColor.fs);
		glAttachShader(shaderSimpleColor.prog, shaderSimpleColor.vs);
		glLinkProgram(shaderSimpleColor.prog);
		printShaderInfoLog( shaderSimpleColor.vs );
		printShaderInfoLog( shaderSimpleColor.fs );
		printProgramInfoLog( shaderSimpleColor.prog );

		shaderSimpleColor.color = glGetUniformLocation( shaderSimpleColor.prog, "color" );
		shaderSimpleColor.lightW = glGetUniformLocation( shaderSimpleColor.prog, "lightW" );
		shaderSimpleColor.campos = glGetUniformLocation( shaderSimpleColor.prog, "camPosW" );
		shaderSimpleColor.wvp = glGetUniformLocation( shaderSimpleColor.prog, "WORLD_VIEW_PROJECTION" );
		shaderSimpleColor.world = glGetUniformLocation( shaderSimpleColor.prog, "WORLD" );
		shaderSimpleColor.world_inverse_transpose = glGetUniformLocation( shaderSimpleColor.prog, "WORLD_INVERSE_TRANSPOSE" );
		Base::log( "shaderSimpleColor.prog = %d\n", shaderSimpleColor.prog );
	}
	CHECK_GL_ERROR;

	{
		shaderIcon.vs = glCreateShader(GL_VERTEX_SHADER);
		shaderIcon.fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource( shaderIcon.vs, 1, ((const char**)&icons_vsdata), NULL );
		glShaderSource( shaderIcon.fs, 1, ((const char**)&icons_fsdata), NULL );
		glCompileShader( shaderIcon.vs );
		glCompileShader( shaderIcon.fs );
		shaderIcon.prog = glCreateProgram();
		glAttachShader(shaderIcon.prog, shaderIcon.fs);
		glAttachShader(shaderIcon.prog, shaderIcon.vs);
		glLinkProgram(shaderIcon.prog);
		printShaderInfoLog( shaderIcon.vs );
		printShaderInfoLog( shaderIcon.fs );
		printProgramInfoLog( shaderIcon.prog );
		shaderIcon.wvp = glGetUniformLocation( shaderIcon.prog, "WORLD_VIEW_PROJECTION" );
		shaderIcon.texture = glGetUniformLocation( shaderIcon.prog, "texture" );
		shaderIcon.alpha = glGetUniformLocation( shaderIcon.prog, "alpha" );
		Base::log( "shaderIcon.prog = %d\n", shaderIcon.prog );

	}
	CHECK_GL_ERROR;

	{
		shaderShadowGenerate.vs = glCreateShader(GL_VERTEX_SHADER);
		shaderShadowGenerate.fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource( shaderShadowGenerate.vs, 1, ((const char**)&shadow_generate_vsdata), NULL );
		glShaderSource( shaderShadowGenerate.fs, 1, ((const char**)&shadow_generate_fsdata), NULL );
		glCompileShader( shaderShadowGenerate.vs );
		glCompileShader( shaderShadowGenerate.fs );
		shaderShadowGenerate.prog = glCreateProgram();
		glAttachShader(shaderShadowGenerate.prog, shaderShadowGenerate.fs);
		glAttachShader(shaderShadowGenerate.prog, shaderShadowGenerate.vs);
		glLinkProgram(shaderShadowGenerate.prog);
		printShaderInfoLog( shaderShadowGenerate.vs );
		printShaderInfoLog( shaderShadowGenerate.fs );
		printProgramInfoLog( shaderShadowGenerate.prog );
		shaderShadowGenerate.wvp_light = glGetUniformLocation( shaderShadowGenerate.prog, "WORLD_VIEW_PROJECTION_LIGHT" );
		Base::log( "shaderShadowGenerate.prog = %d\n", shaderShadowGenerate.prog );
	}

	{
		shaderShadowRender.vs = glCreateShader(GL_VERTEX_SHADER);
		shaderShadowRender.fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource( shaderShadowRender.vs, 1, ((const char**)&shadow_render_vsdata), NULL );
		glShaderSource( shaderShadowRender.fs, 1, ((const char**)&shadow_render_fsdata), NULL );
		glCompileShader( shaderShadowRender.vs );
		glCompileShader( shaderShadowRender.fs );
		shaderShadowRender.prog = glCreateProgram();
		glAttachShader(shaderShadowRender.prog, shaderShadowRender.fs);
		glAttachShader(shaderShadowRender.prog, shaderShadowRender.vs);
		glLinkProgram(shaderShadowRender.prog);
		printShaderInfoLog( shaderShadowRender.vs );
		printShaderInfoLog( shaderShadowRender.fs );
		printProgramInfoLog( shaderShadowRender.prog );
		shaderShadowRender.wvp = glGetUniformLocation( shaderShadowRender.prog, "WORLD_VIEW_PROJECTION" );
		shaderShadowRender.reload_shadow_fragment();
		Base::log( "shaderShadowRender.prog = %d\n", shaderShadowRender.prog );
		glUseProgram( shaderShadowRender.prog );
		glUniform1f( shaderShadowRender.shadowmap_width, shadowmapWidth );
		glUniform1f( shaderShadowRender.shadowmap_height, shadowmapHeight );
	}
	CHECK_GL_ERROR;

	{
		shaderTexture.vs = glCreateShader(GL_VERTEX_SHADER);
		shaderTexture.fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource( shaderTexture.vs, 1, ((const char**)&texture_vsdata), NULL );
		glShaderSource( shaderTexture.fs, 1, ((const char**)&texture_fsdata), NULL );
		glCompileShader( shaderTexture.vs );
		glCompileShader( shaderTexture.fs );
		shaderTexture.prog = glCreateProgram();
		glAttachShader(shaderTexture.prog, shaderTexture.fs);
		glAttachShader(shaderTexture.prog, shaderTexture.vs);
		glLinkProgram(shaderTexture.prog);
		printShaderInfoLog( shaderTexture.vs );
		printShaderInfoLog( shaderTexture.fs );
		printProgramInfoLog( shaderTexture.prog );
		shaderTexture.wvp = glGetUniformLocation( shaderTexture.prog, "WORLD_VIEW_PROJECTION" );
		shaderTexture.texture = glGetUniformLocation( shaderTexture.prog, "texture" );
		shaderTexture.texture_offset = glGetUniformLocation( shaderTexture.prog, "texture_offset" );
		shaderTexture.texture_scale = glGetUniformLocation( shaderTexture.prog, "texture_scale" );
		shaderTexture.reload_shadow_fragment();
		Base::log( "shaderTexture.prog = %d\n", shaderTexture.prog );

	}

	CHECK_GL_ERROR;

	free(marble_vsdata);
	free(marble_fsdata);
	free(simple_vsdata);
	free(simple_fsdata);
	free(texture_vsdata);
	free(texture_fsdata);
	free(shadow_generate_vsdata);
	free(shadow_generate_fsdata);
	free(shadow_render_vsdata);
	free(shadow_render_fsdata);
	free(icons_vsdata);
	free(icons_fsdata);

	if ( shaderMarble.use_3d_texture )
	{
		shaderMarble.volume_texture_data = textureNoise;
	}
	shaderNormal.texture_data_ddn = textureBricksDdn;
	shaderNormal.texture_data_diff = textureBricksDiff;
	glUseProgram( shaderShadowRender.prog );
	glUniform1f( shaderShadowRender.shadowmap_width, shadowmapWidth );
	glUniform1f( shaderShadowRender.shadowmap_height, shadowmapHeight );
	glUseProgram( shaderTexture.prog );
	glUniform1f( shaderTexture.shadowmap_width, shadowmapWidth );
	glUniform1f( shaderTexture.shadowmap_height, shadowmapHeight );
	glUseProgram( shaderMarble.prog );
	glUniform1f( shaderMarble.shadowmap_width, shadowmapWidth );
	glUniform1f( shaderMarble.shadowmap_height, shadowmapHeight );
	glUseProgram( shaderNormal.prog );
	glUniform1f( shaderNormal.shadowmap_width, shadowmapWidth );
	glUniform1f( shaderNormal.shadowmap_height, shadowmapHeight );

	release_mutex( mtxRender );
}

void destroyApp( bool save_the_game )
{
	if ( save_the_game )
	{
		Base::log("saveXMLChessScene");
#if PLATFORM_ARM_ANDROID
		saveXMLChessScene( APPDIRSTR("save.xml"), mainScene, xmldocument );
#else
		saveXMLChessScene( PATHSTR( "xmldata", "save.xml"), mainScene, xmldocument );
#endif
	}

	freeVertexDeclaration( &vdIconResetGame );
	memset(&vdIconResetGame, 0, sizeof(vdIconResetGame));
	freeVertexDeclaration( &vdIconCheckmate );
	memset(&vdIconCheckmate, 0, sizeof(vdIconCheckmate));
	freeVertexDeclaration( &vdIconCheck );
	memset(&vdIconCheck, 0, sizeof(vdIconCheck));
	freeVertexDeclaration( &vdIconChessBusy );
	memset(&vdIconChessBusy, 0, sizeof(vdIconChessBusy));
	freeVertexDeclaration( &vdSphere );
	memset(&vdSphere, 0, sizeof(vdSphere));
	freeVertexDeclaration( &vdBox );
	memset(&vdBox, 0, sizeof(vdBox));
	freeVertexDeclaration( &vdHalfDome );
	memset(&vdHalfDome, 0, sizeof(vdHalfDome));
	glDeleteTextures( 1, &textureIcons.id );
	glDeleteTextures( 1, &textureGalaxy.id );
	glDeleteTextures( 1, &textureBricksDiff.id );
	glDeleteTextures( 1, &textureBricksDdn.id );

	if ( shaderMarble.use_3d_texture )
	{
		glDeleteTextures( 1, &textureNoise.id );
		destroyImage( &textureNoise );
		memset(&textureNoise, 0, sizeof(textureNoise));
	}

	destroyImage( &textureIcons );
	memset(&textureIcons, 0, sizeof(textureIcons));
	destroyImage( &textureGalaxy );
	memset(&textureGalaxy, 0, sizeof(textureGalaxy));
	destroyImage( &textureBricksDiff );
	memset(&textureBricksDiff, 0, sizeof(textureBricksDiff));
	destroyImage( &textureBricksDdn );
	memset(&textureBricksDdn, 0, sizeof(textureBricksDdn));

	for ( int i = 0; i < mainScene->geometryinstances.size(); i++ )
	{
		free( mainScene->geometryinstances[i] );
	}
	mainScene->geometryinstances.clear();
	for ( int i = 0; i < mainScene->geometries.size(); i++ )
	{
		mainScene->geometries[i]->destroy3ds();
		free( mainScene->geometries[i] );
	}
	mainScene->geometries.clear();
	for ( int i = 0; i < mainScene->transforms.size(); i++ )
	{
		free( mainScene->transforms[i] );
	}
	mainScene->transforms.clear();
	free( mainScene );
	mainScene = NULL;

	shaderIcon.destroy();
	shaderMarble.destroy();
	shaderNormal.destroy();
	shaderShadowGenerate.destroy();
	shaderShadowRender.destroy();
	shaderSimpleColor.destroy();
	shaderSky.destroy();
	shaderTexture.destroy();
}

void initApp( bool use_save )
{
	float bgColors[] = { 
		1.10f, 1.10f, 1.f, 
		1.25f, 1.25f, 1.f, 
		0.70f, 0.70f, 1.f, 
		0.90f, 0.90f, 1.f 
	};

	init_chess_thread();

	xmldocument = new tinyxml2::XMLDocument();
#ifdef PLATFORM_ARM_ANDROID
	use_save = (use_save && access( APPDIRSTR("save.xml"), F_OK ) != -1 );

	if (use_save)
	{
		mainScene = readXMLChessScene( APPDIRSTR("save.xml"), xmldocument, mainScene );
	}
	else
	{
		mainScene = readXMLChessScene( PATHSTR( "xmldata", "chessboard_layout8_android.xml"), xmldocument, mainScene );
	} 
#else
	use_save = (use_save && access( PATHSTR( "xmldata", "save.xml" ), 6 ) != -1 );

	if (use_save)
	{
		mainScene = readXMLChessScene( PATHSTR( "xmldata", "save.xml"), xmldocument, mainScene );
		mainScene = readXMLChessScene( PATHSTR( "xmldata", "save.xml"), xmldocument, mainScene );
	}
	else
	{
		mainScene = readXMLChessScene( PATHSTR( "xmldata", "chessboard_layout8.xml"), xmldocument, mainScene );
	}
#endif
	if ( !mainScene)
	{
		Base::log("Mainscene not loaded. aborting");
		assert(0);
	}	
	/* Update shader linkage; fixed */
	for ( int i = 0; i < mainScene->geometryinstances.size(); i++ )
	{
		EE_GeometryInstance *g = mainScene->geometryinstances[i];
		g->shmarble = &shaderMarble;
		g->shaderSimpleColor = &shaderSimpleColor;
		g->shnormal = &shaderNormal;
		g->shshadow_generate = &shaderShadowGenerate;
		g->shshadow_render = &shaderShadowRender;
		g->shtexture = &shaderTexture;
	}

	get_camera()->init();

	glGenFramebuffers(1, &mainScene->FramebufferName);
	CHECK_GL_ERROR;
	glGenTextures(1, &mainScene->depthTexture);
	CHECK_GL_ERROR;

	glBindTexture(GL_TEXTURE_2D, mainScene->depthTexture);
	CHECK_GL_ERROR;

	for (int i = 0; i < SIZE_OF_ARRAY( depth_preferences ); i++ )
	{
		glTexImage2D(GL_TEXTURE_2D, 0, depth_preferences[i].format, shadowmapWidth, shadowmapHeight, 0,GL_DEPTH_COMPONENT, depth_preferences[i].type, 0);
		if ( glGetError() == GL_NO_ERROR )
		{
			depth_preferred = depth_preferences[i];
			break;
		}
	}
	CHECK_GL_ERROR;

	if ( depth_preferred.type == GL_NONE )
	{
		Base::log( "Error! Could not find appropriate depth format\n" );
	}
	else
	{
		Base::log( "Chose depth format 0x%X type: 0x%X\n", depth_preferred.format, depth_preferred.type );
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	CHECK_GL_ERROR;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	CHECK_GL_ERROR;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	CHECK_GL_ERROR;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	CHECK_GL_ERROR;

	// The camera position, target and up vector for use when creating the view matrix
	myvec3 camTarget = myvec3(0.0f, 0.0f, 0.0f);
	for ( int i = 0; i < mainScene->geometryinstances.size(); i++ )
	{
		if ( 0 == strncmp( mainScene->geometryinstances[i]->getName(), "kitchen_floor.X_instance0", 25 ) )
		{
			get_camera()->setLook( mainScene->geometryinstances[i]->matrix * mainScene->geometryinstances[i]->geometry->bbc );
			break;
		}
	}


	// Set up the world matrix for the object.
	mymat4 world_sphere_init = mymat4::scale(100.0,100.0,100.0);
	float scale_inv_sphere = 1.0/100.0;

	mymat4 wvpMatrix_sphere;
	mymat4 projQuadA = mymat4::identity();
	mymat4 world_quadA_init = mymat4::identity();
	mymat4 world_quadA;
	mymat4 wvpMatrix_quadA;

	if (textureGalaxy.data == 0)
	{
		loadCubeTexPNG( PATHSTR( "data","Galaxy"), &textureGalaxy );
		CHECK_GL_ERROR;
	}

	if (textureIcons.data == 0)
	{
		loadQuadTexPNG( PATHSTR( "data","icons_new.png"), &textureIcons );
		CHECK_GL_ERROR;
	}

	if (textureBricksDdn.data == 0)
	{
		loadQuadTexPNG( PATHSTR( "textures", "oldcheckboard_normals.png"), &textureBricksDdn );
		textureBricksDdn.wrap_s = GL_REPEAT;
		textureBricksDdn.wrap_t = GL_REPEAT;
		textureBricksDdn.filter_mag = GL_LINEAR;
		textureBricksDdn.filter_min = GL_LINEAR;
		CHECK_GL_ERROR;
	}

	if (textureBricksDiff.data == 0)
	{
		loadQuadTexPNG( PATHSTR( "textures", "OLDCHECK.JPG"), &textureBricksDiff );
		textureBricksDiff.wrap_s = GL_REPEAT;
		textureBricksDiff.wrap_t = GL_REPEAT;
		textureBricksDiff.filter_mag = GL_LINEAR;
		textureBricksDiff.filter_min = GL_LINEAR;
		CHECK_GL_ERROR;
	}
#ifdef GL_OES_texture_3D
	shaderMarble.use_3d_texture = gl_supports( "GL_OES_texture_3D" );
#else 
	shaderMarble.use_3d_texture = false;
#endif

	if (shaderMarble.use_3d_texture && textureNoise.data == 0)
	{
		Base::log("Loading noiseL8_32x32_slices\n" );
		loadQuadVolumeTexPNG( PATHSTR( "textures", "noiseL8_32x32_slice"), 32, &textureNoise );
		textureNoise.wrap_s = GL_REPEAT;
		textureNoise.wrap_t = GL_REPEAT;
		textureNoise.wrap_u = GL_REPEAT;
		textureNoise.filter_mag = GL_LINEAR;
		textureNoise.filter_min = GL_LINEAR;
		CHECK_GL_ERROR;
	}

	if (vdIconResetGame.velems == 0) {
		createQuadPositionTexcoordVertexDecl( &vdIconResetGame, 0.f, 49.f, 49.f, 0.f, 50.f/100.f, 50.f/300.f, 100.f/100.f, 100.f/300.f );
	}
	if (vdIconCheckmate.velems == 0) {
		createQuadPositionTexcoordVertexDecl( &vdIconCheckmate, 0.f, 49.f, 49.f, 0.f, 50.f/100.f, 150.f/300.f, 100.f/100.f, 200.f/300.f );
	}
	if (vdIconCheck.velems == 0) {
		createQuadPositionTexcoordVertexDecl( &vdIconCheck, 0.f, 49.f, 49.f, 0.f, 50.f/100.f, 100.f/300.f, 100.f/100.f, 150.f/300.f );
	}
	if (vdIconChessBusy.velems == 0) {
		createQuadPositionTexcoordVertexDecl( &vdIconChessBusy, 0.f, 49.f, 49.f, 0.f, 50.f/100.f, 200.f/300.f, 100.f/100.f, 250.f/300.f );
	}
	if (vdHalfDome.velems == 0) {
		createHalfDomePositionTexcoordVertexDecl( &vdHalfDome, 40,40, 1.f );
	}
	if (vdSphere.velems == 0) {
		createSpherePositionNormalVertexDecl( &vdSphere, 10, 10, 1.f );
	}
	if (vdBox.velems == 0) {
		createBoxNormalPositionVertexDecl( &vdBox, 1.0f );
	}
	CHECK_GL_ERROR;

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	glClearColor(0.0,0.0,0.0,1);

	if ( false == reinit_chess_state( mainScene->geometryinstances ) )
	{
		mtxRender = create_mutex("mtxRender");
	}

	reloadShaders();
	prevTimestamp = timer_get_time();
	terminating = false;

	if (player_can_move()) {
		get_scene()->camera->setOrbitTo(white_can_move());
		get_scene()->camera->orbitToHome();
	}
	
}

/* Window handling */
void cbResize( int width, int height )
{
	windowWidth = width;
	windowHeight = height;

#if 1 == OPTIMIZATIONS
	shadowmapWidth = width/2;
	shadowmapHeight = height/2;
#else
	shadowmapWidth = width;
	shadowmapHeight = height;
#endif
	glBindTexture(GL_TEXTURE_2D, mainScene->depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, depth_preferred.format, shadowmapWidth, shadowmapHeight, 0,GL_DEPTH_COMPONENT, depth_preferred.type, 0);
	CHECK_GL_ERROR;
	assert(shaderShadowRender.prog);
	glUseProgram( shaderShadowRender.prog );
	glUniform1f( shaderShadowRender.shadowmap_width, shadowmapWidth );
	glUniform1f( shaderShadowRender.shadowmap_height, shadowmapHeight );
	glUseProgram( shaderTexture.prog );
	glUniform1f( shaderTexture.shadowmap_width, shadowmapWidth );
	glUniform1f( shaderTexture.shadowmap_height, shadowmapHeight );
	glUseProgram( shaderMarble.prog );
	glUniform1f( shaderMarble.shadowmap_width, shadowmapWidth );
	glUniform1f( shaderMarble.shadowmap_height, shadowmapHeight );
	CHECK_GL_ERROR;
	get_camera()->resize(windowWidth, windowHeight);
	get_camera()->build_orbit_view();
	glViewport(0,0,windowWidth, windowHeight);
	CHECK_GL_ERROR;
}


void TestGLSL::onPause()
{
	animating = false;
}

void TestGLSL::onResume()
{
	animating = true;
	prevTimestamp = timer_get_time();
}

void TestGLSL::onSaveState()
{
	Base::log("saveXMLChessScene");
#if PLATFORM_ARM_ANDROID
	saveXMLChessScene( APPDIRSTR("save.xml"), mainScene, xmldocument );
#else
	saveXMLChessScene( PATHSTR( "xmldata", "save.xml"), mainScene, xmldocument );
#endif

}

#if defined ( PLATFORM_ARM_ANDROID )

void TestGLSL::onDestroy() 
{
	EGLBoolean status = EGL_FALSE;
	// Make sure to free all references to surface as we might on next onCreate get
	// native_window_api_connect failed (already connected to another API?) otherwise.
    status = eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    assert( EGL_TRUE == status );//, "eglMakeCurrent" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglMakeCurrent" );
	
    status = eglDestroyContext( display, context );
    assert( EGL_TRUE == status );//, "eglDestroyContext" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglDestroyContext" );

    status = eglDestroySurface( display, surface );
    assert( EGL_TRUE == status );//, "eglDestroySurface" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglDestroySurface" );

    status = eglTerminate( display );
    assert( EGL_TRUE == status );//, "eglTerminate" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglTerminate" );

    if ( NULL != configs ) free( configs );

	grab_mutex(mtxRender);
	terminating = true;
	release_mutex(mtxRender);
	destroyApp(true);

	detach_main_thread();
};

void TestGLSL::onCreate( int argc, char **argv, void *native_window ) 
{
#define MAX_CONFIGS 200
    EGLNativeWindowType win = NULL;
    EGLBoolean status = EGL_FALSE;
    EGLint num_config = 0;
    EGLint egl_error = 0;
    EGLint parameter[] =
        {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };
    int i;

    EGLint attribs_context[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    win = ( EGLNativeWindowType )native_window;
	Base::log( "onCreate\n"  );

    display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    assert( EGL_NO_DISPLAY != display );//, "eglGetDisplay" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglGetDisplay" );
	Base::log( "eglGetDisplay\n"  );

    status = eglInitialize( display, NULL, NULL );
    assert( EGL_TRUE == status );//, "eglInitialise" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglInitialise" );
	Base::log( "eglGetDisplay\n"  );

    configs = ( EGLConfig * ) malloc( sizeof( EGLConfig ) * MAX_CONFIGS );
    assert( NULL != configs );//, "malloc returned NULL" );

    status = eglChooseConfig( display, parameter, configs, MAX_CONFIGS, &num_config );
    assert( EGL_TRUE == status );//, "eglChooseConfig" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglChooseConfig" );
	assert( num_config > 0 );
	Base::log( "eglGetDisplay\n"  );

	for ( i = 0; i < num_config; i++ )
	{
		EGLint r,g,b,a,d;
		eglGetConfigAttrib( display, configs[i], EGL_RED_SIZE, &r );
		eglGetConfigAttrib( display, configs[i], EGL_GREEN_SIZE, &g );
		eglGetConfigAttrib( display, configs[i], EGL_BLUE_SIZE, &b );
		eglGetConfigAttrib( display, configs[i], EGL_ALPHA_SIZE, &a );
		eglGetConfigAttrib( display, configs[i], EGL_DEPTH_SIZE, &d );
		if ( r == 8 && g == 8 && b == 8 && a == 8 && d > 0 ) break;
	}

	if ( i == num_config ) 
	{
		Base::log( "Failed to find an rgba=8888 config with depth\n"  );
		return;
	}

    context = eglCreateContext( display, configs[i], EGL_NO_CONTEXT, attribs_context );
    assert( EGL_NO_CONTEXT != context );//, "eglCreateContext" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglCreateContext" );
	Base::log( "eglCreateContext\n"  );
    surface = eglCreateWindowSurface( display, configs[i], win, NULL );
    assert( EGL_NO_SURFACE != surface );//, "eglCreateWindowSurface() returned EGL_NO_SURFACE" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglCreateWindowSurface() set an error %s", egl_error_string( egl_error ) );
	Base::log( "eglCreateWindowSurfaceFailed to find an rgba=8888 config with depth\n"  );
    status = eglMakeCurrent( display, surface, surface, context );
    assert( EGL_TRUE == status );//, "eglMakeCurrent" );
    assert( EGL_SUCCESS == eglGetError() );//, "eglMakeCurrent" );
	Base::log( "eglMakeCurrentFailed to find an rgba=8888 config with depth\n"  );
	init_shader_functions();
	Base::log( "initAppFailed to find an rgba=8888 config with depth\n"  );
	initApp(true);
#undef MAX_CONFIGS
};

void TestGLSL::onResize( int w, int h )
{
	cbResize(w,h);
}


void TestGLSL::onKeyDown( int key )
{
}

void TestGLSL::onKeyUp( int key )
{
}

#else

void TestGLSL::onCreate( int argc, char **argv, void *native_window ) 
{ 
	// Initializing
	glutInit( &argc, argv );
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(windowWidth,windowHeight);
	glutCreateWindow("ETChess3D");

	init_shader_functions();
	initApp(true);
}

void TestGLSL::onDestroy() 
{
	grab_mutex(mtxRender);
	terminating = true;
	release_mutex(mtxRender);
	destroyApp(true);
}

void TestGLSL::onResize(int w, int h)
{
}

void cbKeyboard (unsigned char key, int x, int y) 
{
	switch( key )
	{
	case 't':
		grab_mutex(mtxRender);
		release_mutex(mtxRender);
		destroyApp(true);
		init_shader_functions();
		initApp(true);
		cbResize(windowWidth, windowHeight);
		break;

	case 'r':
		reloadShaders();
		break;
	case 'q': 
		grab_mutex(mtxRender);
		terminating = true;
		release_mutex(mtxRender);
		exit(0);
		break;
	case 'a':
		{
			static int src_blend_counter = 6;
			src_blend_counter = (src_blend_counter+1) % NELEMS( blend_modes );
			blend_src = blend_modes[src_blend_counter];
			break;
		}
	case 's':
		{
			static int dst_blend_counter = 7;
			dst_blend_counter = (dst_blend_counter+1) % (NELEMS( blend_modes )-1);
			blend_dst = blend_modes[dst_blend_counter];

			break;
		}
	default:
		break;
	}
}

void TestGLSL::onKeyDown( int key )
{
	cbKeyboard( key, 0, 0 );
}

void TestGLSL::onKeyUp( int key )
{
	cbKeyboard( key, 0, 0 );
}

void cbMouseMoving(int x, int y) 
{
	tglsl.onMove( x, y );
}

void cbMouseClick( int button, int state, int x, int y )
{
	switch ( button )
	{
	case GLUT_LEFT_BUTTON:
		if( state == GLUT_DOWN )
		{
			tglsl.onPointerDown( 0, x, y, false );
		} else if ( state == GLUT_UP )
		{
			tglsl.onPointerUp( 0, x, y, false );
		}
		break;
	case GLUT_RIGHT_BUTTON:
		if( state == GLUT_DOWN )
		{
			tglsl.onPointerDown( 0, x, y, true );
		} else if ( state == GLUT_UP )
		{
			tglsl.onPointerUp( 0, x, y, true );
		}
		break;

	default:
		break;
	}
}

void cbWinAtExit()
{
	release_chess_board();

	grab_mutex( mtxRender );
	terminating = true;
	release_mutex( mtxRender );

	destroyApp(true);
}

int main( int argc, char **argv )
{
	tglsl.onCreate( argc, argv, NULL );
	glutDisplayFunc( cbRender );
	glutIdleFunc( cbRender );
	glutReshapeFunc( cbResize );
	glutMotionFunc( cbMouseMoving ); 
	glutKeyboardFunc( cbKeyboard );
	glutMouseFunc( cbMouseClick );
	atexit(cbWinAtExit);
	glutMainLoop();
	tglsl.onDestroy();
}

#endif
