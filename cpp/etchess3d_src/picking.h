#ifndef _PICKING_H_
#define _PICKING_H_

#include "math_helpers.h"

myvec3 calculateCamPos(float radius, float angle);
float closestpoint( myvec3 &p1, myvec3 &p2, myvec3 &sphere_pos );
float intersectRaySphere3( myvec3 ray_dir, myvec3 sphere_pos, float sphere_radius_squared );
float intersectRaySphere2( myvec3 &ray_dir, myvec3 &ray_pos, myvec3 &sphere_pos, float sphere_radius_squared );
float intersectRaySphere( myvec3 &ray_dir, myvec3 &ray_point, myvec3 &sphere_pos, float sphere_radius_squared );
bool intersectRayRectangle( myvec3 &ray_dir, myvec3 &ray_point, myvec3 &bbox_min, myvec3 &bbox_max, float /* out */ *t );
bool intersectRayTriangle( myvec3 &ray_dir, myvec3 &ray_point, myvec3 &v0, myvec3 &v1, myvec3 &v2, float /* out */ *t);

#endif /* _PICKING_H_ */
