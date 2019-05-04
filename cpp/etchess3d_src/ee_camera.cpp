#include "ee_camera.h"
#include "ee_scene_gl.h"
#include "mymath.h"
#include "TestGLSL.h"
#include "main.h"

/* Light state */
myvec3 lightPos = myvec3( 170, -30, -260 );
mymat4 lightProjObjects;

/* Relative move state */
static float prevScaleLength = 0.0f;
static float prevUnitX = 0.f;
static float prevUnitY = 0.f;

float anim_func(float deltatime, void *parent_func)
{
	EE_Animator<float> *parent = (EE_Animator<float>*)parent_func;
	int keyframe = 0;
	int keyframe_next;
	for( int i = 0; i < parent->keyframes->size(); i++ )
	{
		if ( parent->current_time > (*parent->keyframes)[i] )
		{
			keyframe = i;
			continue;
		}
		else
		{
			break;
		}
	}
	if (keyframe == parent->keyframes->size() - 1)
	{
		keyframe_next = keyframe;
		parent->ended = true;
	}
	else
	{
		keyframe_next = keyframe + 1;
	}

	float distance = (*parent->keyframes)[keyframe_next] - (*parent->keyframes)[keyframe];
	if(distance == 0) distance = 1;
	float between = 1 - MAX(0.f, (*parent->keyframes)[keyframe_next] - parent->current_time) / distance;
	float return_value = lerp( between, (*parent->control_points)[keyframe], (*parent->control_points)[keyframe_next] );
	return return_value;
}

const mymat4 *EE_Camera::view()
{
	return &mView;
}

/* Scene world view projections */

mymat4 *get_lightViewProj()
{
	return &lightProjObjects;
}
const myvec3 *EE_Camera::pos()
{
	return &vPos;
}
const myvec3 *EE_Camera::dir()
{
	return &vDir;
}

void EE_Camera::build_orbit_view()
{
	vPos.x = orbitDistance*-sinf( orbitRotation*(M_PI/180.f) ) * cosf( orbitPitch*(M_PI/180.f)) + vLookPos.x;
	vPos.y = orbitDistance*cosf( orbitRotation*(M_PI/180.f) ) * cosf( orbitPitch*(M_PI/180.f)) + vLookPos.y;
	vPos.z = orbitDistance*-sinf( orbitPitch*(M_PI/180.f) ) + vLookPos.z;
	vDir = vPos.normalized();
	mView = mymat4::lookAt(vPos, vLookPos, myvec3(0,0,-1));
	viewDirty = true;
}

void EE_Camera::setLook( const myvec3 &look)
{
	vLookPos = look;
}

const myvec3 *EE_Camera::getLook()
{
	return &vLookPos;
}

static void generate_lightVP(EE_Camera *camera)
{
	mymat4 lightView = mymat4::lookAt( lightPos, *camera->getLook(), myvec3(0,0,-1));
	mymat4 lightProj = mymat4::perspective( camera->pfov(), camera->aspect(), camera->pnear(), camera->pfar());
	lightProjObjects = lightProj*lightView;
}

void EE_Camera::update( float deltaTime )
{
	bool buildOrbitView = orbitUpdate;
	if ( NULL != slide && !slide->stopped()) 
	{
		slide->update(deltaTime);
		orbitRotation = slide->get_value();
		buildOrbitView = true;
	}

	if ( buildOrbitView )
	{
		build_orbit_view();
		orbitUpdate = false;
	}
	generate_lightVP(this);
}

void EE_Camera::resize(int new_width, int new_height)
{
	view_width = new_width;
	view_height = new_height;
	mViewQuad = mymat4::identity();
	mProjectionPerspective = mymat4::perspective( fFov, aspect(), fNear, fFar);
	mProjectionOrthographic = mymat4::orthographic(view_width, view_height);
	projDirty = true;
}

myvec3 EE_Camera::right()
{
	return myvec3( mView.m[0], mView.m[1], mView.m[2] );
}

myvec3 EE_Camera::up()
{
	return myvec3( mView.m[4], mView.m[5], mView.m[6] );
}

myvec3 EE_Camera::forward()
{
	return myvec3( mView.m[8], mView.m[9], mView.m[10] );
}

void EE_Camera::undirty()
{
	if (viewDirty||projDirty)
	{
		mView.invert( mViewInvert );
		mViewProjPerspective = mProjectionPerspective*mView;
		mViewProjQuad =  mProjectionOrthographic*mViewQuad;
		viewDirty = false;
		projDirty = false;
	}
}

mymat4 EE_Camera::viewI()
{
	undirty();
	return mViewInvert;
}

const mymat4 *EE_Camera::viewQuad()
{
	undirty();
	return &mViewQuad;
}

