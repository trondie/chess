#include <iostream>
#include "Rules.h"

using namespace std;

Vec2::Vec2(){}
Vec2::Vec2(int xp, int yp){
	x = xp;
	y = yp;
}
void Vec2::toString(){
	cout << "X : " << x << ", Y : " << y << endl;
}

void Vec2::operator =(const Vec2 &rhs){
	x = rhs.x;
	y = rhs.y;
}
bool Vec2::operator <(const Vec2 &rhs) const{
	if (y < rhs.y){
		return true;
	} 
	else if ((y == rhs.y) && (x < rhs.x)){
		return true;
	}
	return false;
}

bool Vec2::operator ==(const Vec2 &rhs) const{
	if ( (x == rhs.x) && (y == rhs.y))
		return true;
	return false;
}

Vec2 operator -(const Vec2 &lhs, const Vec2 &rhs){
	return Vec2((lhs.x-rhs.x), (lhs.y-rhs.y));
}
Vec2 operator +(const Vec2 &lhs, const Vec2 &rhs){
	return Vec2( (lhs.x+rhs.x), (lhs.y+rhs.y) );
}
