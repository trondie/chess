#ifndef _EE_CAMERA_H_
#define _EE_CAMERA_H_


#include "ee_scene_gl.h"
#include "math_helpers.h"
#include "ee_animator.h"

class EE_Camera : public EE_Node
{
public:
	EE_Camera( const char *nodename, unsigned int uid );
	virtual void render( float deltatime, mymat4 *transform );
	const myvec3 *pos();
	const mymat4 *proj();
	const mymat4 *viewProj();
	const mymat4 *viewProjQuad();
	const mymat4 *viewQuad();
	const mymat4 *view();
	float aspect();
	float pnear();
	float pfar();
	float pfov();
	void update(float deltatime);
	mymat4 viewI();
	myvec3 right();
	myvec3 up();
	myvec3 forward();
	void resize(int new_width, int new_height);
	void orbitScale(myvec2 &first_press, myvec2 &second_press);
	void orbitMove( int x, int y );
	void orbitToHome();
	void setOrbitTo(bool to_white);
	void pointerUp( int x, int y );
	void pointerDown( int x, int y );
	void init();
	void build_orbit_view();
	void setLook(const myvec3 &look);
	const myvec3 *dir();
	const myvec3 *getLook();
	void undirty();
	float viewWidth();
	float viewHeight();

private:
	mymat4 mProjectionOrthographic;
	mymat4 mProjectionPerspective;
	mymat4 mViewProjQuad;
	mymat4 mViewProjPerspective;
	mymat4 mViewQuad;
	mymat4 mView;
	mymat4 mViewInvert;
	myvec3 vLookPos;
	myvec3 vPos;
	myvec3 vDir;
	int view_width;
	int view_height;
	bool orbitUpdate;
	bool orbitRotateHome;
	float orbitRotation;
	float orbitDistance;
	float orbitPitch;
	float orbitPlaneY;
	bool orbit_to_white;
	float cameraSphereSize;
	bool viewDirty;
	bool projDirty;
	EE_Animator<float> *slide; /* Camera lerp animation controller */
	float fNear;
	float fFar;
	float fFov;
};


EE_Camera *get_camera();
mymat4 *get_lightViewProj();

#endif /* _EE_CAMERA_H_ */