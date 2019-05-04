#ifndef _PIECE_H_
#define _PIECE_H_

#include <cstdlib>
#include <string>
#include "Vec2.h"

using namespace std;

enum { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum { WHITE, BLACK }; 

static const int NOBS = -1;

class DLL_EXPORT Piece {
public:
	Piece();
	Piece(int type, int value, int color, Vec2 pos, int rank);

	Vec2 pos;
	int value;
	int color;
	int type;	
	int rank;
	bool firstMove;

	//Need to overload for use in the multimap container in Engine. (Check for ambiguity / better comparison)
	bool operator <(const Piece &rhs) const;
	bool operator ==(const Piece &rhs) const;
};

#endif
