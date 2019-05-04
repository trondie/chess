#include "picking.h"
#include "mymath.h"
#include "math_helpers.h"
#include "stdio.h"
#include "TestGLSL.h"

#define FP32_POSITIVE_INFINITY 0x7FFFFF
#define FP32_NEGATIVE_INFINITY 0x800000

typedef struct fp32int_t
{
	union
	{
		float f;
		unsigned int i;
	} u;
} fp32int;

static fp32int fi =  { FP32_POSITIVE_INFINITY };

/** 
 * Function that calculates the camera position given an angle and a radius. This function
 * enables the camera to rotate around the scene object.
 */
myvec3 calculateCamPos(float radius, float angle)
{
	float angleRad = angle / 180 * M_PI;
	return myvec3(radius * cos(angleRad), 0.0f, radius * sin(angleRad));
}

float closestPoint( myvec3 &p1, myvec3 &p2, myvec3 &sphere_pos )
{
	myvec3 line = p2-p1;
	myvec3 p3mp1 = sphere_pos-p1;
	float ldot = line*line;
	if ( ldot == 0 ) return fi.u.f;
	return (p3mp1*line) / ldot;		
}

float intersectRaySphere3( myvec3 ray_dir, myvec3 sphere_pos, float sphere_radius_squared )
{
	float ldc = ray_dir*sphere_pos;
	float sqrt_value = ldc*ldc - sphere_pos*sphere_pos + sphere_radius_squared;
	if ( sqrt_value < 0 ) 
	{
		Base::log( "indeterministic intersection\n");
		return fi.u.f;
	}
	else if ( sqrt_value == 0 ) 
	{
		Base::log( "one solution exists\n");
	}
	else
	{
		Base::log( "two solution exists\n");
	}
	return ldc + sqrt(sqrt_value);
}

float intersectRaySphere2( myvec3 &ray_dir, myvec3 &ray_pos, myvec3 &sphere_pos, float sphere_radius_squared )
{
	myvec3 dst = sphere_pos-ray_pos;
	float B = (dst*ray_dir);
	float C = (dst*dst) - sphere_radius_squared;
	float D = B*B - C;
	return D > 0 ? -B - sqrt(D) : fi.u.f;
}

bool intersectRayRectangle( myvec3 &ray_dir, myvec3 &ray_point, myvec3 &bbox_min, myvec3 &bbox_max, float /* out */ *t )
{
	myvec3 dirfrac;
	dirfrac = 1.0f / ray_dir;
	myvec3 t1 = (bbox_min - ray_point);
	myvec3 t2 = (bbox_max - ray_point);
	t1 *= dirfrac;
	t2 *= dirfrac;

	float tmin = MAX(MAX(MIN(t1.x, t2.x), MIN(t1.y, t2.y)), MIN(t1.z, t2.z));
	float tmax = MIN(MIN(MAX(t1.x, t2.x), MAX(t1.y, t2.y)), MAX(t1.z, t2.z));

	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
	if (tmax < 0)
	{
		*t = tmax;
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		*t = tmax;
		return false;
	}

	*t = tmin;
	return true;
}

float intersectRaySphere( myvec3 &ray_dir, myvec3 &ray_point, myvec3 &sphere_pos, float sphere_radius_squared )
{
	myvec3 oc = ( ray_point - sphere_pos );
	float l2oc = oc*oc;
	if ( l2oc < sphere_radius_squared )
	{
		/* Base::log( "ray origin inside of sphere as l2oc(%f) < r^2 (%f)\n", l2oc, sphere_radius_squared ); */
		float tca = (oc*ray_dir) / (ray_dir*ray_dir);
		float l2hc = (sphere_radius_squared - l2oc) / (ray_dir*ray_dir) + (tca*tca);
		return tca + sqrt(l2hc);
	}
	else
	{
		/* Base::log( "ray origin outside of sphere as l2oc(%f) >= r^2 (%f)\n", l2oc, sphere_radius_squared ); */
		float tca = oc*ray_dir / (ray_dir*ray_dir);
		if ( tca < 0 )
		{
			/* Base::log( "sphere behind ray \n", l2oc, sphere_radius_squared ); */
			return fi.u.f;
		}
		float l2hc = (sphere_radius_squared - l2oc) / (ray_dir*ray_dir) + tca*tca;
		//return l2hc > 0 ? tca - sqrt(l2hc) : std::numeric_limits<float>::infinity();
		if ( l2hc > 0 ) return tca - sqrt(l2hc);
		else
		{
			/* Base::log( "ray misses sphere\n" );*/
			return fi.u.f;
		}
	}
}

/* http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/ */
bool intersectRayTriangle(myvec3 &d, myvec3 &p, myvec3 &v0, myvec3 &v1, myvec3 &v2, float *t)
{
	myvec3 e1,e2,h,s,q;
	float a,f,u,v;
	e1 = v1-v0;
	e2 = v2-v0;

	h = d.crossProduct(e2);
	a = e1.dot(h);

	if (a > -0.00001 && a < 0.00001)
		return(false);

	f = 1/a;
	s = p - v0;
	u = f * s.dot(h);

	if (u < 0.0 || u > 1.0)
		return(false);

	q = s.crossProduct(e1);
	v = f * d.dot(q);

	if (v < 0.0 || u + v > 1.0)
		return(false);

	// at this stage we can compute t to find out where
	// the intersection point is on the line
	*t = f * e2.dot(q);

	if (*t > 0.00001) // ray intersection
		return(true);

	else // this means that there is a line intersection
		 // but not a ray intersection
		 return (false);
}