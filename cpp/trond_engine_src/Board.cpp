#include <iostream>
#include <map>
#include <string>
#include "Board.h"
#include "Piece.h"
#include "Rules.h"
#include "Vec2.h"
//#include "Engine.h"
#include <algorithm>
#include "Node.h"
#include "NodeHandler.h"

using namespace std;

Board::Board() {
	/**/
	Node * root = new Node();
	nodeHandler = NodeHandler(root);
	scoreWhite  = 0;
	scoreBlack  = 0;
	/**/
	int i, cx;
	string pieceString;
	rules		= Rules();

	//Initialize board
	board = (bool**)calloc(TE_SIZE, sizeof(bool*));
	for (i = 0; i < TE_SIZE; i++)
		board[i] = (bool*)calloc(TE_SIZE, sizeof(bool));

	//Initialize white pieces
	cx = 0;
	for (i = 0; i < TE_SIZE; i++){
		createPiece(PAWN, GetValue(PAWN), WHITE, Vec2(cx++,1), i);
	}
	createPiece(ROOK, GetValue(ROOK), WHITE, Vec2(0,0),8);
	createPiece(KNIGHT, GetValue(KNIGHT), WHITE, Vec2(1,0),9);
	createPiece(BISHOP, GetValue(BISHOP), WHITE, Vec2(2,0),10);
	createPiece(QUEEN, GetValue(QUEEN), WHITE, Vec2(3,0),11);
	createPiece(KING, GetValue(KING), WHITE, Vec2(4,0),12);
	createPiece(BISHOP, GetValue(BISHOP), WHITE, Vec2(5,0),13);
	createPiece(KNIGHT, GetValue(KNIGHT), WHITE, Vec2(6,0),14);
	createPiece(ROOK, GetValue(ROOK), WHITE, Vec2(7,0),15);

	//Black pieces
	cx = 0;
	for (i = 0; i < TE_SIZE; i++){
		createPiece(PAWN, GetValue(PAWN), BLACK, Vec2(cx++,6), i);
	}
	createPiece(ROOK, GetValue(ROOK), BLACK, Vec2(0,7), 16);
	createPiece(KNIGHT, GetValue(KNIGHT), BLACK, Vec2(1,7), 17);
	createPiece(BISHOP, GetValue(BISHOP), BLACK, Vec2(2,7), 18);
	createPiece(QUEEN, GetValue(QUEEN), BLACK, Vec2(3,7), 19);
	createPiece(KING, GetValue(KING), BLACK, Vec2(4,7), 20);
	createPiece(BISHOP, GetValue(BISHOP), BLACK, Vec2(5,7),21);
	createPiece(KNIGHT, GetValue(KNIGHT), BLACK, Vec2(6,7), 22);
	createPiece(ROOK, GetValue(ROOK), BLACK, Vec2(7,7), 23);
}

