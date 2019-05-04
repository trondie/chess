#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <iostream>
#include <map>
#include <string>
#include "Piece.h"
#include "Rules.h"
#include "constants.h"
#include "Vec2.h"
#include "Board.h"
#include "Node.h"
#include "NodeHandler.h"

using namespace std;
enum {NOT_CHECK, NOT_CHECKMATE, CHECKMATE, CHECK};
enum {HITMOVE};
//enum {CHECK, CHECMKATE, NOT_CHECK, NOT_CHECKMATE};
class DLL_EXPORT Engine {
public:

	//This is my temporary (?) solution for the possible moves for each piece. 
	//Not sure if this is a good way to map the moves yet.
	//It's simplifies alot in this way of thinking though!
	//Piece mapped to (position, evaluation()) pairs.
	//Update! : Should really be refactored into Piece, Vec2* pairs, because we evaluate nodes on the go in a-b pruning.
	//However, we can use this evaluation opportunity to evaluate something else.. But what? 
	map<Piece, map<Vec2, int> >	totalMoves;

	Rules						rules;
	Board *						tempBoard;
	NodeHandler					nodeHandler;
	int							turn;
	int							player;
	int							traceCounter;
	int							tracker;
	int                         depth; //(Level goes from 0 to depth-1 (last is evaluate(..))
	bool                        abortMove;

	Engine();
	Engine(int difficulty, bool useEngine, Board &board);

	void    abort() {abortMove=true;}
	void	toString();
	void	toString(Piece &piece);

	//Essentials
	Vec2 *	startAlphaBetaPruning(int depthLeft, int alpha, int beta, Board &board, int color);
	int  	alphaBetaPrune(Node * node, int depth, int alpha, int beta, Board &board, int color );
	int		evaluate(int color);
	int		evaluate(Node * node, Board &board);
	int		evaluate(Board &board, Piece &piece);
	int		evaluate(Board &board, Piece &piece, Vec2 &obstPos);
	int		evaluatePawnPromote(Board &board, Piece &piece);
	int		evaluatePawnPromote(Board &board, Piece &piece, Vec2 &obstPos);
	int		evaluateAnPassant(Board &board, Piece &piece);
	int		evaluateAnPassant(Board &board, Piece &piece, Vec2 &obstPos);
	int		evaluateRocade(Board &board, Piece &piece, Vec2 &obstPos,PureMoveType rocadeType);

	//Trace logic
	void	tracePossibleMoves(Piece &piece, Board &board);
	void	diagonalTrace(Piece &piece, Board &board, map<Vec2, int> &moves);
	void	straightTrace(Piece &piece, Board &board, map<Vec2, int> &moves);
	void	traceKnight(Piece &piece, Board &board, map<Vec2, int> &moves);
	void	tracePawn(Piece &piece, Board &board, map<Vec2, int> &moves);
	int		checkTrace(Board &board, bool escape);
	int		checkCheckMate(Board &board, int color);
	int		checkIfCheckUser(Board &board, int opponentColor);

	//Perform tracing
	void	rocadePerform( Vec2 &toPos, Vec2 &currentPos, Vec2 &obstPos, Piece &piece, Board &board, map<Vec2,int> &moves );
	void	straightDiagonalPerform(Vec2 &toPos, Vec2 &currentPos, Vec2 &obstPos, Piece &piece, Board &board, map<Vec2, int> &moves, int possiblePos, int inc1, int inc2);
	void	knightPerform(Vec2 &toPos, Vec2 &currentPos, Vec2 &obstPos, Piece &piece, Board &board, map<Vec2, int> &moves);
	void    pawnPerform(Vec2 &toPos, Vec2 &currentPos, Vec2 &obstPos, Piece &piece, Board &board, map<Vec2, int> &moves, int &obstacle, bool regular);
	void	updateTotalMoves(Board &board);

	//Temporary / Other
	map<Piece, map<Vec2, int> > copyTotalMoves();
	void	createNodeBranches(Board &board, int color);
	Node *	createNode(Node parentNode, Piece piece, Vec2 position);
	Node *  getRootNode();
	int		switchColor(int color);
	//To be removed
	void	test2(Board &board);
	int     previousMove();

private:
	int		checkIfCheck(Board &board, int opponentColor);

};

#endif
