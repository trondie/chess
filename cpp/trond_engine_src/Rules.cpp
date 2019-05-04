#include <iostream>
#include <cstdlib>
#include <map>
#include <string>
#include <math.h>
#include "Rules.h"
#include "Piece.h"
#include "Vec2.h"
#include "Board.h"
using namespace std;

Rules::Rules(){}

//Check if it's legal to move (almost) independent of obstacles
bool Rules::checkMove(Piece &piece, Vec2 &from, Vec2 &to, int &obstacle, Piece *pieceTo, int &moveType){
	
	Vec2 diff, destination;

	switch(piece.type){

		case PAWN:
		{
			diff = to-from;
			destination = from+diff;
			if (destination.x > 7 || destination.x < 0 || destination.y > 7 || destination.y < 0){
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			if (piece.color == WHITE){

				if ( (from.y < 7) && (to.y-from.y == 1) && ( ( ( (obstacle == BLACK) && abs(diff.x) == 1)) || ((obstacle == NOBS) && abs(diff.x == 0)) )){
					if( to.y == 0 || to.y == 7 ) moveType = (obstacle == NOBS) ? (int)PURE_MOVE_PAWN_PROMOTE : (int)PURE_MOVE_HIT_PAWN_PROMOTE;
					else moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				else if ( (obstacle == NOBS) && ((from.y == 1) && ( (to.y-from.y == 1) || (to.y-from.y == 2) ) && (to.x == from.x)) ){
					moveType = (int)PURE_MOVE_ANPASSANT;
					return true;
				}
			}
			else{
				if ( (from.y > 0) && (diff.y == -1) && ( ( ((obstacle==WHITE) && abs(diff.x) == 1)) || ((obstacle==NOBS) && abs(diff.x == 0)) ) ){
					if( to.y == 0 || to.y == 7 ) moveType = (obstacle == NOBS) ? (int)PURE_MOVE_PAWN_PROMOTE : (int)PURE_MOVE_HIT_PAWN_PROMOTE;
					else moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				else if ( (obstacle == NOBS) && ((from.y == 6) && ( (to.y-from.y == -1) || (to.y-from.y == -2) ) && (to.x == from.x)) ){
					moveType = (int)PURE_MOVE_ANPASSANT;
					return true;
				}
			}
			return false;
		}

		case ROOK:
		{
			diff = to-from;
			destination = from+diff;
			if (destination.x > 7 || destination.x < 0 || destination.y > 7 || destination.y < 0){
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			if (piece.color == WHITE){
				if ( NULL != pieceTo 
					&& pieceTo->firstMove 
					&& piece.firstMove 
					&& (pieceTo->color == WHITE) 
					&& (pieceTo->type == KING) 
					&& (destination.x == pieceTo->pos.x) 
					&& (destination.y == pieceTo->pos.y) )
				{
					moveType = (diff.x == 4) ? (int)PURE_MOVE_ROCADE_RKLONG : (int)PURE_MOVE_ROCADE_RKSHORT;
					return true;//Rocade
				}
				//if there is a vertical / horizontal move
				if ( ( (diff.x == 0) || (diff.y == 0) ) && ( (obstacle == BLACK) || (obstacle == NOBS) ) ) 
				{
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;				
			}
			else{
				if ( NULL != pieceTo
					&& piece.firstMove 
					&& pieceTo->firstMove
					&& (pieceTo->color == BLACK) 
					&& (pieceTo->type == KING) 
					&& (destination.x == pieceTo->pos.x) 
					&& (destination.y == pieceTo->pos.y) )
				{
					moveType = (diff.x == 4) ? (int)PURE_MOVE_ROCADE_RKLONG : (int)PURE_MOVE_ROCADE_RKSHORT;
					return true;//Rocade
				}
				//if there is a vertical / horizontal move
				if ( ( (diff.x == 0) || (diff.y == 0) ) && ( (obstacle == WHITE) || (obstacle == NOBS) ) ) 
				{
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			return false;
		}
		case BISHOP:
		{
			diff = to-from;
			destination = from+diff;
			if (destination.x > 7 || destination.x < 0 || destination.y > 7 || destination.y < 0){
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			
			if (piece.color == WHITE){
				if ( (abs(diff.x) == abs(diff.y)) && ( (obstacle == BLACK) || (obstacle == NOBS) ) )
				{
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			else{
				if ( (abs(diff.x) == abs(diff.y)) && ( (obstacle == WHITE) || (obstacle == NOBS) ) )
				{
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			
		}
		case QUEEN:
		{
			diff = to-from;
			destination = from+diff;
			if (destination.x > 7 || destination.x < 0 || destination.y > 7 || destination.y < 0){
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			
			if (piece.color == WHITE){
				if ( (abs(diff.x) == abs(diff.y))||( diff.x == 0 || diff.y == 0)  && ( (obstacle == BLACK) || (obstacle == NOBS) ))
				{
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			else{
				if ( (abs(diff.x) == abs(diff.y))||( diff.x == 0 || diff.y == 0) && ( (obstacle == WHITE) || (obstacle == NOBS) ))
				{
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
		}
		case KNIGHT:
		{
			diff = to-from;
			destination = from+diff;
			if (destination.x > 7 || destination.x < 0 || destination.y > 7 || destination.y < 0){
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			if (piece.color == WHITE){
				if ( ( ((abs(diff.x) == 2)&&(abs(diff.y) == 1)) || ((abs(diff.x)==1)&&(abs(diff.y)==2)) && ( (obstacle == BLACK) || (obstacle == NOBS) ))){
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = PURE_MOVE_ILLEGAL;
				return false;
			}
			else{
				if ( ( ((abs(diff.x) == 2)&&(abs(diff.y) == 1)) || ((abs(diff.x)==1)&&(abs(diff.y)==2)) && ( (obstacle == WHITE) || (obstacle == NOBS) )))
				{
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			moveType = (int)PURE_MOVE_ILLEGAL;
			return false;
		}
		case KING:
		{
			diff = to-from;
			destination = from+diff;
			if (destination.x > 7 || destination.x < 0 || destination.y > 7 || destination.y < 0){
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			if (piece.color == WHITE){
				if ( NULL != pieceTo 
					&& pieceTo->firstMove 
					&& piece.firstMove 
					&& (pieceTo->color == WHITE) 
					&& (pieceTo->type == ROOK) 
					&& (destination.x == pieceTo->pos.x) 
					&& (destination.y == pieceTo->pos.y) )
				{
					moveType = (diff.x == 4) ? (int)PURE_MOVE_ROCADE_KRLONG : (int)PURE_MOVE_ROCADE_KRSHORT;
					return true;//Rocade
				}
				if ( ((abs(diff.x) == 1 && abs(diff.y) == 1) || (abs(diff.x) == 1 && abs(diff.y) == 0) || (abs(diff.x) == 0 && abs(diff.y) == 1))
					&&(obstacle == BLACK || obstacle == NOBS) ){
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
			else{
				if ( NULL != pieceTo 
					&& pieceTo->firstMove 
					&& piece.firstMove 
					&& (pieceTo->color == BLACK) 
					&& (pieceTo->type == ROOK) 
					&& (destination.x == pieceTo->pos.x) 
					&& (destination.y == pieceTo->pos.y) )
				{
					moveType = (diff.x == 4) ? (int)PURE_MOVE_ROCADE_KRLONG : (int)PURE_MOVE_ROCADE_KRSHORT;
					return true;//Rocade
				}
				if ( ((abs(diff.x) == 1 && abs(diff.y) == 1) || (abs(diff.x) == 1 && abs(diff.y) == 0) || (abs(diff.x) == 0 && abs(diff.y) == 1))
					&&(obstacle == WHITE || obstacle == NOBS) ){
					moveType = (obstacle == NOBS) ? (int)PURE_MOVE_REGULAR : (int)PURE_MOVE_HIT;
					return true;
				}
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
			}
		}
		default:
		{
				moveType = (int)PURE_MOVE_ILLEGAL;
				return false;
		}
	}
	moveType = (int)PURE_MOVE_ILLEGAL;
	return false;
}

