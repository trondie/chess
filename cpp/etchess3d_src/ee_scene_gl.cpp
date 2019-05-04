#include <lib3ds.h>
#include <stdio.h>
#include <stdlib.h>

#if defined( PLATFORM_WINDOWS )
#include <GL/glut.h>
#include <windows.h>
#endif

#include "ee_scene_gl.h"
#include "3ds_helpers.h"
#include "picking.h"
#include "main.h"
#include "shader_helper.h"
#include "chess_helpers.h"
#include "ee_camera.h"

EE_Node::EE_Node( const char *nodename, EENodeType nodetype, unsigned int nodeuid )
{ 
	name = (char*)malloc(strlen(nodename)+1);
	strcpy(name,nodename);
	type = nodetype; 
	uid = nodeuid;
	child = NULL;
	sibling = NULL;
}

EE_Node::~EE_Node()
{
	if ( name )
	{		
		free( name );
		name = NULL;
	}
}

const char *EE_Node::getName()
{
	return name;
}

void EE_Node::render( float deltatime, mymat4 *transform )
{
	/* To be overloaded */
}

EE_Geometry::EE_Geometry( const char *nodename, const char *fileurl, unsigned int uid ) : EE_Node( nodename, EE_NODE_GEOMETRY, uid )
{
	file3ds = NULL;		
	url = (char*)malloc( strlen( fileurl ) + 1 );
	strcpy( url, fileurl );
}

EE_Geometry::~EE_Geometry()
{
	if ( url )
	{
		free( url );
		url = NULL;
	}
}

bool EE_Geometry::load3ds()
{
	return read3DSFile( &file3ds, url, &bmin, &bmax, &bbs, &size, &bbc, true );
}

void EE_Geometry::destroy3ds()
{
	if (file3ds)
	{
		destroy3DSFile(file3ds);
	}
}

bool EE_Geometry::process3ds()
{			
	return process3DSFile( file3ds, &model_matrix );;
}

void EE_Geometry::render( float deltatime, mymat4 *transform, EE_GeometryInstance *instance )
{
	mymat4 cum = (*transform) * model_matrix;
	
	if ( file3ds ) 
	{		
		render3DSFile( file3ds, &cum, get_camera()->viewProj(), instance );
	}
}

bool EE_Geometry::pick( myvec3 &ray_dir, myvec3 &ray_point, mymat4 &ctm, myvec3 &position, float *intersection, bool bounding_only)
{
	if ( !file3ds ) return false;

#if BOUNDING_SPHERE
	float size = (start-end).length();
	intersection = intersectRaySphere( myvec3(ray_prime), myvec3(ray_start_point_prime), position,(size/2)*(size/2) );
	if( !(abs( intersection ) < PFAR) ) return false;
#else
	myvec3 start = ctm*bmin;
	myvec3 end = ctm*bmax;
	if ( false == intersectRayRectangle( ray_dir, ray_point, start, end, intersection ) ) return false;
#endif

	if ( bounding_only ) return true;

	for (Lib3dsNode *p = file3ds->nodes; p != 0; p = p->next) 
	{
		if ( true == pick3DSFileHelper( file3ds, p, ray_dir, ray_point, ctm, intersection ) ) return true;
	}

	return false;
}

EE_Transform::EE_Transform( const char *nodename, unsigned int uid, unsigned int parentuid ) : EE_Node( nodename, EE_NODE_TRANSFORM, uid ) 
{
	this->parentuid = parentuid; 
}

void EE_Transform::render( float deltatime, mymat4 *transform )
{		
	mymat4 ctm = matrix*(*transform) ;
	if ( sibling ) sibling->render(deltatime, transform);
	if ( child ) child->render(deltatime, &ctm);
}

EE_GeometryInstance::EE_GeometryInstance( const char *nodename, unsigned int nodegeometryuid, unsigned int nodeparentuid, unsigned int uid) : EE_Node( nodename, EE_NODE_GEOMETRY_INSTANCE, uid )
{
	geometryuid = nodegeometryuid; 
	parentuid = nodeparentuid; geometry = NULL;
	pickable = true; enabled = true;
	do_render_sphere = false;
	do_render_box = false;
	ctm = mymat4::identity();
}

EE_GeometryInstance *EE_GeometryInstance::clone()
{
	char * clone_name = (char*)malloc( strlen(name) + 6 );
	strcpy( clone_name, name );
	memcpy( &clone_name[strlen(name)], "queen\0", 6 );
	EE_GeometryInstance *cloned_instance = new EE_GeometryInstance( clone_name,  geometryuid, parentuid, uid);
	cloned_instance->shaderSimpleColor = shaderSimpleColor;
	cloned_instance->shmarble = shmarble;
	cloned_instance->shshadow_generate = shshadow_generate;
	cloned_instance->shshadow_render = shshadow_render;
	cloned_instance->shnormal = shnormal;
	cloned_instance->shtexture = shtexture;
	return cloned_instance;
}

