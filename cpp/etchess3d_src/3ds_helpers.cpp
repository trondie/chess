#include "3ds_helpers.h"
#include "texture_helper.h"
#include "vertex_declaration_helpers.h"
#include "ee_scene_gl.h"
#include "picking.h"
#include "main.h"
#include <lib3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include "os_helpers.h"
#include "ee_camera.h"

#if defined( PLATFORM_WINDOWS )
#include <GL/glut.h>
#include <windows.h>
#endif

static bool read3DSFileHelper( Lib3dsFile *file, Lib3dsNode *node )
{
	if ( !file ) return false;
	if ( !node ) return false;
    {
        Lib3dsNode *p;
        for (p = node->childs; p != 0; p = p->next) {
            if ( false == read3DSFileHelper(file, p) ) return false;
        }
    }
    if (node->type == LIB3DS_NODE_MESH_INSTANCE) {
        int index;
        Lib3dsMesh *mesh;
        Lib3dsMeshInstanceNode *n = (Lib3dsMeshInstanceNode*)node;

        if (strcmp(node->name, "$$$DUMMY") == 0) {
            return true;
        }

        index = lib3ds_file_mesh_by_name(file, n->instance_name);
        if (index < 0)
            index = lib3ds_file_mesh_by_name(file, node->name);
        if (index < 0) {
            return true;
        }
        mesh = file->meshes[index];
        
		if(!mesh) return false;

        if (!mesh->user_ptr)
		{
			MyVertexElement *elements;
			MyVertexDeclaration *avdecl = (MyVertexDeclaration*)malloc(sizeof(MyVertexDeclaration));

			if ( mesh->texcos )
			{
				elements = (MyVertexElement*)malloc( sizeof(MyVertexElement)*5 );
				elements[0].components = 3;
				elements[0].offset = 0;
				elements[0].semantic = SPOSITION;
				elements[0].stream = 0;
				elements[0].type = GL_FLOAT;

				elements[1].components = 3;
				elements[1].offset = 12;
				elements[1].semantic = SNORMAL;
				elements[1].stream = 0;
				elements[1].type = GL_FLOAT;

				elements[2].components = 3;
				elements[2].offset = 24;
				elements[2].semantic = SBINORMAL;
				elements[2].stream = 0;
				elements[2].type = GL_FLOAT;

				elements[3].components = 3;
				elements[3].offset = 36;
				elements[3].semantic = STANGENT;
				elements[3].stream = 0;
				elements[3].type = GL_FLOAT;

				elements[4].components = 2;
				elements[4].offset = 48;
				elements[4].semantic = STEXCOORD0;
				elements[4].stream = 0;
				elements[4].type = GL_FLOAT;

				avdecl->velems = elements;
				avdecl->nvelems = 5;
				avdecl->elem_size = 0;
				for ( int i = 0; i < 5; i++ ) avdecl->elem_size += elements[i].components*sizeof(elements[i].type);
			}
			else
			{
				elements = (MyVertexElement*)malloc( sizeof(MyVertexElement)*2 );
				elements[0].components = 3;
				elements[0].offset = 0;
				elements[0].semantic = SPOSITION;
				elements[0].stream = 0;
				elements[0].type = GL_FLOAT;
				elements[1].components = 3;
				elements[1].offset = 12;
				elements[1].semantic = SNORMAL;
				elements[1].stream = 0;
				elements[1].type = GL_FLOAT;

				avdecl->velems = elements;
				avdecl->nvelems = 2;
				avdecl->elem_size = 0;
				for ( int i = 0; i < 2; i++ ) avdecl->elem_size += elements[i].components*sizeof(elements[i].type);
			}

			glGenBuffers( 1, &avdecl->vbo );   
			glGenBuffers( 1, &avdecl->ibo );   
			CHECK_GL_ERROR;

			mesh->user_ptr = avdecl;

            {
                int p;
				unsigned int normalDataSize = 3 * sizeof(float) * mesh->nvertices;
				unsigned int vertexDataSize = 3 * sizeof(float) * mesh->nvertices;
				unsigned int indexDataSize = 3 * sizeof(unsigned short int)*mesh->nfaces;
				unsigned int textureDataSize = 2 * sizeof(float) * mesh->nvertices;
				unsigned int binormalDataSize = 3 * sizeof(float) * mesh->nvertices;
				unsigned int tangentDatasize = 3 * sizeof(float) * mesh->nvertices;
				unsigned int dataSize;
				if ( mesh->texcos )
				{
					dataSize = 2*vertexDataSize+binormalDataSize+tangentDatasize+textureDataSize; /* since normaldatasize == vertexdatasize */
				}
				else
				{
					dataSize =  2*vertexDataSize; /* since normaldatasize == vertexdatasize */
				}

                /*float (*normalL)[3] = (float(*)[3])malloc(normalDataSize);*/
				unsigned short int *indexData = (unsigned short int*)malloc(indexDataSize);
				float *textureData = (float*)malloc(dataSize);

				/*lib3ds_mesh_calculate_vertex_normals(mesh, normalL);*/

				unsigned int ic = 0;
                for (p = 0; p < mesh->nfaces; ++p) 
				{
					indexData[ic++] = mesh->faces[p].index[0];
					indexData[ic++] = mesh->faces[p].index[1];
					indexData[ic++] = mesh->faces[p].index[2];
				}

				unsigned int vcan = 0;
				for( p = 0; p < mesh->nvertices; p++ )
				{
					textureData[vcan++] = mesh->vertices[p][0];
					textureData[vcan++] = mesh->vertices[p][1];
					textureData[vcan++] = mesh->vertices[p][2];					
					if ( mesh->texcos )
					{
						vcan += 9; /* skip normals, binormals and tangent. Done below */
						textureData[vcan++] = mesh->texcos[p][0];
						textureData[vcan++] = mesh->texcos[p][1];
					}
					else
					{
						vcan += 3; /* skip normals. Done below*/
					}
                }

				unsigned int comsize = (mesh->texcos ? 14 : 6);

#if 1
				/* Lengyel, Eric. �Computing Tangent Space Basis Vectors for an Arbitrary Mesh�. 
				 * Terathon Software 3D Graphics Library, 2001. http://www.terathon.com/code/tangent.html */
				myvec3 *tan1 = new myvec3[mesh->nvertices];
				myvec3 *tan2 = new myvec3[mesh->nvertices];
				myvec3 *normal = new myvec3[mesh->nvertices];
				memset( tan1, 0, sizeof( myvec3 ) * mesh->nvertices );
				memset( tan2, 0, sizeof( myvec3 ) * mesh->nvertices );
				memset( normal, 0, sizeof( myvec3 ) * mesh->nvertices );

				for( p = 0; p < mesh->nfaces; p++ )
				{
					myvec3 v1,v2,v3;
					unsigned short i1,i2,i3;

					i1 = mesh->faces[p].index[0];
					i2 = mesh->faces[p].index[1];
					i3 = mesh->faces[p].index[2];

					memcpy( &v1, mesh->vertices[i1], sizeof(float)*3 );
					memcpy( &v2, mesh->vertices[i2], sizeof(float)*3 );
					memcpy( &v3, mesh->vertices[i3], sizeof(float)*3 );

					if ( mesh->texcos )
					{
						myvec2 w1,w2,w3;

						memcpy( &w1, mesh->texcos[i1], sizeof(float)*2 );
						memcpy( &w2, mesh->texcos[i2], sizeof(float)*2 );
						memcpy( &w3, mesh->texcos[i3], sizeof(float)*2 );

						float x1 = v2.x - v1.x;
						float x2 = v3.x - v1.x;
						float y1 = v2.y - v1.y;
						float y2 = v3.y - v1.y;
						float z1 = v2.z - v1.z;
						float z2 = v3.z - v1.z;

						float s1 = w2.x - w1.x;
						float s2 = w3.x - w1.x;
						float t1 = w2.y - w1.y;
						float t2 = w3.y - w1.y;
						float dividend = (s1 * t2 - s2 * t1);
						float r = 1.0F / dividend == 0 ? 1 : dividend;
						myvec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
						myvec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
						tan1[i1] += sdir;
						tan1[i2] += sdir;
						tan1[i3] += sdir;
						tan2[i1] += tdir;
						tan2[i2] += tdir;
						tan2[i3] += tdir;
					}

					myvec3 nu = (v2 - v1);
					myvec3 nv = (v3 - v1);
					myvec3 nx = nv.crossProduct(nu);
					normal[i1] += nx;
					normal[i2] += nx;
					normal[i3] += nx;
				}

				for ( int v = 0; v < mesh->nvertices; v++ )
				{
					normal[v] = normal[v].normalized();
					memcpy(&textureData[3+comsize*v], &normal[v], sizeof(float)*3);

					if ( mesh->texcos )
					{
						myvec3 bitangent;
						myvec3 tangent = (tan1[v] - normal[v] * normal[v].dot(tan1[v]));
						tangent = tangent.normalized();
						float handedness = (normal[v].crossProduct(tan1[v]).dot(tan2[v]) < 0.0f) ? -1.0f : 1.0f;	
						bitangent = normal[v].crossProduct(tangent)*handedness;
						memcpy(&textureData[6+comsize*v], bitangent, sizeof(float)*3);
						memcpy(&textureData[9+comsize*v], tangent, sizeof(float)*3);
					}
				}

				free(tan1);
				free(tan2);
				free(normal);
#else
				for( p = 0; p < mesh->nfaces; p++ )
				{
					if ( mesh->texcos )
					{
						float x1 = textureData[0+comsize*(mesh->faces[p].index[1])] - textureData[0+comsize*(mesh->faces[p].index[0])];
						float x2 = textureData[0+comsize*(mesh->faces[p].index[2])] - textureData[0+comsize*(mesh->faces[p].index[0])];
						float y1 = textureData[1+comsize*(mesh->faces[p].index[1])] - textureData[1+comsize*(mesh->faces[p].index[0])];
						float y2 = textureData[1+comsize*(mesh->faces[p].index[2])] - textureData[1+comsize*(mesh->faces[p].index[0])];
						float z1 = textureData[2+comsize*(mesh->faces[p].index[1])] - textureData[2+comsize*(mesh->faces[p].index[0])];
						float z2 = textureData[2+comsize*(mesh->faces[p].index[2])] - textureData[2+comsize*(mesh->faces[p].index[0])];

						float s1 = textureData[12+comsize*(mesh->faces[p].index[1])] - textureData[12+comsize*(mesh->faces[p].index[0])];
						float s2 = textureData[12+comsize*(mesh->faces[p].index[2])] - textureData[12+comsize*(mesh->faces[p].index[0])];
						float t1 = textureData[13+comsize*(mesh->faces[p].index[1])] - textureData[13+comsize*(mesh->faces[p].index[0])];
						float t2 = textureData[13+comsize*(mesh->faces[p].index[2])] - textureData[13+comsize*(mesh->faces[p].index[0])];

						float dividend = (s1 * t2 - s2 * t1);
						float r = (dividend == 0) ? 0.0f : 1.0f / dividend;
						myvec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
								(t2 * z1 - t1 * z2) * r);
						myvec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
								(s1 * z2 - s2 * z1) * r);
						myvec3 tangent = (sdir + tdir);
						if (dividend != 0) tangent = tangent.normalized();
						for (int i = 0; i < 3; i++)
						{
							myvec3 normal = myvec3(normalL[p*3+i][0],normalL[p*3+i][1],normalL[p*3+i][2]);
							myvec3 binormal = tangent.crossProduct(normal);
							memcpy(&textureData[3+comsize*(mesh->faces[p].index[i])], &normal, sizeof(normal));
							memcpy(&textureData[6+comsize*(mesh->faces[p].index[i])], &binormal, sizeof(binormal));
							memcpy(&textureData[9+comsize*(mesh->faces[p].index[i])], &tangent, sizeof(tangent));
						}
					}
					else
					{
						textureData[3+comsize*(mesh->faces[p].index[0])] = normalL[p*3+0][0];
						textureData[4+comsize*(mesh->faces[p].index[0])] = normalL[p*3+0][1];
						textureData[5+comsize*(mesh->faces[p].index[0])] = normalL[p*3+0][2];
						textureData[3+comsize*(mesh->faces[p].index[1])] = normalL[p*3+1][0];
						textureData[4+comsize*(mesh->faces[p].index[1])] = normalL[p*3+1][1];
						textureData[5+comsize*(mesh->faces[p].index[1])] = normalL[p*3+1][2];
						textureData[3+comsize*(mesh->faces[p].index[2])] = normalL[p*3+2][0];
						textureData[4+comsize*(mesh->faces[p].index[2])] = normalL[p*3+2][1];
						textureData[5+comsize*(mesh->faces[p].index[2])] = normalL[p*3+2][2];
					}
				}
#endif

				//Then bind the vbo and ibo and store the vertex data and index data in OpenGL
				glBindBuffer( GL_ARRAY_BUFFER, avdecl->vbo );
				glBufferData( GL_ARRAY_BUFFER, dataSize, textureData, GL_DYNAMIC_DRAW );
				glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, avdecl->ibo );
				glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData, GL_DYNAMIC_DRAW );
				free(indexData);
				free(textureData);
            }
        }
    }
	return true;
}