Board::~Board(){
	for (int i = 0; i < TE_SIZE; i++)
		delete [] board[i];
	delete board;
}
Board * Board::copyBoard(){

	Board * newBoard = new Board();
	map<Vec2, Piece>::iterator it		= newBoard->pieces.begin();
	map<Vec2, Piece>::iterator itThis	= pieces.begin();

	while(it != newBoard->pieces.end())
		newBoard->removePiece(it++->first);
	while(itThis != pieces.end()){ 
		newBoard->insertPiece((Vec2&)itThis->first, itThis->second);
		itThis++;
	}
	Node *n = nodeHandler.getLastChild();
	/* we need to store last move for anpassant evaluations to work. */
	if ( NULL != n && n->moveType == PURE_MOVE_ANPASSANT)
	{
		newBoard->nodeHandler.getLastChild()->movePiece = n->movePiece;
		newBoard->nodeHandler.getLastChild()->hitPiece = n->hitPiece;
		newBoard->nodeHandler.getLastChild()->fromPos = n->fromPos;
		newBoard->nodeHandler.getLastChild()->moveType = n->moveType;
	}
	newBoard->scoreBlack = scoreBlack;
	newBoard->scoreWhite = scoreWhite;
	return newBoard;
}
void Board::toString(){
	Vec2 temp;
	for (int i = TE_SIZE-1; i >= 0; i--){
		cout << endl;
		for (int j = 0; j < TE_SIZE; j++){
			if (board[j][i]){
				temp.x = j;
				temp.y = i;
				//cout << "[" << pieces[temp].value << "]" ;
				//cout << "[" << pieces[temp].type << "]" ;
				//cout << "[" << board[j][i] << "]";
				if (pieces[temp].type == KING){
					if (pieces[temp].color == BLACK){
						cout << "[B: -1]" ;
					}
					else{
						cout << "[W: -1]";
					}
				}
				else{
					if (pieces[temp].color == BLACK){
						cout << "[B: "<<pieces[temp].value<<" ]";
					}
					else{
						cout << "[W: " << pieces[temp].value<<" ]";
					}
				}
			}
			else{

				cout << "[  " << board[j][i] << "  ]"; 
			}
		}
	}
	cout << endl << "SCORE : [White : " << scoreWhite << ", Black : " << scoreBlack << "]" << endl;
	for (map<Vec2, Piece>::iterator it = pieces.begin(); it != pieces.end(); ++it){
		int value =  it->second.color + it->second.value + (it->second.type+1) +it->second.rank*10;
		for (map<Vec2, Piece>::iterator it2 = pieces.begin(); it2 != pieces.end(); ++it2){
			if (it == it2)
				continue;
			if (value == (it2->second.color + it2->second.value + (it2->second.type+1) +it2->second.rank*10)){
				cout << "AMBIGUITY! :" << value << endl ;
				system("pause");
			}
		}
	}
	
}
void Board::createPiece(int type, int value, int color, Vec2 pos, int rank){
	pieces[pos] = Piece(type, value, color, pos, rank);
	board[pos.x][pos.y] = true;

}

int Board::GetValue( int type )
{
	switch( type )
	{
	case KING: return 9000;
	case QUEEN: return 40;
	case ROOK: return 5;
	case KNIGHT: return 5;
	case BISHOP: return 3;
	case PAWN: return 1;
	}
	return 0;
}

bool Board::promotePawn(Vec2 &pos, int promotionType)
{
	if ( false == board[pos.x][pos.y] 
		|| !(QUEEN == promotionType 
			|| BISHOP == promotionType 
			|| KNIGHT == promotionType
			|| ROOK == promotionType ) )
	{
		return false;
	}
	Piece p = pieces[pos];
	p.type = promotionType;
	p.value = GetValue(promotionType);
	removePiece(pos);
	insertPiece(pos, p);
	/*nodeHandler.addChild(&p, NULL, pos);*/
	return true;
}

int Board::previousMove()
{
	return nodeHandler.getLastChild()->moveType;
}

