#include <iostream>
#include <string>
#include <math.h>
#include "Piece.h"
#include "Vec2.h"

using namespace std;

Piece::Piece(){}
Piece::Piece(int typed, int valued, int colord, Vec2 posd, int rankd){
	value	= valued;
	color	= colord;
	type	= typed;
	pos		= posd;
	rank	= rankd;
	firstMove = true;
}
//Haven't checked for ambiguity!!!! I guess it works for now :) 
//Also, the ordering of the pieces will affect performance in relation to that we search through the pieces in a DFS manner. (!)
//That is, some pieces are best to evaluate before others.
bool Piece::operator <(const Piece &rhs) const{
	if ( (color + value + (type+1) + rank*10) < (rhs.color + rhs.value + (rhs.type+1) + rhs.rank*10)){
		return true;
	}
	//else if ((color + value + (type+1) + rank*2) <= (rhs.color + rhs.value + (rhs.type+1) + rhs.rank*2)){
		//return false; 
	//}
	return false;
}

bool Piece::operator ==(const Piece &rhs) const{
	if ( (color + value + (type+1) + rank*2) == (rhs.color + rhs.value + (rhs.type+1) + rhs.rank*2))
		return true;
	return false;
}