bool read3DSFile( Lib3dsFile **file, const char *filename, 
	myvec3 *bmin , myvec3 *bmax, 
	myvec3 *bbs, float *size, /* bounding box dimensions */
	myvec3 *bbc, /* bounding box center */
	bool ignore_transform )
{
    *file = lib3ds_file_open(filename);

    if (!(*file)) {
        Base::log("read3DSFile %s failed.\n", filename);
        return false;
    }

    /* No nodes?  Fabricate nodes to display all the meshes. */
    if (!(*file)->nodes) {
        Lib3dsNode *node;
        int i;
        for (i = 0; i < (*file)->nmeshes; ++i) {
            Lib3dsMesh *mesh = (*file)->meshes[i];
            node = lib3ds_node_new(LIB3DS_NODE_MESH_INSTANCE);
            strcpy(node->name, mesh->name);
            lib3ds_file_insert_node((*file), node, NULL);
        }
    }

    if ( !ignore_transform ) lib3ds_file_eval((*file), 0.0f);

	lib3ds_file_bounding_box_of_nodes((*file), 1, 0, 0, bmin->coord, bmax->coord, NULL);
	bbs->x = bmax->x - bmin->x;
    bbs->y = bmax->y - bmin->y;
    bbs->z = bmax->z - bmin->z;
    *size = MAX(bbs->x, bbs->y);
    *size = MAX(*size, bbs->z);
    bbc->x = (bmin->x + bmax->x) / 2;
    bbc->y = (bmin->y + bmax->y) / 2;
    bbc->z = (bmin->z + bmax->z) / 2;

#if 0
    /* No cameras in the file?  Add four */

    if (!file->ncameras) {

        /* Add some cameras that encompass the bounding box */

        Lib3dsCamera *camera = lib3ds_camera_new("Camera_X");
        camera->target[0] = cx;
        camera->target[1] = cy;
        camera->target[2] = cz;
        memcpy(camera->position, camera->target, sizeof(camera->position));
        camera->position[0] = bmax[0] + 1.5 * MAX(sy, sz);
        camera->near_range = (camera->position[0] - bmax[0]) * .5;
        camera->far_range = (camera->position[0] - bmin[0]) * 2;
        lib3ds_file_insert_camera(file, camera, -1);

        /* Since lib3ds considers +Y to be into the screen, we'll put
        * this camera on the -Y axis, looking in the +Y direction.
        */
        camera = lib3ds_camera_new("Camera_Y");
        camera->target[0] = cx;
        camera->target[1] = cy;
        camera->target[2] = cz;
        memcpy(camera->position, camera->target, sizeof(camera->position));
        camera->position[1] = bmin[1] - 1.5 * MAX(sx, sz);
        camera->near_range = (bmin[1] - camera->position[1]) * .5;
        camera->far_range = (bmax[1] - camera->position[1]) * 2;
        lib3ds_file_insert_camera(file, camera, -1);

        camera = lib3ds_camera_new("Camera_Z");
        camera->target[0] = cx;
        camera->target[1] = cy;
        camera->target[2] = cz;
        memcpy(camera->position, camera->target, sizeof(camera->position));
        camera->position[2] = bmax[2] + 1.5 * MAX(sx, sy);
        camera->near_range = (camera->position[2] - bmax[2]) * .5;
        camera->far_range = (camera->position[2] - bmin[2]) * 2;
        lib3ds_file_insert_camera(file, camera, -1);

        camera = lib3ds_camera_new("Camera_ISO");
        camera->target[0] = cx;
        camera->target[1] = cy;
        camera->target[2] = cz;
        memcpy(camera->position, camera->target, sizeof(camera->position));
        camera->position[0] = bmax[0] + .75 * size;
        camera->position[1] = bmin[1] - .75 * size;
        camera->position[2] = bmax[2] + .75 * size;
        camera->near_range = (camera->position[0] - bmax[0]) * .5;
        camera->far_range = (camera->position[0] - bmin[0]) * 3;
        lib3ds_file_insert_camera(file, camera, -1);
    }

	/* No lights in the file?  Add some. */

    if (!file->nlights) {
        Lib3dsLight *light;

        light = lib3ds_light_new("light0");
        light->spot_light = 0;
        light->see_cone = 0;
        light->color[0] = light->color[1] = light->color[2] = .6;
        light->position[0] = cx + size * .75;
        light->position[1] = cy - size * 1.;
        light->position[2] = cz + size * 1.5;
        light->position[3] = 0.;
        light->outer_range = 100;
        light->inner_range = 10;
        light->multiplier = 1;
        lib3ds_file_insert_light(file, light, -1);

        light = lib3ds_light_new("light1");
        light->spot_light = 0;
        light->see_cone = 0;
        light->color[0] = light->color[1] = light->color[2] = .3;
        light->position[0] = cx - size;
        light->position[1] = cy - size;
        light->position[2] = cz + size * .75;
        light->position[3] = 0.;
        light->outer_range = 100;
        light->inner_range = 10;
        light->multiplier = 1;
        lib3ds_file_insert_light(file, light, -1);

        light = lib3ds_light_new("light2");
        light->spot_light = 0;
        light->see_cone = 0;
        light->color[0] = light->color[1] = light->color[2] = .3;
        light->position[0] = cx;
        light->position[1] = cy + size;
        light->position[2] = cz + size;
        light->position[3] = 0.;
        light->outer_range = 100;
        light->inner_range = 10;
        light->multiplier = 1;
        lib3ds_file_insert_light(file, light, -1);
    }
    camera = file->cameras[0]->name;

#endif


    for (int i = 0; i < (*file)->nmaterials; ++i) {
        Lib3dsMaterial *mat = (*file)->materials[i];
        if (mat->texture1_map.name[0]) {  /* texture map? */
            Lib3dsTextureMap *tex = &mat->texture1_map;
			MyTexture2D *user_texture = (MyTexture2D*)malloc(sizeof(MyTexture2D));
            char texname[1024];

            strcpy(texname, PATHSTR( "textures", "" ));
            strcat(texname, tex->name);
						
            Base::log("Loading %s\n", texname);
			char *extension = strrchr( texname, '.');
			if ( 0 == STRCMPI( ".png", extension ) )
			{
				if (!readPNGImage(texname, user_texture)) {
					free(user_texture);
					continue;
				}
			} else if ( 0 == STRCMPI( ".jpg", extension ) || 0 == STRCMPI( ".jpeg", extension ) )
			{
				if ( !readJPEGImage(texname, user_texture ) )
				{
					free(user_texture);
					continue;
				}
			} else if ( 0 == STRCMPI( ".p", extension ) )
			{
				char texname2[1024];
				strcpy( texname2, texname );
				strcat( texname2, "ng" );
				if (!readPNGImage(texname2, user_texture)) {
					free(user_texture);
					continue;
				}
			}
			else
			{
				Base::log("Could not load texture %s on model %s\n", texname, filename );
				free(user_texture);
				continue;
			}
			glGenTextures( 1, &user_texture->id );
			glBindTexture( GL_TEXTURE_2D, user_texture->id );
			glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
			if ( user_texture->Bpp == 4 )
			{
				glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,  user_texture->width, user_texture->height, GL_FALSE, GL_RGBA, GL_UNSIGNED_BYTE, user_texture->data );
			}
			else if ( user_texture->Bpp == 3 )
			{
				glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,  user_texture->width, user_texture->height, GL_FALSE, GL_RGB, GL_UNSIGNED_BYTE, user_texture->data );
			}
			else 
			{
				Base::log("Unknown texture depth %d for texture %s, model %s\n", user_texture->Bpp, texname, filename );
				continue;
			}

			user_texture->wrap_s = GL_REPEAT;
			user_texture->wrap_t = GL_REPEAT;
			user_texture->filter_min = GL_LINEAR;
			user_texture->filter_mag = GL_LINEAR;
			user_texture->target = GL_TEXTURE_2D;

			tex->user_ptr = (void*)user_texture;
        }
    }

	lib3ds_file_eval((*file), 0);

	CHECK_GL_ERROR;

	for (Lib3dsNode *p = (*file)->nodes; p != 0; p = p->next) 
	{
		if ( false == read3DSFileHelper( (*file), p ) )
		{
			Base::log( "Something went wrong while reading (%s)'s vertex, index and normal data into GL", filename );
		}
    }

	return true;
}


