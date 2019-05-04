#ifndef __3DS_HELPERS_H__
#define __3DS_HELPERS_H__

#include "math_helpers.h"

struct Lib3dsFile;
struct Lib3dsNode;
class EE_GeometryInstance;

bool read3DSFile( struct Lib3dsFile **file3ds, const char *filename, 
	myvec3 *bmin , myvec3 *bmax, 
	myvec3 *bbs, float *size, /* bounding box dimensions */
	myvec3 *bbc, /* bounding box center */
	bool ignore_transform);

bool render3DSFile( Lib3dsFile *file, mymat4 *transform, const mymat4 *viewproj, EE_GeometryInstance *geometry );
bool process3DSFile( struct Lib3dsFile *file, mymat4 *matrix );
bool pick3DSFileHelper( struct Lib3dsFile *file, struct Lib3dsNode *node, myvec3 &ray_dir, myvec3 &ray_point, mymat4 &ctm, float *intersection);
void destroy3DSFile( Lib3dsFile *file3ds );

#endif /* __3DS_HELPERS_H__ */