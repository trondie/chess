#include <iostream>
#include "Piece.h"
#include "Engine.h"
#include "Board.h"
#include "Vec2.h"
#include "Rules.h"
#include "constants.h"
#include <map>
#include <math.h>
#include <algorithm>
#include <vector>

using namespace std;
long nodes = 0;

Engine::Engine(){
	//Turn based functionality is not implemented yet.
	turn = WHITE;
	player = WHITE;
	depth = 3;
	abortMove = false;
}
Engine::Engine(int difficulty, bool useEngine, Board &board){
	turn = WHITE;
	player = WHITE;
	depth = difficulty;
}

//Trace overview
void Engine::tracePossibleMoves(Piece &piece, Board &board){
	if ( false == board.checkPieceExist( piece )) return;
	map<Vec2, int> moves;
	switch(piece.type){
	case PAWN:
		{
			tracePawn(piece, board, moves);
			break;
		}
	case ROOK:
		{
			straightTrace(piece, board, moves);
			break;
		}
	case BISHOP:
		{
			diagonalTrace(piece, board, moves);
			break;
		}
	case KNIGHT:
		{
			traceKnight(piece, board, moves);
			break;
		}
	case QUEEN:
		{
			straightTrace(piece, board, moves);
			diagonalTrace(piece, board, moves);
			break;
		}
	case KING:
		{
			straightTrace(piece, board, moves);
			diagonalTrace(piece, board, moves);
			break;
		}
	}

}

/*** ESSENTIAL ***/ 
//Create node branches for a node. This derives all possible moves that can be made from a position. 
//We want to traverse these moves later.
void Engine::createNodeBranches(Board &board, int color){
	totalMoves.clear();
	
	for (map<Vec2, Piece>::iterator it = board.pieces.begin(); it != board.pieces.end(); ++it){
		if (it->second.color == color){
			tracePossibleMoves((Piece&)it->second, board);
		}
	}
}

int Engine::previousMove()
{
	return tempBoard->nodeHandler.lastChild->moveType;
}

//Starts the a-b pruning and returns the calculated move
Vec2 * Engine::startAlphaBetaPruning(int depth, int alpha, int beta, Board &board, int color){
	
	Vec2 * move = new Vec2[2];
	//This board is used by the engine for all the moves it goes through
	tempBoard = board.copyBoard();
	//The root node will contain the move that the engine eventually decides to choose
	cout << "Calculating move . . . " << endl;
	nodes = 0;
	this->depth = depth;
	int value = alphaBetaPrune(tempBoard->nodeHandler.lastChild, depth, alpha, beta, board, color );
	cout << endl << "NODES PROCESSED : " << nodes;
	Node *child = tempBoard->nodeHandler.lastChild;
	Vec2 hitPos = Vec2(-1, -1);

	/* workaround for CPU move-to-chess issue */
	Board *sboard = board.copyBoard();
	sboard->move(child->alphaFrom, child->alphaTo, hitPos );
	if ( CHECK == checkIfCheck( *sboard, color == WHITE ? BLACK : WHITE ) )
	{
		/* Engine moved to chess, escape-move needed
		 * Basically iterate through possible moves. 
		 * Select the first one not in chess.*/
		createNodeBranches(*tempBoard, color);
		map<Piece, map<Vec2, int> > *tempTotalMoves = new map<Piece, map<Vec2, int> >();
		*tempTotalMoves = totalMoves;
		map<Piece, map<Vec2, int> >::iterator internalIterator = tempTotalMoves->begin();
		map<Vec2, int>::iterator innerIt;
		while (internalIterator != tempTotalMoves->end()){
			innerIt = internalIterator->second.begin();
			while(innerIt != internalIterator->second.end()){
				Board *sboard = board.copyBoard();
				Piece movePieceTemp = internalIterator->first;
				sboard->move(movePieceTemp.pos, innerIt->first, hitPos );
				if ( NOT_CHECK == checkIfCheck( *sboard, color == WHITE ? BLACK : WHITE ) ) 
				{
					move[0] = movePieceTemp.pos;
					move[1] = innerIt->first;
					return move;
				}
				innerIt++;
			}
			internalIterator++;
		}
	}

	move[0] = child->alphaFrom;
	move[1] = child->alphaTo;
	abortMove = false;

	return move;
}
//This function can be used to retrieve the root node after calling startAlphaBetaPruning(..)
Node * Engine::getRootNode(){
	return nodeHandler.rootNode;
}