bool render3DSFileHelper( Lib3dsFile *file, Lib3dsNode *node, mymat4 *world_ctm,const mymat4 *viewproj, EE_GeometryInstance *geometry )
{
	if ( !file ) return false;
	if ( !node ) return false;
    {
        Lib3dsNode *p;
        for (p = node->childs; p != 0; p = p->next) {
            render3DSFileHelper(file, p, world_ctm, viewproj, geometry );
        }
    }
    if (node->type == LIB3DS_NODE_MESH_INSTANCE) {
        int index;
        Lib3dsMesh *mesh;
        Lib3dsMeshInstanceNode *n = (Lib3dsMeshInstanceNode*)node;

        if (strcmp(node->name, "$$$DUMMY") == 0) {
            return true;
        }

        index = lib3ds_file_mesh_by_name(file, n->instance_name);
        if (index < 0)
            index = lib3ds_file_mesh_by_name(file, node->name );
        if (index < 0) {
            return true;
        }
        mesh = file->meshes[index];
		if (!mesh ) return false;
#if 0
		mymat4 translation = mymat4::translation( -n->pivot[0], -n->pivot[1], -n->pivot[2] );
		mymat4 mr,rotation;
		memcpy( rotation.m, node->matrix, sizeof(float)*16 );
		/*mr.invert(rotation);
		world_ctm = mr*translation;*/
#endif
        if (mesh->user_ptr)
		{
            MyVertexDeclaration *avdecl = (MyVertexDeclaration*)mesh->user_ptr;
			mymat4 world_mesh = (*world_ctm);
			mymat4 wvp_mesh = (*viewproj) * world_mesh;
			mymat4 wvp_light = (*get_lightViewProj()) * world_mesh;

			Lib3dsMaterial *mat = file->materials ? file->materials[mesh->faces->material] : NULL;
			if ( get_scene()->shadow_generate )
			{
				glUseProgram( geometry->shshadow_generate->prog );
				glUniformMatrix4fv( geometry->shshadow_generate->wvp_light, 1, GL_FALSE, wvp_light );
				setVertexDeclaration( avdecl, geometry->shshadow_generate );
				glDrawElements( GL_TRIANGLES, mesh->nfaces*3, GL_UNSIGNED_SHORT, (void*)0 );
				unbindDeclaration( avdecl, geometry->shshadow_generate );
				CHECK_GL_ERROR;
			}
			else if ( get_scene()->shadow_render && 0 )
			{
				glUseProgram( geometry->shshadow_render->prog );
				glUniformMatrix4fv( geometry->shshadow_render->wvp, 1, GL_FALSE, wvp_mesh );
				glUniformMatrix4fv( geometry->shshadow_render->wvp_light, 1, GL_FALSE, wvp_light );
				glActiveTexture( GL_TEXTURE0 + 0 );
				glBindTexture( GL_TEXTURE_2D, get_scene()->depthTexture );
				CHECK_GL_ERROR;
				glUniform1i( geometry->shshadow_render->shadow_texture, 0 );
				setVertexDeclaration( avdecl, geometry->shshadow_render );
				glDrawElements( GL_TRIANGLES, mesh->nfaces*3, GL_UNSIGNED_SHORT, (void*)0 );
				unbindDeclaration( avdecl, geometry->shshadow_render );
			}
			else if ( mat && mat->texture1_map.user_ptr ) 
			{
#define USE_NORMALS 1
#if USE_NORMALS
				mymat4 world_inverse;
				world_mesh.invert( world_inverse );
				
				glUseProgram( geometry->shnormal->prog );
				glUniformMatrix4fv( geometry->shnormal->wvp, 1, GL_FALSE, wvp_mesh );
				glUniformMatrix4fv( geometry->shnormal->world, 1, GL_FALSE, world_mesh );
				glUniformMatrix4fv( geometry->shnormal->world_inverse_transpose, 1, GL_FALSE, world_inverse.transposed() );
				glUniform2f( geometry->shnormal->texture_offset, mat->texture1_map.offset[0],  mat->texture1_map.offset[1] );
				glUniform2f( geometry->shnormal->texture_scale, mat->texture1_map.scale[0], mat->texture1_map.scale[1] );
				setTexture( &geometry->shnormal->texture_data_ddn, 2 );
				glUniform1i( geometry->shnormal->texture_ddn, 2 );
				setTexture( &geometry->shnormal->texture_data_diff, 1 );
				glUniform1i( geometry->shnormal->texture_diff, 1 );
				glUniform3fv( geometry->shnormal->lightW, 1, get_scene()->getLight());
				glUniform3fv( geometry->shnormal->camposW , 1, (const GLfloat *)get_camera()->pos() );
				CHECK_GL_ERROR;
			
				if ( get_scene()->shadow_render )
				{
					glUniformMatrix4fv( geometry->shnormal->wvp_light, 1, GL_FALSE, wvp_light );
					glActiveTexture( GL_TEXTURE0 + 3 );
					glBindTexture( GL_TEXTURE_2D, get_scene()->depthTexture );
					glUniform1i( geometry->shnormal->shadow_texture, 3 );
					CHECK_GL_ERROR;
				}
				setVertexDeclaration( avdecl, geometry->shnormal );
				glDrawElements( GL_TRIANGLES, mesh->nfaces*3, GL_UNSIGNED_SHORT, (void*)0 );
				unbindDeclaration( avdecl, geometry->shnormal );
#else
				MyTexture2D *user_texture = (MyTexture2D*)mat->texture1_map.user_ptr;

				glUseProgram( geometry->shtexture->prog );
				glUniformMatrix4fv( geometry->shtexture->wvp, 1, GL_FALSE, wvp_mesh );
				setTexture( user_texture, 1 );
				glUniform1i( geometry->shtexture->texture, 1 );
				glUniform2fv( geometry->shtexture->texture_offset, 2, mat->texture1_map.offset );
				glUniform2fv( geometry->shtexture->texture_scale, 2, mat->texture1_map.scale );
				if ( get_scene()->shadow_render )
				{
					glUniformMatrix4fv( geometry->shtexture->wvp_light, 1, GL_FALSE, wvp_light );
					glActiveTexture( GL_TEXTURE0 + 3 );
					glBindTexture( GL_TEXTURE_2D, get_scene()->depthTexture );
					glUniform1i( geometry->shtexture->shadow_texture, 3 );
					CHECK_GL_ERROR;
				}
				setVertexDeclaration( avdecl, geometry->shtexture );
				glDrawElements( GL_TRIANGLES, mesh->nfaces*3, GL_UNSIGNED_SHORT, (void*)0 );
				unbindDeclaration( avdecl, geometry->shtexture );
				CHECK_GL_ERROR;
#endif
			}
			else
			{
#if USE_NORMALS && 0
				glUseProgram( geometry->shnormal->prog );
				glUniformMatrix4fv( geometry->shnormal->wvp, 1, GL_FALSE, wvp_mesh );
				glUniformMatrix4fv( geometry->shnormal->world, 1, GL_FALSE, world_mesh );
				setVertexDeclaration( avdecl, geometry->shnormal );
				glDrawElements( GL_TRIANGLES, mesh->nfaces*3, GL_UNSIGNED_SHORT, (void*)0 );
				unbindDeclaration( avdecl, geometry->shtexture );
#else
				mymat4 world_inverse_mesh;
				world_mesh.invert(world_inverse_mesh);
				glUseProgram( geometry->shmarble->prog );
				glUniformMatrix4fv( geometry->shmarble->wvp, 1, GL_FALSE, wvp_mesh );
				glUniformMatrix4fv( geometry->shmarble->world, 1, GL_FALSE, world_mesh );
				glUniformMatrix4fv( geometry->shmarble->world_inverse, 1, GL_FALSE, world_inverse_mesh );
				glUniformMatrix4fv( geometry->shmarble->world_inverse_transpose, 1, GL_FALSE, world_inverse_mesh.transposed() );
				if ( geometry->shmarble->use_3d_texture )
				{
					setTexture( &geometry->shmarble->volume_texture_data, 0 );
					glUniform1i( geometry->shmarble->texture_noise, 0 );
					CHECK_GL_ERROR;
				}

				if ( get_scene()->shadow_render )
				{
					glUniformMatrix4fv( geometry->shmarble->wvp_light, 1, GL_FALSE, wvp_light );
					glActiveTexture( GL_TEXTURE0 + 3 );
					glBindTexture( GL_TEXTURE_2D, get_scene()->depthTexture );
					glUniform1i( geometry->shmarble->shadow_texture, 3 );
					CHECK_GL_ERROR;
				}
				setVertexDeclaration( avdecl, geometry->shmarble );
				glDrawElements( GL_TRIANGLES, mesh->nfaces*3, GL_UNSIGNED_SHORT, (void*)0 );
				unbindDeclaration( avdecl, geometry->shmarble );
				CHECK_GL_ERROR;
#endif
			}
        }
    }

	return true;
}