//Checks if obstacle and if that obstacle result in a hit move. If not: regular move. 
bool Board::move(Vec2 from, Vec2 to, Vec2 &hitPos ){
	cout << endl << "Trying to move from : " ;
	from.toString();
	cout << "to : " ;
	to.toString();
	cout << endl;
	if (!board[from.x][from.y])
		return false;

	if ( from.x == to.x && from.y == to.y )
		return false;

	Vec2 pos = Vec2(0,0);
	Piece temp, temp2;
	int obstacle = NOBS;

	if (obstacleCheck(from, to, pos)) {
		//If piece is trying to hit another
		if (pos == to){
			//Check if piece is trying to hit oppnonent piece
			if (pieces[from].color != pieces[to].color){
				//Store the color of the obstacle to compare it regarding hit
				obstacle = pieces[pos].color;
				cout << "About to check checkmove with obst";
				//Check if move is legal
				int moveType;
				if (rules.checkMove(pieces[from], from, to, obstacle, NULL, moveType)){
					temp = pieces[from];
					temp2 = pieces[to];
					removePiece(from);
					hitMove(to);
					hitPos = to;
					temp.firstMove = false;
					insertPiece(to, temp);					
					nodeHandler.addChild(&temp, &temp2, from);
					nodeHandler.lastChild->moveType = moveType;
					updateScore(temp.color, temp2.value);
					return true; 
				}
				return false;
			}
			if( pieces[from].type == ROOK && board[to.x][to.y] && pieces[to].type == KING && pieces[from].firstMove && pieces[to].firstMove ) 
			{
				int moveType;
				if ( rules.checkMove(pieces[from], from, to, obstacle, &pieces[to], moveType))
				{
					cout << "performing rook-king ";
					temp = pieces[from];
					temp2 = pieces[to];
					hitPos = to;
					removePiece(to);
					removePiece(from);
					if ( to.x > from.x )
					{
						cout << "long rocade" << endl;
						to.x -= 2;
						from.x += 3;
					}
					else
					{
						cout << "short rocade" << endl;
						to.x += 2;
						from.x -= 2;
					}
					temp.firstMove = false;
					temp2.firstMove = false;
					insertPiece(from, temp);
					insertPiece(to, temp2);
					nodeHandler.addChild(&temp, NULL, from);
					nodeHandler.addChild(&temp2, NULL, to);
					nodeHandler.lastChild->moveType = moveType;
					return true;
				}
				return false;
			}
			else if (pieces[from].type == KING && board[to.x][to.y] && pieces[to].type == ROOK && pieces[from].firstMove && pieces[to].firstMove )
			{
				int moveType;
				if ( rules.checkMove(pieces[from], from, to, obstacle, &pieces[to], moveType))
				{
					cout << "performing king-rook ";
					temp = pieces[from];
					temp2 = pieces[to];
					hitPos = to;
					removePiece(to);
					removePiece(from);
					if ( from.x > to.x )
					{
						cout << "long rocade" << endl;
						from.x -= 2;
						to.x += 3;
					}
					else
					{
						cout << "short rocade" << endl;
						from.x += 2;
						to.x -= 2;
					}
					temp.firstMove = false;
					temp2.firstMove = false;
					insertPiece(from, temp);
					insertPiece(to, temp2);
					nodeHandler.addChild(&temp, NULL, from);
					nodeHandler.addChild(&temp2, NULL, to);
					nodeHandler.lastChild->moveType = moveType;
					return true;
				}
				return false;
			}
			cout << "Can't kill your own kind." << endl;
			//Same opponent
			return false;
		}
		cout << "Something's in the way." << endl;
		//Obstacle between desired position and current
		return false;
	}
	//If no obstacles
	else{
		Node *n = this->nodeHandler.getLastChild();
		if ( NULL != n && n->moveType == PURE_MOVE_ANPASSANT )
		{
			Vec2 anPassantTo = n->movePiece.pos;
			Vec2 anPassantPos = n->fromPos - Vec2((n->fromPos.x - n->movePiece.pos.x)/2,(n->fromPos.y - n->movePiece.pos.y)/2);
			if ( to.x == anPassantPos.x && to.y == anPassantPos.y )
			{
				temp = pieces[from];
				temp2 = pieces[anPassantTo];
				removePiece(from);
				removePiece(anPassantTo);
				hitPos = anPassantTo;
				insertPiece(to, temp);
				nodeHandler.addChild(&temp, &temp2, from);	
				updateScore(temp.color, temp2.value);
				nodeHandler.getLastChild()->moveType = PURE_MOVE_HIT_ANPASSANT;	
				return true;
			}
		}
		int moveType;
		if (rules.checkMove(pieces[from], from, to, obstacle, NULL, moveType)){
			cout << "performing move" << endl;
			temp = pieces[from];
			removePiece(from);
			temp.firstMove = false;
			insertPiece(to, temp);
			nodeHandler.addChild(&temp, NULL, from);
			nodeHandler.getLastChild()->moveType = moveType;	
			return true;
		}
	}
	cout << "Illegal move!";
	return false;
	
}
bool Board::undoMove(){
	Node * lastMove = nodeHandler.getLastChild();
	if (lastMove->parentNode == NULL)
		return false;
	
	bool retval = false;
	Vec2 fromPos = lastMove->movePiece.pos;
	Vec2 toPos   = lastMove->fromPos;
	Piece temp = lastMove->movePiece;
	removePiece(fromPos);

	if (PURE_MOVE_HIT == lastMove->moveType)
	{
		insertPiece(toPos, temp);
		insertPiece(fromPos, lastMove->hitPiece);
		updateScore(temp.color, -lastMove->hitPiece.value);
		retval = nodeHandler.removeLastChild();
	} 
	else if ( PURE_MOVE_PAWN_PROMOTE == lastMove->moveType )
	{
		updateScore(temp.color, GetValue(PAWN) - temp.value);
		temp.type = PAWN;
		temp.value = GetValue(PAWN);
		insertPiece(toPos, temp);
		retval = nodeHandler.removeLastChild();
	}
	else if ( PURE_MOVE_HIT_PAWN_PROMOTE == lastMove->moveType )
	{
		updateScore(temp.color, GetValue(PAWN) - temp.value - lastMove->hitPiece.value);
		temp.type = PAWN;
		temp.value = GetValue(PAWN);
		insertPiece(toPos, temp);
		insertPiece(fromPos, lastMove->hitPiece);
		retval = nodeHandler.removeLastChild();
	}
	else if ( PURE_MOVE_ANPASSANT == lastMove->moveType )
	{
		insertPiece(toPos, temp);
		retval = nodeHandler.removeLastChild();
	}
	else if ( PURE_MOVE_HIT_ANPASSANT == lastMove->moveType )
	{
		insertPiece(toPos, temp);
		fromPos.y += ( fromPos.y-lastMove->hitPiece.pos.y );
		insertPiece(fromPos, lastMove->hitPiece);
		updateScore(temp.color, -lastMove->hitPiece.value);
		retval = nodeHandler.removeLastChild();
	}
	else if ( PURE_MOVE_REGULAR == lastMove->moveType )
	{
		insertPiece(toPos, temp);
		retval = nodeHandler.removeLastChild();
	}
	else 
	{
		insertPiece(toPos, temp);
		retval = nodeHandler.removeLastChild();
		{
			Node * last2ndMove = nodeHandler.getLastChild();
			if (last2ndMove->parentNode == NULL)
				return false;
	
			Vec2 fromPos2nd = last2ndMove->movePiece.pos;
			Vec2 toPos2nd   = last2ndMove->fromPos;
			Piece temp2nd   = last2ndMove->movePiece;
			removePiece(fromPos2nd);
			insertPiece(toPos2nd, temp2nd);
			retval &= nodeHandler.removeLastChild();
		}
	}
	return retval;
}
//Used by engine. The program will crash if we make an illegal move - but, the move taken from the traced moves are already checked.
void Board::pureMove(Vec2 from, Vec2 to, PureMoveType pureMove, Vec2 &hitPos){
	Piece temp = pieces[from], temp2;
	if (PURE_MOVE_REGULAR == pureMove){
		removePiece(from);
		temp.firstMove = false;
		insertPiece(to, temp);
		nodeHandler.addChild(&temp, NULL, from);
		hitPos = Vec2(-1,-1);
	}
	else if(PURE_MOVE_ROCADE_KRSHORT == pureMove )
	{
		removePiece(from);
		temp.firstMove = false;
		Vec2 newfrom = Vec2( from.x + 2, from.y );
		insertPiece(newfrom, temp);
		nodeHandler.addChild(&temp, NULL, from);
		nodeHandler.lastChild->moveType = PURE_MOVE_ROCADE_KRSHORT;
		temp2 = pieces[to];
		removePiece(to);
		temp2.firstMove = false;
		Vec2 newTo = Vec2( to.x - 2, to.y );
		insertPiece(newTo, temp2);
		nodeHandler.addChild(&temp2, NULL, to);
		nodeHandler.lastChild->moveType = PURE_MOVE_ROCADE_KRSHORT;
		hitPos = Vec2(-1,-1);
	} 
	else if(PURE_MOVE_ROCADE_KRLONG == pureMove )
	{
		temp2 = pieces[to];
		removePiece(from);
		removePiece(to);
		temp2.firstMove = false;
		temp.firstMove = false;
		Vec2 newFrom = Vec2( from.x - 2, from.y );
		Vec2 newTo = Vec2( to.x + 3, to.y );
		insertPiece(from, temp);
		insertPiece(to, temp2);
		nodeHandler.addChild(&temp, NULL, from);
		nodeHandler.lastChild->moveType = PURE_MOVE_ROCADE_KRLONG;
		nodeHandler.addChild(&temp2, NULL, to);
		nodeHandler.lastChild->moveType = PURE_MOVE_ROCADE_KRLONG;
		hitPos = Vec2(-1,-1);
	} 
	else if(PURE_MOVE_ROCADE_RKSHORT == pureMove )
	{
		temp2 = pieces[to];
		removePiece(from);
		removePiece(to);
		temp2.firstMove = false;
		temp.firstMove = false;
		Vec2 newFrom = Vec2( from.x - 2, from.y );
		Vec2 newTo = Vec2( to.x + 2, to.y );
		insertPiece(newFrom, temp);
		insertPiece(newTo, temp2);
		nodeHandler.addChild(&temp, NULL, from);
		nodeHandler.lastChild->moveType = PURE_MOVE_ROCADE_RKSHORT;
		nodeHandler.addChild(&temp2, NULL, to);
		nodeHandler.lastChild->moveType = PURE_MOVE_ROCADE_RKSHORT;
		hitPos = Vec2(-1,-1);
	} 
	else if(PURE_MOVE_ROCADE_RKLONG == pureMove )
	{
		temp2 = pieces[to];
		removePiece(from);
		removePiece(to);
		temp2.firstMove = false;
		temp.firstMove = false;
		Vec2 newFrom = Vec2( from.x + 3, from.y );
		Vec2 newTo = Vec2( to.x - 2, to.y );
		insertPiece(newFrom, temp);
		insertPiece(newTo, temp2);
		nodeHandler.addChild(&temp, NULL, from);
		nodeHandler.lastChild->moveType = PURE_MOVE_ROCADE_RKLONG;
		nodeHandler.addChild(&temp2, NULL, to);
		nodeHandler.lastChild->moveType = PURE_MOVE_ROCADE_RKLONG;
		hitPos = Vec2(-1,-1);
	} 
	else if ( PURE_MOVE_HIT == pureMove ) 
	{
		temp2 = pieces[to];
		removePiece(from);
		hitMove(to);
		hitPos = to;
		temp.firstMove = false;
		insertPiece(to, temp);
		nodeHandler.addChild(&temp, &temp2, from);	
		updateScore(temp.color, temp2.value);		
	}
	else if ( PURE_MOVE_PAWN_PROMOTE == pureMove )
	{
		removePiece(from);
		temp.type = QUEEN;
		temp.value = GetValue(QUEEN);
		insertPiece(to, temp);
		nodeHandler.addChild(&temp, NULL, from);
		nodeHandler.lastChild->moveType = PURE_MOVE_PAWN_PROMOTE;
		updateScore(temp.color, temp.value-GetValue(PAWN));
		hitPos = Vec2(-1,-1);
	}
	else if ( PURE_MOVE_HIT_PAWN_PROMOTE == pureMove )
	{
		temp2 = pieces[to];
		removePiece(from);
		hitMove(to);
		hitPos = to;
		temp.firstMove = false;
		temp.type = QUEEN;
		temp.value = GetValue( QUEEN );
		insertPiece(to, temp);
		nodeHandler.addChild(&temp, &temp2, from);	
		nodeHandler.lastChild->moveType = PURE_MOVE_HIT_PAWN_PROMOTE;
		updateScore(temp.color, temp2.value+temp.value-GetValue(PAWN));
	}
	else if ( PURE_MOVE_ANPASSANT == pureMove )
	{
		removePiece(from);
		temp.firstMove = false;
		insertPiece(to, temp);
		nodeHandler.addChild(&temp, NULL, from);
		nodeHandler.lastChild->moveType = PURE_MOVE_ANPASSANT;
		hitPos = Vec2(-1,-1);
	}
	else if ( PURE_MOVE_HIT_ANPASSANT == pureMove )
	{
		Node *n = nodeHandler.getLastChild();
		temp = pieces[from];
		temp2 = pieces[n->movePiece.pos];
		removePiece(from);
		removePiece(n->movePiece.pos);
		hitPos = n->movePiece.pos;
		Vec2 moveToPos = Vec2(n->movePiece.pos.x, (n->fromPos.y + (n->movePiece.pos.y-n->fromPos.y)/2));
		insertPiece(moveToPos, temp);
		nodeHandler.addChild(&temp, &temp2, from);	
		nodeHandler.lastChild->moveType = PURE_MOVE_HIT_ANPASSANT;
		updateScore(temp.color, temp2.value);
	}
	else
	{
		cout << "Something is wrong for puremove " << pureMove << endl;
	}
}
void Board::updateScore(int color, int value){
	if (color == WHITE){
		scoreWhite += value;
	}
	else{
		scoreBlack += value;
	}
}
int Board::getScore(int color){
	if (color == WHITE){
		return scoreWhite;
	}
	else{
		return scoreBlack;
	}
}
bool Board::moveCheck(Vec2 from, Vec2 to, bool &hitMove){

	if (!board[from.x][from.y])
		return false;

	Vec2 pos		= Vec2(0,0);
	int obstacle	= NOBS;
	hitMove			= false;
	int moveType;
	if (obstacleCheck(from, to, pos)) {
		//If piece is trying to hit another
		if (pos == to){
			//Check if piece is trying to hit oppnonent piece
			if (pieces[from].color != pieces[to].color){
				//Store the color of the obstacle to compare it regarding hit
				obstacle = pieces[pos].color;
				//Check if move is legal
				if (rules.checkMove(pieces[from], from, to, obstacle, NULL, moveType)){
					hitMove = true;
					return true; 
				}
				return false;
			}
			return false;
		}
		return false;
	}
	//If no obstacles
	else{
		if (rules.checkMove(pieces[from], from, to, obstacle, NULL, moveType)){
			return true;
		}
	}
	return false;
	
}