/*ESSENTIAL*/

/*
We want to use the alpha and beta values in such a way that we move the overlapping domain of their number lines
until they do not overlap anymore, and then prune the following subtrees. In this way, it's impossible that pruning will
result in a poorer move than evaluating the whole tree. 
*/

int Engine::alphaBetaPrune(Node * node, int depthLeft, int alpha, int beta, Board &board, int color ){

	Piece movePieceTemp;
	int a = alpha; 
	int b = beta; 
	PureMoveType hitTemp;
	bool loopBreak = false;
	if (depthLeft <= 0){
		nodes++;
		return evaluate(color);
	}
	Vec2 tempTo; 
	createNodeBranches(*tempBoard, color);

	map<Piece, map<Vec2, int> > *tempTotalMoves = new map<Piece, map<Vec2, int> >();
	*tempTotalMoves = totalMoves;

	std::vector<map<Piece,map<Vec2,int> >::iterator> toErase;

	map<Piece, map<Vec2, int> >::iterator internalIterator = tempTotalMoves->begin();
	map<Vec2, int>::iterator innerIt;
	
	if (color == WHITE){
		while (internalIterator != tempTotalMoves->end()){
			innerIt = internalIterator->second.begin();
			while(innerIt != internalIterator->second.end()){
				nodes++;
				tempTo = innerIt->first;
				movePieceTemp = internalIterator->first;
				hitTemp = (PureMoveType)innerIt++->second;
				Vec2 hitPos;
				//Making a pure move from totalMoves<> and thereby creating a new child node in tempBoard's nodeHandler.
				tempBoard->pureMove(movePieceTemp.pos, tempTo, hitTemp, hitPos); 
				
				//Recursive dfs calls to check if alpha value is larger than current alpha value in this node
				a = max(a, alphaBetaPrune(tempBoard->nodeHandler.lastChild, depthLeft-1, a, beta, board, BLACK ));
				if ( abortMove ) return 100000;

				//If the number lines of alpha and beta no longer overlap, we want to stop.
				if (beta <= a){
					internalIterator = tempTotalMoves->end();
					loopBreak = true;
					break;
				}

				//If parent node has alpha value less than the evaluated alpha, then we want that alpha!
				//We update the parent node with the move (if level 1) and value that corresponds to the higher alpha
				if (a > tempBoard->nodeHandler.lastChild->alpha){
					tempBoard->nodeHandler.lastChild->alpha = a;
					//We need this information only from tree level 0
					if (depthLeft == depth)
					{
						tempBoard->nodeHandler.rootNode->alpha = a;
						tempBoard->nodeHandler.lastChild->alphaFrom = movePieceTemp.pos;
						tempBoard->nodeHandler.lastChild->alphaTo = tempTo;
						tempBoard->nodeHandler.lastChild->moveType = hitTemp;						
					}
				}
			}
			if (loopBreak)
				break;
			internalIterator++;
		}

		tempTotalMoves->clear();
		//This happens for non-terminal nodes, and we will move back to parent position
		tempBoard->undoMove();
		//This alpha is returned from non-terminal nodes
		return a;
	}
	else{
		while (internalIterator != tempTotalMoves->end()){
			innerIt = internalIterator->second.begin();
			while(innerIt != internalIterator->second.end()){
				nodes++;
				tempTo = innerIt->first;
				movePieceTemp = internalIterator->first;
				hitTemp = (PureMoveType)innerIt++->second;
				Vec2 hitPos;
				tempBoard->pureMove(movePieceTemp.pos, tempTo, hitTemp, hitPos);

				b = min(b, alphaBetaPrune(tempBoard->nodeHandler.lastChild, depthLeft-1, alpha, b, board, WHITE ));
				if (b <= alpha){
					internalIterator = tempTotalMoves->end();
					loopBreak = true;
					break;
				}

				if (b < tempBoard->nodeHandler.lastChild->beta){
					tempBoard->nodeHandler.lastChild->beta = b;
					if (depthLeft == depth)
					{
						tempBoard->nodeHandler.lastChild->alphaFrom = movePieceTemp.pos;
						tempBoard->nodeHandler.lastChild->alphaTo = tempTo;
						tempBoard->nodeHandler.lastChild->moveType = hitTemp;
					}
				}
			}
			if (loopBreak)
				break;
			internalIterator++;
		}
		tempTotalMoves->clear();
		tempBoard->undoMove();
		return b;
	}
}