bool render3DSFile( Lib3dsFile *file, mymat4 *transform, const mymat4 *viewproj, EE_GeometryInstance *geometry )
{
	if ( !file ) return false;

	for (Lib3dsNode *p = file->nodes; p != 0; p = p->next) 
	{
		if ( false == render3DSFileHelper( file, p, transform, viewproj, geometry ) )
		{
			 return false;
		}
    }


	return true;
}


bool process3DSFileHelper( Lib3dsFile *file, Lib3dsNode *node, mymat4 /* out */ *matrix )
{
	if ( !file ) return false;
	if ( !node ) return false;
    {
        Lib3dsNode *p;
        for (p = node->childs; p != 0; p = p->next) {
            process3DSFileHelper(file, p, matrix );
        }
    }
    if (node->type == LIB3DS_NODE_MESH_INSTANCE) {
        int index;
        Lib3dsMesh *mesh;
        Lib3dsMeshInstanceNode *n = (Lib3dsMeshInstanceNode*)node;

        if (strcmp(node->name, "$$$DUMMY") == 0) {
            return true;
        }

        index = lib3ds_file_mesh_by_name(file, n->instance_name);
        if (index < 0)
            index = lib3ds_file_mesh_by_name(file, node->name );
        if (index < 0) {
            return true;
        }
        mesh = file->meshes[index];
		if (!mesh ) return false;

        if (mesh->user_ptr)
		{
            MyVertexDeclaration *avdecl = (MyVertexDeclaration*)mesh->user_ptr;
			mymat4 m, minv;
			memcpy( m.m, &mesh->matrix[0][0], sizeof(float)*4*4 );
			m.invert( (*matrix) );
        }
    }

	return true;
}