//Gives obstacle position, independent of end position or a position in between
bool Board::obstacleCheck(Vec2 from, Vec2 to, Vec2 &pos){
	//Difference (Direction)
	Vec2 diff = to-from;
	//If x,y are equal, then it's a diagonal move
	if (abs(diff.x) == abs(diff.y)){
		//For each tile between start position to and including destination pos
		for (int i = 1; i <= abs(diff.x); i++){
			if (board[from.x+( (diff.x/abs(diff.x))*i )][from.y+( (diff.y/abs(diff.y))*i )]){
				pos.x = from.x+( (diff.x/abs(diff.x))*i );
				pos.y = from.y+( (diff.y/abs(diff.y))*i );
				return true;
			}
		}
	}
	//If horizontal/vertical move
	else if (( (diff.x == 0) || (diff.y == 0) )){
		if (diff.x == 0){
			for (int i = 1; i <= abs(diff.y); i++){
				if (board[from.x][from.y+( (diff.y/abs(diff.y))*i )]){
					pos.y = from.y+( (diff.y/abs(diff.y))*i );
					pos.x = from.x;
					return true;
				}
			}
		}
		else{
			for (int i = 1; i <= abs(diff.x); i++){
				if (board[from.x+( (diff.x/abs(diff.x))*i )][from.y]){
					pos.y = from.y;
					pos.x = from.x+( (diff.x/abs(diff.x))*i );
					return true;
				}
			}
		}
	}
	//To cover knight's move. The only possible obstacle is at destination.
	else if ( ( (abs(diff.x) == 1) && (abs(diff.y) == 2) ) || ( (abs(diff.x)==2) && (abs(diff.y)==1)) ){
		if (board[to.x][to.y]){
			pos.x = to.x;
			pos.y = to.y;
			return true;
		}
		return false;
	}
	return false;
}