int Engine::evaluate(int color){
	int score = tempBoard->getScore(color);
	int tempY = tempBoard->nodeHandler.lastChild->movePiece.pos.y;
	int tempM = totalMoves[tempBoard->nodeHandler.lastChild->movePiece].size();
	if (tempBoard->nodeHandler.lastChild->movePiece.type > 0){
		score += tempM;
		if ((tempY > 1))
			score += 5;
	}
	else{
		score += 2;
	}
	if (PURE_MOVE_HIT == tempBoard->nodeHandler.lastChild->moveType){
		score += 50 * tempBoard->nodeHandler.lastChild->hitPiece.value;
	}
	if (color == WHITE){
		score -= tempBoard->getScore(BLACK);
		tempBoard->undoMove();
		return score;
	}
	score -= tempBoard->getScore(WHITE);
	tempBoard->undoMove();
	return -score;
	
}

int Engine::evaluate(Node * node, Board &board){
	return 0;
}

int Engine::evaluate(Board &board, Piece &piece){
	piece.firstMove = false;
	return PURE_MOVE_REGULAR;
}

int Engine::evaluatePawnPromote(Board &board, Piece &piece){
	piece.firstMove = false;
	return PURE_MOVE_PAWN_PROMOTE;
}

int Engine::evaluatePawnPromote(Board &board, Piece &piece, Vec2 &obstPos){
	piece.firstMove = false;
	return PURE_MOVE_HIT_PAWN_PROMOTE;
}

int Engine::evaluateAnPassant(Board &board, Piece &piece, Vec2 &obstPos){
	piece.firstMove = false;
	return PURE_MOVE_HIT_ANPASSANT;
}

int Engine::evaluateAnPassant(Board &board, Piece &piece){
	piece.firstMove = false;
	return PURE_MOVE_ANPASSANT;
}

int Engine::evaluate(Board &board, Piece &piece, Vec2 &obstPos){
	piece.firstMove = false;
	return PURE_MOVE_HIT;
}

int Engine::evaluateRocade(Board &board, Piece &piece, Vec2 &obstPos,PureMoveType rocadeType){
	piece.firstMove = false;
	return rocadeType;
}

void Engine::toString(){
	int count = 0; 
	map<Piece, map<Vec2, int> >::iterator it;
	map<Vec2, int>::iterator innerIt;
	for (it = totalMoves.begin(); it != totalMoves.end(); ++it){
		for (innerIt = it->second.begin(); innerIt != it->second.end(); ++innerIt){
			count++;
			cout << "[X : " << innerIt->first.x << ", Y : " << innerIt->first.y << "], Eval :"<< innerIt->second << "Piece : " << it->first.type << endl;
		}
	}
	cout << "TOTAL COUNT! : " << count << endl;
	cout << "TOTAL MAP TE_SIZE : " << (int)totalMoves.size() << endl;
}
void Engine::toString(Piece &piece){
	map<Vec2, int>::iterator it;
	Vec2 temp;
	//map<Piece, map<Vec2, int > > totalMoves
	for (it = totalMoves[piece].begin(); it != totalMoves[piece].end(); ++it){
		temp = it->first;
		temp.toString();
	}
}