void EE_GeometryInstance::render( float deltatime, mymat4 *transform )
{
	ctm = matrix * (*transform);
	if ( enabled )
	{
		if ( do_render_sphere ) render_sphere( this );
		if ( geometry ) 
		{
			if ( !get_scene()->shadow_generate && !get_scene()->shadow_render )
			{
				GLfloat color[4] = {1.0,0.0,0.5,1.0};
				myvec3 lightPos = get_scene()->getLight();
				myvec4 lightPosW = (myvec4( lightPos.x, lightPos.y, lightPos.z, 1.0 ));

				if ( is_chess_piece( name ) && chess_piece_is_white( name ))
				{
					color[0] = 0.0;
					color[1] = 1.0;
				}

				glUseProgram( shnormal->prog );
				setTexture( &shnormal->texture_data_ddn, 0 );
				glUniform1i( shnormal->texture_ddn, 0 );
				setTexture( &shnormal->texture_data_diff, 1 );
				glUniform1i( shnormal->texture_diff, 1 );
				glUniform3fv( shnormal->lightW, 1, lightPosW );
				glUniform3fv( shnormal->camposW , 1, (const GLfloat*)get_camera()->pos() );
				CHECK_GL_ERROR;
			}
			if ( get_scene()->shadow_generate )
			{
				 if ( shadow_occluder ) geometry->render( deltatime, &ctm, this );
			}
			else if ( get_scene()->shadow_render )
			{
				GLfloat marble_color[3] = { 0.25, 0.2488, 0.198962 };

				myvec3 lightPos = get_scene()->getLight();
				myvec4 lightPosW = (myvec4( lightPos.x, lightPos.y, lightPos.z, 1.0 ));
				if ( is_chess_piece( name ) && chess_piece_is_white( name ))
				{
					marble_color[0] = 1.0;
					marble_color[1] = 0.9788;
					marble_color[2] = 0.788962;
				}
				glUseProgram( shmarble->prog );
				glUniform3fv( shmarble->color, 1, marble_color );
				glUniform3fv( shmarble->lightW, 1, lightPosW );
				glUniform3fv( shmarble->camposW, 1, (const GLfloat*)get_camera()->pos() );

				if ( shadow_receiver ) geometry->render( deltatime, &ctm, this );
			}
			else geometry->render( deltatime, &ctm, this );
		}
		if ( do_render_box && !get_scene()->shadow_generate ) 
		{
			get_scene()->selected = this;
		}
	}
	if ( sibling ) sibling->render(deltatime, transform);
	if ( child ) child->render(deltatime, transform);
}

void EE_Scene::render( float deltatime, mymat4 *transform )
{
	if ( camera ) 
	{
		camera->render(deltatime, transform );
	}		
	if ( sibling ) 
	{
		sibling->render( deltatime, transform );
	}
	if ( child )
	{
		child->render( deltatime, transform );
	}
}

void EE_Scene::load()
{
	for( int i = 0; i < geometries.size(); i++ )
	{
		geometries[i]->load3ds();
		geometries[i]->process3ds();
	}
}

void EE_Scene::link()
{
	for ( int i = 0; i < geometryinstances.size(); i++ )
	{
		for ( int j = 0; j < geometries.size(); j++ )
		{
			if ( geometryinstances[i]->geometryuid == geometries[j]->uid ) 
			{
				geometryinstances[i]->geometry = geometries[j];
				break;
			}
		}
		for ( int j = 0; j < geometryinstances.size(); j++ )
		{
			if ( geometryinstances[i]->parentuid == geometryinstances[j]->uid )
			{
				EE_Node *n = geometryinstances[j];
				if ( n->child != NULL )
				{
					EE_Node *c = n->child;
					while ( c->sibling != NULL ) c = c->sibling;
					c->sibling = geometryinstances[i];
				}
				else
				{
					n->child = geometryinstances[i];
				}					
				break;
			}
		}
		if ( geometryinstances[i]->parentuid == camera->uid )
		{
			EE_Node *n = camera;
			if ( n->child != NULL )
			{
				EE_Node *c = n->child;
				while ( c->sibling != NULL ) c = c->sibling;
				c->sibling = geometryinstances[i];
			}
			else
			{
				n->child = geometryinstances[i];
			}
		}
		for ( int j = 0; j < transforms.size(); j++ )
		{
			if ( geometryinstances[i]->parentuid == transforms[j]->uid )
			{
				EE_Node *n = transforms[j];
				if ( n->child != NULL )
				{
					EE_Node *c = n->child;
					while ( c->sibling != NULL ) c = c->sibling;
					c->sibling = geometryinstances[i];
				}
				else
				{
					n->child = geometryinstances[i];
				}
				break;
			}
		}
	}
	for ( int i = 0; i < transforms.size(); i++ )
	{
		for ( int j = 0; j < transforms.size(); j++ )
		{
			if ( transforms[i]->parentuid == transforms[j]->uid )
			{
				EE_Node *n = transforms[j];
				if ( n->child != NULL )
				{
					EE_Node *c = n->child;
					while ( c->sibling != NULL ) c = c->sibling;
					c->sibling = transforms[i];
				}
				else
				{
					n->child = transforms[i];
				}
				break;
			}
		}
		for ( int j = 0; j < geometryinstances.size(); j++ )
		{
			if ( transforms[i]->parentuid == geometryinstances[j]->uid )
			{
				EE_Node *n = geometryinstances[j];
				if ( n->child != NULL )
				{
					EE_Node *c = n->child;
					while ( c->sibling != NULL ) c = c->sibling;
					c->sibling = transforms[i];
				}
				else
				{
					n->child = transforms[i];
				}
				break;
			}
		}
		if ( transforms[i]->parentuid == camera->uid )
		{
			EE_Node *n = camera;
			if ( n->child != NULL )
			{
				EE_Node *c = n->child;
				while ( c->sibling != NULL ) c = c->sibling;
				c->sibling = transforms[i];
			}
			else
			{
				n->child = transforms[i];
			}
			break;
		}
	}
}