void Board::hitMove(Vec2 posd){
	pieces.erase(posd);
}

void Board::removePiece(Vec2 posd){
	pieces.erase(posd);
	board[posd.x][posd.y] = false;
}

void Board::insertPiece(Vec2 &posd, Piece &piece){
	piece.pos = posd;
	pieces[posd] = piece;
	board[posd.x][posd.y] = true;
}
//Nada
bool Board::checkMateCheck(int color){
	switch (color){
		case WHITE:
		{
			return false;
		}
		case BLACK:
		{
			return false;
		}
	}
	return false;
}

bool Board::boundaryCheck(Vec2 from, Vec2 to){
	Vec2 diff, destination;
	diff = to-from;
	destination = from+diff;
	if (destination.x > 7 || destination.x < 0 || destination.y > 7 || destination.y < 0){
		return false;
	}
	return true;
}

void Board::updateBools()
{
	map<Vec2, Piece>::iterator it;
	for ( int x = 0; x < 8; x++ )
	{
		for ( int y = 0; y < 8; y++ )
		{
			board[x][y] = false;
		}
	}

	for (it = pieces.begin(); it != pieces.end(); ++it)
	{
		board[it->second.pos.x][it->second.pos.y] = true;
	}
}


bool Board::checkPieceExist(const Piece &piece){
	map<Vec2, Piece>::iterator it;
	for (it = pieces.begin(); it != pieces.end(); ++it){
		if (it->second == piece){
			return true;
		}
	}
	return false;
}