void Engine::diagonalTrace(Piece &piece, Board &board, map<Vec2, int> &moves) {
			
	Vec2 currentPos		= piece.pos;
	Vec2 toPos			= piece.pos;
	Vec2 obstPos		= Vec2();
	int dxw				= currentPos.x;
	int dxe				= (TE_SIZE-1)-currentPos.x;
	int dyn				= (TE_SIZE-1)-currentPos.y;
	int dys				= currentPos.y;
	int possiblePosNW	= min(dxw, dyn); 
	int possiblePosNE	= min(dxe, dyn);
	int possiblePosSW	= min(dxw, dys);
	int possiblePosSE	= min(dxe, dys); 
	int i, inc1, inc2;
	
	//Simply reassign
	if (piece.type == KING){

		possiblePosNW	= min(min(dxw,1), min(dyn,1));
		possiblePosNE	= min(min(dxe,1), min(dyn,1));
		possiblePosSW	= min(min(dxw,1), min(dys,1));
		possiblePosSE	= min(min(dxe,1), min(dys,1));

	}
	//NW
	inc1 = -1;
	inc2 = 1;
	straightDiagonalPerform(toPos, currentPos, obstPos, piece, board, moves, possiblePosNW, inc1, inc2);
	//NE
	toPos = currentPos;
	inc1 = 1;
	inc2 = 1;
	straightDiagonalPerform(toPos, currentPos, obstPos, piece, board, moves, possiblePosNE, inc1, inc2);
	//SW
	toPos = currentPos;
	inc1 = -1;
	inc2 = -1;
	straightDiagonalPerform(toPos, currentPos, obstPos, piece, board, moves, possiblePosSW, inc1, inc2);
	//SE
	toPos = currentPos;
	inc1 = 1;
	inc2 = -1;
	straightDiagonalPerform(toPos, currentPos, obstPos, piece, board, moves, possiblePosSE, inc1, inc2);
	totalMoves[piece] = moves;

}
void Engine::straightTrace(Piece &piece, Board &board, map<Vec2, int> &moves){

	Vec2 currentPos = piece.pos;

	Vec2 toPos = piece.pos;
	Vec2 obstPos = Vec2();
	int possiblePosHorizontalWest	= currentPos.x;
	int possiblePosHorizontalEast	= (TE_SIZE-1)-currentPos.x;
	int possiblePosNorth			= (TE_SIZE-1)-currentPos.y;
	int possiblePosSouth			= currentPos.y;
	int i, inc1, inc2; 

	//Just reassign
	if (piece.type == KING){

		possiblePosHorizontalWest	= min(currentPos.x, 1);
		possiblePosHorizontalEast	= min((TE_SIZE-1)-currentPos.x,1);
		possiblePosNorth			= min((TE_SIZE-1)-currentPos.y,1);
		possiblePosSouth			= min(currentPos.y, 1);

	}
	
	if (piece.type == KING || piece.type == ROOK )
	{
		rocadePerform( toPos, currentPos, obstPos, piece, board, moves );
		toPos = piece.pos;
	}
	//WEST
	inc1 = -1;
	inc2 = 0;
	straightDiagonalPerform(toPos, currentPos, obstPos, piece, board, moves, possiblePosHorizontalWest, inc1, inc2);
	//EAST
	toPos = currentPos;
	inc1 = 1;
	inc2 = 0;
	straightDiagonalPerform(toPos, currentPos, obstPos, piece, board, moves, possiblePosHorizontalEast, inc1, inc2);
	//NORTH
	toPos = currentPos;
	inc1 = 0;
	inc2 = 1;
	straightDiagonalPerform(toPos, currentPos, obstPos, piece, board, moves, possiblePosNorth, inc1, inc2);
	//SOUTH
	toPos = currentPos;
	inc1 = 0;
	inc2 = -1;
	straightDiagonalPerform(toPos, currentPos, obstPos, piece, board, moves, possiblePosSouth, inc1, inc2);

	totalMoves[piece] = moves;
	
}

