#ifndef __EE_SCENE_GL_H_
#define __EE_SCENE_GL_H_

#include "math_helpers.h"
#include "simple_vector.h"
#include <tinyxml2.h>

struct Lib3dsFile;
class MyShader;
class MyShaderSimpleColor;
class MyShaderMarble;
class MyShaderSimpleTexture;
class MyShaderNormal;
class MyShaderShadowGenerate;
class MyShaderShadowRender;
class MyShaderTexture;
class EE_GeometryInstance;
class EE_Camera;

typedef enum
{
	EE_NODE_TRANSFORM,
	EE_NODE_CAMERA,
	EE_NODE_SCENE,
	EE_NODE_GEOMETRY,
	EE_NODE_GEOMETRY_INSTANCE
} EENodeType;

class EE_Node
{
protected:
	char * name;
	EENodeType type;

public:
	EE_Node *child;
	EE_Node *sibling;
	mymat4 matrix;
	unsigned int uid;
	
	const char *getName();
	EE_Node( const char *nodename, EENodeType nodetype, unsigned int nodeuid );
	virtual ~EE_Node();
	virtual void render( float deltatime, mymat4 *transform );
};

class EE_Geometry : public EE_Node
{
	struct Lib3dsFile *file3ds;
	char *url;
	MyShader *shader;

public:
	myvec3 bmin;
	myvec3 bmax;
	myvec3 bbs;
	myvec3 bbc;
	mymat4 model_matrix;
	float size;

	EE_Geometry( const char *nodename, const char *fileurl, unsigned int uid );
	virtual ~EE_Geometry();

	bool load3ds();
	bool process3ds();
	void destroy3ds();
	void render( float deltatime, mymat4 *transform, EE_GeometryInstance *instance );
	bool pick( myvec3 &ray_dir, myvec3 &ray_point, mymat4 &ctm, myvec3 &position, float *intersection, bool bounding_only);
};

class EE_Transform : public EE_Node
{
friend class EE_Scene;
	unsigned int parentuid;
public:
	EE_Transform( const char *nodename, unsigned int uid, unsigned int parentuid );
	virtual void render( float deltatime, mymat4 *transform );
};


class EE_GeometryInstance : public EE_Node
{
friend class EE_Scene;	
	unsigned int parentuid;
	unsigned int geometryuid;
public:
	tinyxml2::XMLElement* xmlelement;
	MyShaderSimpleColor *shaderSimpleColor;
	MyShaderMarble *shmarble;
	MyShaderTexture *shtexture;
	MyShaderNormal *shnormal;
	MyShaderShadowGenerate *shshadow_generate;
	MyShaderShadowRender *shshadow_render;
	EE_Geometry *geometry;
	bool enabled;
	bool pickable;
	bool do_render_sphere;
	bool do_render_box;
	bool shadow_receiver;
	bool shadow_occluder;
	mymat4 ctm;
	EE_GeometryInstance( const char *nodename, unsigned int nodegeometryuid, unsigned int nodeparentuid, unsigned int uid);
	virtual void render( float deltatime, mymat4 *transform );
	EE_GeometryInstance *clone();
};

class EE_Scene : public EE_Node
{
	myvec3 light;
	myvec3 snapToGrid;
public:
	EE_Camera *camera;
	EE_GeometryInstance *selected;

	vector<EE_Geometry*> geometries;
	vector<EE_GeometryInstance*> geometryinstances;
	vector<EE_Transform*> transforms;
	EE_Scene( const char *scenename, unsigned int uid );

	virtual void render( float deltatime, mymat4 *transform );
	void load();
	void link();
	myvec3 &getLight();
	myvec3 &getSnapToGrid();
	bool shadow_generate;
	bool shadow_render;
	unsigned int depthTexture;
	unsigned int FramebufferName;
	void moveObjectAlongCameraPlane( EE_Node *node, float deltaX, float deltaY, myvec3 &pickInitialPos );
	EE_GeometryInstance *pick( unsigned int x, unsigned int y );
};


#endif /* __EE_SCENE_GL_H_ */
