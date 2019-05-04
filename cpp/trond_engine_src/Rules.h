#ifndef _RULES_H_
#define _RULES_H_

#include <iostream>
#include "Piece.h"
using namespace std;

//Let's make the rules class as independent as possible
class DLL_EXPORT Rules {
public:
	Rules();
	bool checkMove(Piece &piece, Vec2 &from, Vec2 &to, int &obstacle, Piece *pieceTo, int &moveType);
	bool borderCheck(Vec2 &from, Vec2 &to);
};


#endif 




