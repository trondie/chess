#include <iostream>
#include "Piece.h"
#include "Vec2.h"
#include "Node.h"
#include "NodeHandler.h"
#include "constants.h"
#include "Board.h"
using namespace std;

/*Node and NodeHandler implementation. This is the data structure that is used when tracking moves in the search tree*/

Node::Node(){
	parentNode	= NULL;
	moveType    = PURE_MOVE_REGULAR;
	this->alpha = ALPHA;
	this->beta	= BETA;
	rootTag = -28;
}
Node::Node(Node * parentNode, Piece * movePiece, Piece * hitPiece, Vec2 fromPos){
	rootTag = parentNode->rootTag-1;
	this->alpha			= ALPHA;
	this->beta			= BETA;
	this->parentNode	= parentNode;
	this->movePiece		= *movePiece;
	moveType            = PURE_MOVE_REGULAR;

	if (hitPiece != NULL){
		moveType = PURE_MOVE_HIT;
		this->hitPiece = *hitPiece;
	}
	this->fromPos = fromPos;
}

Node::Node(Node * parent, Piece * movePiece, Piece * hitPiece,  Vec2 fromPos, int moveType)
{
	rootTag = parentNode->rootTag-1;
	this->alpha			= ALPHA;
	this->beta			= BETA;
	this->parentNode	= parentNode;
	this->movePiece		= *movePiece;
	this->moveType      = moveType;
	this->fromPos = fromPos;

	if (hitPiece != NULL){
		this->hitPiece = *hitPiece;
	}
}

NodeHandler::NodeHandler(){}

NodeHandler::NodeHandler(Node * root){
	nodeId = 0;
	rootNode = root;
	lastChild = root;
}
NodeHandler::~NodeHandler(){
	/*Node * tempNode = lastChild;
	while (tempNode->parentNode != NULL){
		lastChild = tempNode->parentNode;
		delete tempNode;
		tempNode = lastChild;
	}*/
}
Node * NodeHandler::getLastChild(){
	return lastChild;
}
void NodeHandler::addChild(Piece * movePiece, Piece * hitPiece, Vec2 fromPos){
	Node * newNode = new Node(lastChild, movePiece, hitPiece, fromPos);
	lastChild = newNode;
	nodeId++;;
}

bool NodeHandler::removeLastChild(){
	if (lastChild->parentNode == NULL)
		return false;
	Node * tempNode = lastChild;
	lastChild = lastChild->parentNode;
	delete tempNode;
	return true;
}

