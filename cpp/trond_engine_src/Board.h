#ifndef _BOARD_H_
#define _BOARD_H_

#include <iostream>
#include <cstdlib>
#include <map>
#include <string>
#include "Rules.h"
#include "Piece.h"
#include "constants.h"
#include "Node.h"
#include "NodeHandler.h"

#if defined( TROND_ENGINE_EXTERNAL_GUI )
#if PLATFORM_WINDOWS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif
#else
#define DLL_EXPORT
#endif

DLL_EXPORT typedef enum PureMoveType_t {
	PURE_MOVE_REGULAR,
	PURE_MOVE_HIT,
	PURE_MOVE_ROCADE_KRSHORT,
	PURE_MOVE_ROCADE_KRLONG,
	PURE_MOVE_ROCADE_RKSHORT,
	PURE_MOVE_ROCADE_RKLONG,
	PURE_MOVE_PAWN_PROMOTE,
	PURE_MOVE_ANPASSANT,
	PURE_MOVE_HIT_ANPASSANT,
	PURE_MOVE_HIT_PAWN_PROMOTE,
	PURE_MOVE_ILLEGAL,
} PureMoveType;
using namespace std;

/*
	The board..
*/

class DLL_EXPORT Board {

private:
	bool **				board;
public:
	Board();
	~Board();
	Rules				rules;
	map<Vec2, Piece>	pieces;
	int					turn;
	int					scoreWhite;
	int					scoreBlack;
	int					moveCount;
	NodeHandler			nodeHandler;

	Board	*	copyBoard();
	int         previousMove();
	int         GetValue( int type );
	bool        promotePawn(Vec2 &pos, int promotionType);
	void		replacePiece(Piece &oldPiece, Piece &newPiece);
	bool		move(Vec2 from, Vec2 to, Vec2 &hitPos); //Move from pos to loc. Check for obstacles, check if hit, check if rules apply
	void		pureMove(Vec2 from, Vec2 to, PureMoveType pureMove, Vec2 &hitPos); //Must be used by engine only!
	bool		undoMove();
	void		updateScore(int color, int value);
	int			getScore(int color);
	bool		moveCheck(Vec2 from, Vec2 to, bool &hitMove);
	void		toString(); 
	void		createPiece(int type, int value, int color, Vec2 pos, int rank);
	bool		obstacleCheck(Vec2 from, Vec2 to, Vec2 &pos); //Check obstacles from pos to and including location
	void		hitMove(Vec2 to); //Simply remove piece at location. 
	void		removePiece(Vec2 posd);
	void		insertPiece(Vec2 &posd, Piece &piece);
	bool		checkMateCheck(int color); //Not implemented
	bool		boundaryCheck(Vec2 from, Vec2 to);
	bool		checkPieceExist(const Piece &piece);	
	void        updateBools();
};

#endif