myvec3 &EE_Scene::getLight()
{
	return light;
}

myvec3 &EE_Scene::getSnapToGrid()
{
	return snapToGrid;
}

EE_Scene::EE_Scene( const char *scenename, unsigned int uid ) : EE_Node( scenename, EE_NODE_SCENE, uid )
{
	camera = NULL;
	selected = NULL;
	snapToGrid = myvec3(0,0,0);
	light = myvec3(0,100,0);
}

void EE_Scene::moveObjectAlongCameraPlane( EE_Node *node, float deltaX, float deltaY, myvec3 &pickInitialPos )
{
	float width = get_camera()->viewWidth();
	myvec3 right, up;
	myvec3 node_pos = myvec3( node->matrix[12], node->matrix[13], node->matrix[14] );
	right = get_camera()->right();
	up = get_camera()->up();
	myvec3 diff = node_pos - (*get_camera()->pos());
	float dist = diff.length();
	float unitX = deltaX;//deltaX * dist  / float(windowWidth*0.4);
	float unitY = deltaY;//deltaY * dist  / float(windowHeight*0.4);
	right.x *= dist/float(width*0.30);
	right.y *= dist/float(width*0.26);
	up.x *= dist/float(width*0.30);
	up.y *= dist/float(width*0.26);
	pickInitialPos += right*unitX + up*unitY;

	if ( snapToGrid.x > 0 ) node->matrix[12] = ROUND( pickInitialPos.x / snapToGrid.x ) * snapToGrid.x;
	if ( snapToGrid.y > 0 ) node->matrix[13] = ROUND( pickInitialPos.y / snapToGrid.y ) * snapToGrid.y;
	if ( snapToGrid.z > 0 ) node->matrix[14] = ROUND( pickInitialPos.z / snapToGrid.z ) * snapToGrid.z;
}

EE_GeometryInstance *EE_Scene::pick( unsigned int x, unsigned int y )
{
	EE_GeometryInstance *pickedInstance = NULL;
	float centered_y = ( camera->viewHeight() - y ) - camera->viewHeight() / 2;
	float centered_x = ( x - camera->viewWidth() / 2 );
	float unit_x = centered_x / ( camera->viewWidth() / 2 );
	float unit_y = centered_y / ( camera->viewHeight() / 2 );

	float near_height = get_camera()->pnear() * float( tan( get_camera()->pfov()* M_PI / 360.0 ));

	myvec4 ray = myvec4( -unit_x*near_height*get_camera()->aspect(), -unit_y*near_height, 1, 0);
	myvec4 ray_start_point = myvec4( 0.f, 0.f, 0.f, 1.f );
	myvec4 ray_prime = (get_camera()->viewI())*ray;
	myvec4 ray_start_point_prime = (get_camera()->viewI())*ray_start_point;

	int i;
	float intersection = get_camera()->pfar() + 1.f;
	float closest_distance = get_camera()->pfar()+ 1.f;

	for ( i = 0; i < geometryinstances.size(); i++ )
	{
		if ( !geometryinstances[i]->enabled || geometryinstances[i]->geometry == NULL || !geometryinstances[i]->pickable ) continue;
		EE_GeometryInstance *instance = (EE_GeometryInstance *)geometryinstances[i];
		myvec3 position = myvec3( instance->ctm*myvec4( 0,0,0, 1) );
		myvec3 pick_ray = myvec3(myvec4(0,0,0,0)-ray_prime).normalized();
		myvec3 pick_pos = myvec3(ray_start_point_prime);
		if ( instance->geometry->pick( pick_ray, pick_pos, instance->ctm, position, &intersection, false ) )
		{
			/* Polygon intersection distance sort of works, but fails for some cases of the chess-board vs pieces */
			Base::log( "found instance %s at position:(%.2f,%.2f,%.2f) distance: %.2f\n", instance->getName(), position.x, position.y, position.z, intersection );
			if ( intersection < closest_distance && is_chess_piece( instance->getName() ))
			{
				pickedInstance = instance;
				closest_distance = intersection;
			}
		}
	}
	return pickedInstance;
}