void Engine::traceKnight(Piece &piece, Board &board, map<Vec2, int> &moves){
	
	Vec2 currentPos = piece.pos;
	Vec2 toPos		= piece.pos;
	Vec2 obstPos	= Vec2();
	int i;
	
	toPos.x += 1;
	toPos.y += 2;
	knightPerform(toPos, currentPos, obstPos, piece, board, moves);
	toPos.x -= 2;
	knightPerform(toPos, currentPos, obstPos, piece, board, moves);
	toPos.y -= 4;
	knightPerform(toPos, currentPos, obstPos, piece, board, moves);
	toPos.x += 2;
	knightPerform(toPos, currentPos, obstPos, piece, board, moves);
	toPos = currentPos;
	toPos.x += 2;
	toPos.y += 1;
	knightPerform(toPos, currentPos, obstPos, piece, board, moves);
	toPos.x -= 4;
	knightPerform(toPos, currentPos, obstPos, piece, board, moves);
	toPos.y -= 2;
	knightPerform(toPos, currentPos, obstPos, piece, board, moves);
	toPos.x += 4;
	knightPerform(toPos, currentPos, obstPos, piece, board, moves);

	totalMoves[piece] = moves;

}
void Engine::tracePawn(Piece &piece, Board &board, map<Vec2, int> &moves){

	Vec2 currentPos = piece.pos;
	Vec2 toPos		= piece.pos;
	Vec2 obstPos	= Vec2();
	int obstacle	= TEMPNOBS;

	if (piece.color == WHITE)
	{
		//N+1
		toPos.y += 1;
		pawnPerform(toPos, currentPos, obstPos, piece, board, moves, obstacle, true);
		//NE
		toPos.x += 1;
		pawnPerform(toPos, currentPos, obstPos, piece, board, moves, obstacle, false);
		//NW
		toPos.x -= 2;
		pawnPerform(toPos, currentPos, obstPos, piece, board, moves, obstacle, false);
		//Initial move
		toPos.x += 1;
		toPos.y += 1;
		pawnPerform(toPos, currentPos, obstPos, piece, board, moves, obstacle, true);
	}
	else{

		//S-1
		toPos = currentPos;
		toPos.y += -1;
		pawnPerform(toPos, currentPos, obstPos, piece, board, moves, obstacle, true);
		//SE
		toPos.x += 1;
		pawnPerform(toPos, currentPos, obstPos, piece, board, moves, obstacle, false);
		//SW
		toPos.x -= 2;
		pawnPerform(toPos, currentPos, obstPos, piece, board, moves, obstacle, false);
		//Initial move
		toPos.x += 1;
		toPos.y -= 1;
		pawnPerform(toPos, currentPos, obstPos, piece, board, moves, obstacle, true);
	}
	totalMoves[piece] = moves;
}

void Engine::rocadePerform( Vec2 &toPos, Vec2 &currentPos, Vec2 &obstPos, Piece &piece, Board &board, map<Vec2,int> &moves )
{
	for (int i = 0; i < 2; i++)
	{
		if ( piece.type == KING )
		{
			toPos.x = (i == 0) ? 0 : 7;
			toPos.y = currentPos.y;
		}
		else if ( piece.type == ROOK )
		{
			if ( i == 1 ) break;
			toPos.x = 4;
			toPos.y = currentPos.y;
		}
		if (board.obstacleCheck(currentPos, toPos, obstPos))
		{
			Piece obstPiece = board.pieces[obstPos];
			if (obstPiece.color == piece.color 
				&& obstPiece.firstMove 
				&& piece.firstMove )
			{
				if (obstPiece.type == KING && piece.type == ROOK)
				{ 
					PureMoveType rocadeType = (obstPiece.pos.x > piece.pos.x) ? PURE_MOVE_ROCADE_RKLONG : PURE_MOVE_ROCADE_RKSHORT;
					moves[toPos] = evaluateRocade( board, piece, obstPos, rocadeType );
				}
				else if ( obstPiece.type == ROOK && piece.type == KING )
				{
					PureMoveType rocadeType = (piece.pos.x > obstPiece.pos.x) ? PURE_MOVE_ROCADE_KRLONG : PURE_MOVE_ROCADE_KRSHORT;
					moves[toPos] = evaluateRocade( board, piece, obstPos, rocadeType );
				}
			}
		}
	}
}

