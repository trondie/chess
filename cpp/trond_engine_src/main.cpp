#if !defined( TROND_ENGINE_EXTERNAL_GUI )

#include <iostream>
#include <map>
#include <string>
#include "Board.h"
#include "Piece.h"
#include "Vec2.h"
#include "Engine.h"
#include <time.h>

using namespace std;
double difftime(clock_t clock1,clock_t clock2)
{
	double diff=clock1-clock2;
	double dms=(diff*1000)/CLOCKS_PER_SEC;
	return dms;
}

int main(){

Engine engine = Engine();
Board board = Board();
board.toString();
Vec2 temp;
Vec2 temp2;
Vec2 fromTemp1 = Vec2(4,0);
Vec2 toTemp1 = Vec2(1,2);
Vec2 fromTemp2 = Vec2(3,1);
Vec2 toTemp2 = Vec2(3,3);
Vec2 fromTemp3 = Vec2(1,6);
Vec2 toTemp3 = Vec2(4, 6);
Vec2 fromTemp4 = Vec2(3,6);
Vec2 toTemp4 = Vec2(3, 4);
Vec2 fromTemp5 = Vec2(3,7);
Vec2 toTemp5 = Vec2(0, 4);
Vec2 fromTemp6 = Vec2(4,6);
Vec2 toTemp6 = Vec2(4, 4);
//Vec2 fromTemp7 = Vec2(3,0);
//Vec2 toTemp7 = Vec2(4, 5);
int canMovePure = 0;
bool check = true;

board.pureMove(fromTemp1, toTemp1, true);
board.pureMove(fromTemp2, toTemp2, true);
board.pureMove(fromTemp3, toTemp3, true);
board.pureMove(fromTemp4, toTemp4, true);
board.pureMove(fromTemp5, toTemp5, true);
board.pureMove(fromTemp6, toTemp6, true);
//board.pureMove(fromTemp7, toTemp7, true);


while (true){
	if (canMovePure == 0){
		clock_t begin=clock();
		Vec2 * move = engine.startAlphaBetaPruning(engine.depth, ALPHA, BETA, board, WHITE);
		if (board.move(move[0], move[1])){
			cout << endl << "Moved from [" << move[0].x << ", " << move[0].y << "]" << " to " << "[" << move[1].x << ", " << move[1].y << "]" << endl;
			clock_t end = clock();
			cout << "Wall time : " << double(difftime(end, begin))/1000 << endl;
		}
		if (engine.checkIfCheckUser(board, BLACK) == CHECK){
			cout << "CHECK! Checking if checkmate . . ." << endl;
			if (engine.checkCheckMate(board, WHITE) == CHECKMATE){
				cout << "CHECKMATE!" << endl;
			}
			else{
				cout << "NOT CHECKMATE" << endl;
			}

		}
		delete move;
		board.toString();
	}
	if (canMovePure == 1){
		cout << "Enter the other pureMove : " << endl;
		canMovePure++;
	}
	cout << "Enter from x : ";
	cin >> temp.x;
	if (temp.x == 100){
		break;
	}
	else if (temp.x == -2){
		board.toString();
		continue;
	}
	else if (temp.x == -1){
		canMovePure++;
		cout << "Enter first pure move : " << endl;
		cout << "Enter from x : ";
		cin >> temp.x;
	}
	cout << endl << "Enter from y : ";
	cin >> temp.y;
	cout << endl << "Enter to x : ";
	cin >> temp2.x;
	cout << endl << "Enter to y : ";
	cin >> temp2.y;
	cout << endl << endl << endl << "Board is now : " << endl;
	if (canMovePure == 1 || canMovePure == 2){
		if (canMovePure == 2)
			canMovePure = 0;
		board.pureMove(temp, temp2, 1);
		continue;
	}
	else{
		bool check = board.move(temp, temp2);
		//engine.createNodeBranches(board, WHITE);
		//engine.toString();
		if (engine.checkIfCheckUser(board, BLACK) == CHECK){
			cout << "CHECK! Checking if checkmate . . ." << endl;
			if (engine.checkCheckMate(board, WHITE) == CHECKMATE){
				cout << "CHECKMATE!!!!!!!!!!!!!!!!!" << endl;
			}
			else{
				cout << "NOT CHECKMATE" << endl;
			}

		}
	}
	canMovePure = 0;

	if (!check)
		continue;
	board.toString();
	cout << endl;
	
}
bool turnk = true;
/*
while (true){

	if (turnk){
		system("pause");
		clock_t begin=clock();
		Vec2 * move = engine.startAlphaBetaPruning(engine.depth, ALPHA, BETA, board, WHITE);
		if (board.move(move[0], move[1])){
			cout << endl << "Moved from [" << move[0].x << ", " << move[0].y << "]" << " to " << "[" << move[1].x << ", " << move[1].y << "]" << endl;
			clock_t end = clock();
			cout << "Wall time : " << double(difftime(end, begin))/1000 << endl;
		}
		delete move;
		//board.toString();
	}
	else{
		clock_t begin=clock();
		Vec2 * move = engine.startAlphaBetaPruning(engine.depth, ALPHA, BETA, board, BLACK);
		if (board.move(move[0], move[1])){
			cout << endl << "Moved from [" << move[0].x << ", " << move[0].y << "]" << " to " << "[" << move[1].x << ", " << move[1].y << "]" << endl;
			clock_t end = clock();
			cout << "Wall time : " << double(difftime(end, begin))/1000 << endl;
		}
		delete move;
		//board.toString();
	}
	turnk = !turnk;

	//board.toString();
	cout << endl;
	
}*/

system("pause");
return 0;
}

#endif