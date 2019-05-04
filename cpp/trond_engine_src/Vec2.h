#ifndef _H_VEC2_
#define _H_VEC2_

#include <cstdlib>
#include <string>

using namespace std;

#if defined( TROND_ENGINE_EXTERNAL_GUI )
#ifdef PLATFORM_WINDOWS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif
#else
#define DLL_EXPORT
#endif

//A simple and very handy Vec2 class..
class DLL_EXPORT Vec2 {
public:
	Vec2();
	Vec2(int,int);
	int x;
	int y;
	void toString();
	bool operator <(const Vec2 &rhs) const;
	bool operator ==(const Vec2 &rhs) const;
	void operator =(const Vec2 &rhs);
	friend Vec2 operator -(const Vec2 &lhs, const Vec2 &rhs);
	friend Vec2 operator +(const Vec2 &lhs, const Vec2 &rhs);
	
};

#endif