void EE_Camera::orbitScale(myvec2 &first_press, myvec2 &second_press)
{
	myvec2 scale_vector = first_press - second_press;
	float speed = 30.f;
	float scale_vector_length = scale_vector.length() * speed /float(view_width/2.f);
	if( (scale_vector_length > 0.01f) && (prevScaleLength > 0.01f) )
	{
		float newradius = orbitDistance - logf(orbitDistance)*(prevScaleLength-scale_vector_length);
		if ( newradius > 5.f ) 
		{
			myvec3 newpos = vDir*newradius;
			if ( newpos.y > orbitPlaneY + cameraSphereSize )
			{
				orbitDistance = newradius;
				Base::log("prevScaleLength: %f new_scale_length:%f diff:%f (%d,%d,%d,%d)\n", prevScaleLength, scale_vector_length, prevScaleLength-scale_vector_length, first_press.x, first_press.y, second_press.x, second_press.y);
				orbitUpdate = true;
			}
		}
	}
	prevScaleLength = scale_vector_length;
}

void EE_Camera::orbitMove( int x, int y )
{
	//No chess pieces selected, assume pitch rotation 
	float speed = 50.f;
	float centered_y = ( view_height - y ) - view_height / 2;
	float centered_x = ( x - view_width / 2 );
	float unit_x = centered_x / ( view_width / 2 );
	float unit_y = centered_y / ( view_height / 2 );

	orbitPitch += (unit_y-prevUnitY)*speed;
	orbitPitch = MIN( 89.9f, MAX( 0.f, orbitPitch ) );
	orbitRotation += (prevUnitX-unit_x)*speed;
	orbitRotation = fmod( orbitRotation, 360.0f );
	prevUnitX = unit_x;
	prevUnitY = unit_y;
	orbitUpdate = true;
	orbitRotateHome = true;
}

void EE_Camera::setOrbitTo(bool to_white)
{
	orbit_to_white = to_white;
	orbitRotateHome = true;
}

void EE_Camera::orbitToHome()
{
	if (orbitRotateHome)
	{
		/* Slerp animate back */
		if( NULL == slide) slide = new EE_Animator<float>(float(0),anim_func);
		else slide->clear();
		vector<float> ctrlpts;
		vector<float> keyframes;
		ctrlpts.push_back(orbitRotation);
		if (orbit_to_white)
		{
			ctrlpts.push_back(-90.0);
		}
		else
		{
			ctrlpts.push_back(90.0);
		}
		keyframes.push_back(0);
		keyframes.push_back(0.5f);
		slide->set_control_points(&ctrlpts, &keyframes);
		//free(slide);
		orbitRotateHome = false;
	}
	
}

void EE_Camera::pointerUp( int x, int y )
{
	orbitToHome();
	prevUnitX = 0;
	prevUnitY = 0;
}

void EE_Camera::pointerDown( int x, int y )
{
	float centered_y = ( view_height - y ) - view_height / 2;
	float centered_x = ( x - view_width / 2 );
	prevUnitX = centered_x / ( view_width / 2 );
	prevUnitY = centered_y / ( view_height / 2 );
}

void EE_Camera::init()
{
	get_scene()->getLight() = lightPos;
	mView = mymat4::identity();
	mViewInvert = mymat4::identity();
	vLookPos = myvec3(0,0,0);
	projDirty = true;
	viewDirty = true;
	slide = NULL;
	orbitRotation = -90.0f;
	orbitDistance = 300.f;
	orbitPitch = 45.0f;
	orbitPlaneY = -75.0f;
	cameraSphereSize = 4.0f;
	fNear = 1.0f;
	fFar = 1000.0f;
	fFov = 90.0f;
	orbit_to_white = true;
}

const mymat4 *EE_Camera::viewProj()
{
	undirty();
	return &mViewProjPerspective;
}

const mymat4 *EE_Camera::proj()
{
	undirty();
	return &mProjectionPerspective;
}

const mymat4 *EE_Camera::viewProjQuad()
{
	undirty();
	return &mViewProjQuad;
}

EE_Camera::EE_Camera( const char *nodename, unsigned int uid ) : EE_Node( nodename, EE_NODE_CAMERA, uid )
{
	orbitUpdate = false;
	orbitRotateHome = false;
}

void EE_Camera::render( float deltatime, mymat4 *transform )
{
	if ( sibling ) sibling->render( deltatime, transform );
	if ( child ) child->render( deltatime, transform );
}

EE_Camera *get_camera()
{
	return get_scene()->camera;
}

float EE_Camera::aspect()
{
	return view_width/float(view_height);
}

float EE_Camera::pnear()
{
	return fNear;
}

float EE_Camera::pfar()
{
	return fFar;
}

float EE_Camera::pfov()
{
	return fFov;
}

float EE_Camera::viewWidth()
{
	return view_width;
}

float EE_Camera::viewHeight()
{
	return view_height;
}