bool process3DSFile( Lib3dsFile *file, mymat4 *matrix )
{
	if ( !file ) return false;

	for (Lib3dsNode *p = file->nodes; p != 0; p = p->next) 
	{
		if ( false == process3DSFileHelper( file, p, matrix ) )
		{
			 return false;
		}
    }

	return true;
}


bool pick3DSFileHelper( Lib3dsFile *file, Lib3dsNode *node, myvec3 &ray_dir, myvec3 &ray_point, mymat4 &ctm, float *intersection)
{
	if ( !file ) return false;
	if ( !node ) return false;
    {
        Lib3dsNode *p;
        for ( p = node->childs; p != 0; p = p->next ) {
            pick3DSFileHelper( file, p, ray_dir, ray_point, ctm, intersection );
        }
    }
    if (node->type == LIB3DS_NODE_MESH_INSTANCE) {
        int index;
        Lib3dsMesh *mesh;
        Lib3dsMeshInstanceNode *n = (Lib3dsMeshInstanceNode*)node;

        if (strcmp(node->name, "$$$DUMMY") == 0) {
            return false;
        }

        index = lib3ds_file_mesh_by_name(file, n->instance_name);
        if (index < 0)
            index = lib3ds_file_mesh_by_name(file, node->name );
        if (index < 0) {
            return false;
        }
        mesh = file->meshes[index];
		if (!mesh ) return false;

        if (mesh->user_ptr)
		{
			/* We only use the inverse mesh->matrix, node->matrix and n->pivot are skipped */
			mymat4 M, Minv;
			memcpy( M.m, mesh->matrix, sizeof(M) );
			//mymat4 pivot = mymat4::translation(-n->pivot[0],-n->pivot[1],-n->pivot[2]);
			M.invert(Minv);
			//memcpy( M.m, node->matrix, sizeof(M) );
			//M = M * pivot;
			//M = Minv;
			for ( int f = 0; f < mesh->nfaces; f++ )
			{
				unsigned short *idx = mesh->faces[f].index;
				float (*vptr)[3] = mesh->vertices;
				myvec3 v0 = myvec3( vptr[idx[0]][0],vptr[idx[0]][1],vptr[idx[0]][2] );
				myvec3 v1 = myvec3( vptr[idx[1]][0],vptr[idx[1]][1],vptr[idx[1]][2] );
				myvec3 v2 = myvec3( vptr[idx[2]][0],vptr[idx[2]][1],vptr[idx[2]][2] );
				myvec3 v3 = Minv*v0;
				myvec3 v4 = Minv*v1;
				myvec3 v5 = Minv*v2;
				v3 = ctm*v3;
				v4 = ctm*v4;
				v5 = ctm*v5;
				if ( intersectRayTriangle( ray_dir, ray_point, v3, v4, v5, intersection ) )
				{
					return true;
				}
			}
		}
    }

	return false;
}

void destroy3DSFile( Lib3dsFile *file3ds )
{
	lib3ds_file_free( file3ds );
}
