#ifndef _NODE_H_
#define _NODE_H_

#include <iostream>
#include "Piece.h"
#include "Vec2.h"

/*A node in the search tree. Should make simpler nodes than this besides the root node*/
class DLL_EXPORT Node {
public:
	Node();
	Node(Node * parent, Piece * movePiece, Piece * hitPiece,  Vec2 fromPos);
	Node(Node * parent, Piece * movePiece, Piece * hitPiece,  Vec2 fromPos, int moveType);
	Node  *	parentNode;
	Piece   movePiece;
	Piece   hitPiece;
	//Piece   alphaPiece;
	//Piece   betaPiece;
	Vec2    fromPos;

	Vec2	alphaFrom;
	Vec2    alphaTo;
	int		alpha;
	int		beta;
	int moveType;
	int rootTag;
};

#endif _NODE_H_