//Performs a straight or diagonal trace.
void Engine::straightDiagonalPerform(Vec2 &toPos, Vec2 &currentPos, Vec2 &obstPos, Piece &piece, Board &board, map<Vec2, int> &moves, int possiblePos, int inc1, int inc2){

	for (int i = 0; i < possiblePos; i++){
		toPos.x += inc1;
		toPos.y += inc2;
		if (board.obstacleCheck(currentPos, toPos, obstPos)){
			if (board.pieces[obstPos].color == piece.color){
				break;
			}
			else{
				//If we can hit the obstacle, we can move to that position. 
				moves[toPos] = evaluate(board, piece, obstPos);
				break;
			}
		}
		moves[toPos] = evaluate(board, piece);
	}
}
//Performs a trace for a knight 
void Engine::knightPerform(Vec2 &toPos, Vec2 &currentPos, Vec2 &obstPos, Piece &piece, Board &board, map<Vec2, int> &moves){
	
	if (board.boundaryCheck(currentPos, toPos)){
		if (board.obstacleCheck(currentPos, toPos, obstPos)){
			if (board.pieces[obstPos].color != piece.color){
				moves[toPos] = evaluate(board, piece, obstPos);
			}
		}
		else{
			moves[toPos] = evaluate(board, piece);
		}
	}
}
void Engine::pawnPerform(Vec2 &toPos, Vec2 &currentPos, Vec2 &obstPos, Piece &piece, Board &board, map<Vec2, int> &moves, int &obstacle, bool regular){
	
	if (board.boundaryCheck(currentPos, toPos)){
		if (regular){
			if (!board.obstacleCheck(currentPos, toPos, obstPos)){
				
				int moveType;
				if (board.rules.checkMove(piece, currentPos, toPos, obstacle, NULL, moveType)){
					int diffy = toPos.y - currentPos.y;

					if ( abs( diffy ) == 2 )
					{
						moves[toPos] = evaluateAnPassant(board, piece);
					}
					else if ( toPos.y == 0 || toPos.y == 7 )
					{
						moves[toPos] = evaluatePawnPromote(board, piece);
					}
					else
					{
						moves[toPos] = evaluate(board, piece);
					}
				}
			}
		}
		else{
			if (board.obstacleCheck(currentPos, toPos, obstPos)){
				obstacle = board.pieces[obstPos].color;

				int moveType;
				if (board.rules.checkMove(piece, currentPos, toPos, obstacle, NULL, moveType)){
					if ( toPos.y == 0 || toPos.y == 7 )
					{
						moves[toPos] = evaluatePawnPromote(board, piece, obstPos);
					}
					else
					{
						moves[toPos] = evaluate(board, piece, obstPos);
					}
				}
			}
			else
			{
				//Can be an an-Passant. Attacking blank space
				Node *n = board.nodeHandler.getLastChild();
				if ( n != NULL && n->moveType == PURE_MOVE_ANPASSANT && n->movePiece.color != piece.color )
				{
					Vec2 obstPos = Vec2(n->movePiece.pos.x, (n->fromPos.y + (n->movePiece.pos.y-n->fromPos.y)/2));
					if ( (toPos.y == obstPos.y) && (toPos.x == obstPos.x))
					{
						moves[toPos] = evaluateAnPassant(board,piece,n->fromPos);
						return;
					}
				}
			}
		}
	}
}
void Engine::updateTotalMoves(Board &board){
	map<Piece, map<Vec2, int> >::iterator it;
	for (it = totalMoves.begin(); it != totalMoves.end(); ++it){
		if (!board.checkPieceExist(it->first)){
			totalMoves.erase(it++->first);
		}
	}
}
//See if an escape move is totally checked (Use color for opponent)
int Engine::checkIfCheck(Board &board, int opponentColor){

	Board * boardi = new Board();
	boardi = board.copyBoard();
	totalMoves.clear();
	createNodeBranches(*boardi, opponentColor);
	map<Piece, map<Vec2, int> > *tempTotalMoves = new map<Piece, map<Vec2, int> >();
	*tempTotalMoves = totalMoves;
	map<Piece, map<Vec2, int> >::iterator internalIterator = tempTotalMoves->begin();
	map<Vec2, int>::iterator innerIt;
	while (internalIterator != tempTotalMoves->end()){
		innerIt = internalIterator->second.begin();
		while(innerIt != internalIterator->second.end()){	
			if (boardi->pieces[innerIt->first].color == switchColor(opponentColor) && (boardi->pieces[innerIt->first].type == KING) ){
				innerIt++;
				return CHECK;
			}
			else innerIt++;
		}
		internalIterator++;
	}
	return NOT_CHECK;
}
//Checks if the opponent has user in check (Use color for opponent)
int Engine::checkIfCheckUser(Board &board, int opponentColor){

	tempBoard = board.copyBoard();
	createNodeBranches(*tempBoard, opponentColor);
	map<Piece, map<Vec2, int> > *tempTotalMoves = new map<Piece, map<Vec2, int> >();
	*tempTotalMoves = totalMoves;

	map<Piece, map<Vec2, int> >::iterator internalIterator = tempTotalMoves->begin();
	map<Vec2, int>::iterator innerIt;
	while (internalIterator != tempTotalMoves->end()){
		innerIt = internalIterator->second.begin();
		while(innerIt != internalIterator->second.end()){
			if (tempBoard->pieces[innerIt->first].color == switchColor(opponentColor) && tempBoard->pieces[innerIt->first].type == KING){
				innerIt++;
				return CHECK;
			}
			else innerIt++;
		}
		internalIterator++;
	}
	return NOT_CHECK;
}
//Here we use the color for the player that is checked!
int Engine::checkCheckMate(Board &board, int color){
	
	tempBoard = board.copyBoard();
	int result;
	PureMoveType hitTemp;
	Piece movePieceTemp;
	Vec2 tempTo;

	totalMoves.clear();
	createNodeBranches(*tempBoard, color);
	map<Piece, map<Vec2, int> > * tempTotalMoves = new map<Piece, map<Vec2, int> >();
	*tempTotalMoves = totalMoves;
	map<Piece, map<Vec2, int> >::iterator internalIterator = tempTotalMoves->begin();
	map<Vec2, int>::iterator innerIt;

	while (internalIterator != tempTotalMoves->end()){
		innerIt = internalIterator->second.begin();
		while(innerIt != internalIterator->second.end()){
			tempTo = innerIt->first;
			movePieceTemp = internalIterator->first;
			hitTemp = (PureMoveType)innerIt++->second;
			Vec2 hitPos;
			tempBoard->pureMove(movePieceTemp.pos, tempTo, hitTemp, hitPos); 
			result = checkIfCheck((Board&)*tempBoard, switchColor(color));
			if (result == NOT_CHECK){
				return NOT_CHECKMATE;
			}
			tempBoard->undoMove();
		}
		internalIterator++;
	}
	tempTotalMoves->clear();
	return CHECKMATE;
}
int Engine::switchColor(int color){
	if (color == WHITE)
		return BLACK;
	return WHITE;
}
