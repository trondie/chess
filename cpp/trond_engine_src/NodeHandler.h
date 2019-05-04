#ifndef _NODEHANDLER_H_
#define _NODEHANDLER_H_

#include <cstdlib>
#include "Node.h"
#include "Piece.h"

using namespace std;

/*NodeHandler to track the nodes / moves in the tree and in the board as well*/

class DLL_EXPORT NodeHandler {
public:

	Node * rootNode;
	Node * lastChild;
	long nodeId;

	NodeHandler();
	~NodeHandler();
	NodeHandler(Node * root);
	Node *	getLastChild();
	void	addChild(Piece * movePiece, Piece * hitPiece, Vec2 fromPos);
	bool	removeLastChild();

};

#endif _NODEHANDLER_H_