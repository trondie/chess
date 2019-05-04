#include <tinyxml2.h>
#include <Board.h>
#include <Engine.h>
#include <Vec2.h>

#include "os_helpers.h"
#include "TestGLSL.h"
#include "chess_helpers.h"
#include "mymath.h"
#include "ee_scene_gl.h"
#include "mythreading.h"
#include "ee_camera.h"
#include "main.h"

#if PLATFORM_ARM_ANDROID
#include "../native-lib.h"
#endif

/* Chess state */
struct chess_engine_thread_data
{
	Board *chessboard;
	Engine *chessengine;
	Vec2 *move;
	int depth;
	bool cpu_is_white;
};

static thread_handle chess_engine_thread = THREADING_INVALID_HANDLE;
mutex_handle chessboard_mutex;
#if defined(NO_DEBUG)
static int chess_engine_depth = 5;
#else
static int chess_engine_depth = 3;
#endif

typedef enum
{
	TYPE_CPU,
	TYPE_PLAYER,
	TYPE_PLAYER_EXTERNAL,
} player_type;

player_type white_player = TYPE_PLAYER;
#if PLATFORM_ARM_ANDROID
player_type black_player = TYPE_CPU;
#else
player_type black_player = TYPE_PLAYER;
#endif
static bool white_checkmate = false;
static bool black_checkmate = false;
static bool white_check = false;
static bool black_check = false;
static int numMoves = 0;

Vec2 posFrom = Vec2(-1,-1);
Board *chessboard = NULL;
Engine *chessengine = NULL;

void set_white_is_cpu()
{
	white_player = TYPE_CPU;
}

void set_black_is_cpu()
{
	black_player = TYPE_CPU;
}

void set_white_is_player()
{
	white_player = TYPE_PLAYER;
}

void set_black_is_player()
{
	black_player = TYPE_PLAYER;
}

void set_white_is_external_player()
{
	white_player = TYPE_PLAYER_EXTERNAL;
}

void set_black_is_external_player()
{
	black_player = TYPE_PLAYER_EXTERNAL;
}

void set_engine_search_depth(int depth)
{
	chess_engine_depth = depth;
}

Vec2 geometryInstanceToChessPos( EE_GeometryInstance *p )
{
	Vec2 retval;
	retval.x = int( (ROUND(p->matrix[13])/get_scene()->getSnapToGrid().x) ) + 4;
	retval.y = 7 - int(ROUND((p->matrix[12])/get_scene()->getSnapToGrid().y) );
	return retval;
}

myvec2 chessToGeometryInstancePos( Vec2 chessPos )
{
	myvec2 retval;
	retval.x = (7-chessPos.y)*get_scene()->getSnapToGrid().x;
	retval.y = (chessPos.x-4)*get_scene()->getSnapToGrid().y;	
	return retval;
}


bool is_chess_piece( const char *name )
{
	return ((0 == strncmp( name, "king.X_instance", 15 )) || 
			(0 == strncmp( name, "queen.X_instance", 16 )) || 
			(0 == strncmp( name, "bishop_b2.X_instance", 20 )) ||
			(0 == strncmp( name, "knight.X_instance", 17 )) ||
			(0 == strncmp( name, "rook_b1.X_instance", 18 )) ||
			(0 == strncmp( name, "pawn.X_instance", 15 )) );
}

bool chess_piece_is_white( const char *name )
{
	return ((0 == strncmp( name, "king.X_instance", 15 ) && (name[15] - '0') < 1) || 
			(0 == strncmp( name, "queen.X_instance", 16 ) && (((name[16] - '0')&1) == 0)) || 
			(0 == strncmp( name, "bishop_b2.X_instance", 20 ) && (name[20] - '0') < 2) ||
			(0 == strncmp( name, "knight.X_instance", 17 ) && (name[17] - '0') < 2) ||
			(0 == strncmp( name, "rook_b1.X_instance", 18 ) && (name[18] - '0') < 2) ||
			(0 == strncmp( name, "pawn.X_instance", 15 ) && (atoi(&name[15]) < 8) ));
}

static void *chess_engine_thread_func(thread_handle thread)
{
	struct chess_engine_thread_data *data = (struct chess_engine_thread_data *)get_thread_data(thread);
	Base::log("data: %p data->chessboard: %p", data, data->chessboard );
	grab_mutex( chessboard_mutex );
	data->move = chessengine->startAlphaBetaPruning(data->depth, ALPHA, BETA, *data->chessboard, (data->cpu_is_white ? WHITE : BLACK ) );
	release_mutex( chessboard_mutex );
	signal_exit(thread);
	return NULL;
}

bool cpu_is_white()
{
	return white_player == TYPE_CPU;
}

bool cpu_is_black()
{
	return black_player == TYPE_CPU;
}

bool white_can_move()
{
	return !(numMoves & 1);
}

bool black_can_move()
{
	return (numMoves & 1);
}

bool cpu_can_move()
{
	if (cpu_is_white() && white_can_move())
		return true;
	if (cpu_is_black() && black_can_move())
		return true;
	return false;
}

bool player_is_white()
{
	return white_player == TYPE_PLAYER;
}

bool player_is_black()
{
	return black_player == TYPE_PLAYER;
}

bool player_can_move()
{
	if (player_is_white() && !(numMoves & 1))
		return true;
	if (player_is_black() && (numMoves & 1))
		return true;
	return false;
}

bool player_external_is_white()
{
	return (white_player == TYPE_PLAYER_EXTERNAL);
}

bool player_external_is_black()
{
	return (black_player == TYPE_PLAYER_EXTERNAL);
}

bool player_external_can_move()
{
	if (player_external_is_white() && !(numMoves & 1))
		return true;
	if (player_external_is_black() && (numMoves & 1))
		return true;
	return false;
}
bool check_chess_engine_done( vector<EE_GeometryInstance *> &all_instances, vector<EE_Geometry *> &all_geometries, tinyxml2::XMLDocument *xmldocument )
{
	bool opponent_in_checkmate = false;
	EE_GeometryInstance *castling = NULL;
	EE_GeometryInstance *found = NULL;

	//Bail out in case of checkmates
	if ( white_can_move() && white_checkmate ) return true;
	if ( black_can_move() && black_checkmate ) return true;

	if ( THREADING_INVALID_HANDLE == chess_engine_thread )
	{
		grab_mutex( chessboard_mutex );

		if ( NULL == chessboard )
		{
			chessboard = new Board();
			chessengine = new Engine();
			chessboard->updateBools();
			Base::log("chessengine created: %p", chessengine);
		}
		if ( cpu_can_move() )
		{
			chess_engine_thread_data *thread_data = new chess_engine_thread_data();
			thread_data->chessboard = chessboard;
			thread_data->chessengine = chessengine;
			thread_data->cpu_is_white = !(numMoves & 1);
			thread_data->depth = chess_engine_depth;
			chess_engine_thread = create_thread(chess_engine_thread_func,thread_data);
			Base::log("thread:%d startAlphaBetaPruning for data:%p chessboard:%p\n", chess_engine_thread, thread_data, thread_data->chessboard);
			release_mutex(chessboard_mutex);
			return false;
		}

		release_mutex(chessboard_mutex);

		return true;
	}
	if ( !exit_is_signalled(chess_engine_thread) )
	{
		return false;
	}

	join_thread(chess_engine_thread);
	chess_engine_thread_data *data = (chess_engine_thread_data*)get_thread_data(chess_engine_thread);
	destroy_thread(chess_engine_thread);
	chess_engine_thread = THREADING_INVALID_HANDLE;

	Base::log( "Suggested move from (%d,%d) to (%d,%d) data:%p chessengine:%p\n", data->move[0].y, data->move[0].x, data->move[1].y, data->move[1].x, data, data->chessengine );
	Base::log( "check_chess_engine_done: chessboard: %p", chessboard );

	grab_mutex( chessboard_mutex );

	if ( NULL == chessboard )
	{
		chessboard = new Board();
		chessengine = new Engine();
		chessboard->updateBools();
		Base::log("chessengine created: %p", chessengine);
		return true;
	}

	numMoves += 1;

	for ( int i = 0; i < all_instances.size(); i++ )
	{
		Vec2 pos = geometryInstanceToChessPos( all_instances[i] );

		if (data->move[0].x == pos.x && data->move[0].y == pos.y && all_instances[i]->enabled && is_chess_piece( all_instances[i]->getName() ) )
		{
			Vec2 hitPos = Vec2(-1,-1);

			chessboard->pureMove( data->move[0], data->move[1], (PureMoveType)chessengine->previousMove(), hitPos );

			if ( hitPos.x >= 0 && hitPos.y >= 0 ) 
			{
				for ( int j = 0; j < all_instances.size(); j++ )
				{
					Vec2 checkPos = geometryInstanceToChessPos( all_instances[j] );
					
					if ((hitPos.x == checkPos.x && hitPos.y == checkPos.y) &&
						(all_instances[j] != all_instances[i]) &&
						all_instances[j]->enabled )
					{
						if (chess_piece_is_white(all_instances[j]->getName()))
						{
							if (data->cpu_is_white) castling = all_instances[j];
							else all_instances[j]->enabled = false;
						}
						else
						{
							if (!data->cpu_is_white) castling = all_instances[j];
							else all_instances[j]->enabled = false;
						}
						break;
					}						
				}

				Base::log( "piece at (%d,%d) was hit\n", hitPos.x, hitPos.y );
			}

			myvec2 engineMovePos = chessToGeometryInstancePos( data->move[1] );
			all_instances[i]->matrix[12] = engineMovePos.x;
			all_instances[i]->matrix[13] = engineMovePos.y;
			found = all_instances[i];
			if ( chessboard->previousMove() == PURE_MOVE_PAWN_PROMOTE || chessboard->previousMove() == PURE_MOVE_HIT_PAWN_PROMOTE )
			{
				EE_Geometry *queen_geometry = NULL;
				EE_GeometryInstance *pawn_queen = NULL;
				EE_GeometryInstance *queen_instance = NULL;

				pawn_queen = all_instances[i]->clone();
				for ( int j = 0; j < all_geometries.size(); j++ )
				{
					if ( 0 == strcmp( all_geometries[j]->getName(),"queen.X") )
					{
						queen_geometry = all_geometries[j];
						break;
					}					
				}
				for ( int j = 0; j < all_instances.size(); j++ )
				{
					if ( 0 == strncmp( all_instances[j]->getName(),"queen.X_instance", 16 ))
					{
						queen_instance = all_instances[j];
						break;
					}
				}
				pawn_queen->geometry = queen_geometry;
				pawn_queen->matrix = queen_instance->matrix;
				pawn_queen->matrix[12] = engineMovePos.x;
				pawn_queen->matrix[13] = engineMovePos.y;
				pawn_queen->pickable = all_instances[i]->pickable;
				pawn_queen->shadow_occluder = all_instances[i]->shadow_occluder;
				pawn_queen->shadow_receiver = all_instances[i]->shadow_receiver;
				pawn_queen->enabled = true;
				all_instances[i]->enabled = false;
				EE_Node *child = all_instances[i]->child;
				if ( child == NULL )
				{
					all_instances[i]->child = pawn_queen;
				}
				else
				{
					while ( child->sibling != NULL ) child = child->sibling;
					child->sibling = pawn_queen;
				}

				tinyxml2::XMLElement* nameElement = all_instances[i]->xmlelement->FirstChildElement( "Name" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* cxElement = all_instances[i]->xmlelement->FirstChildElement( "CoordX" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* cyElement = all_instances[i]->xmlelement->FirstChildElement( "CoordY" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* czElement = all_instances[i]->xmlelement->FirstChildElement( "CoordZ" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* rxElement = all_instances[i]->xmlelement->FirstChildElement( "RotateX" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* ryElement = all_instances[i]->xmlelement->FirstChildElement( "RotateY" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* rzElement = all_instances[i]->xmlelement->FirstChildElement( "RotateZ" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* sxElement = all_instances[i]->xmlelement->FirstChildElement( "ScaleX" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* syElement = all_instances[i]->xmlelement->FirstChildElement( "ScaleY" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* szElement = all_instances[i]->xmlelement->FirstChildElement( "ScaleZ" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* thetaElement = all_instances[i]->xmlelement->FirstChildElement( "Theta" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* enabledElement = all_instances[i]->xmlelement->FirstChildElement( "Enabled" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* parentuidElement = all_instances[i]->xmlelement->FirstChildElement( "ParentUID" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* uidElement = all_instances[i]->xmlelement->FirstChildElement( "UID" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* geometryuidElement = all_instances[i]->xmlelement->FirstChildElement( "GeometryUID" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* pickableElement = all_instances[i]->xmlelement->FirstChildElement( "Pickable" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* receiverElement = all_instances[i]->xmlelement->FirstChildElement( "SMReceiver" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* occluderElement = all_instances[i]->xmlelement->FirstChildElement( "SMOccluder" )->ShallowClone(NULL)->ToElement();

				nameElement->InsertEndChild( xmldocument->NewText( pawn_queen->getName() ) );
				cxElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "CoordX" )->GetText() ) );
				cyElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "CoordY" )->GetText() ) );
				czElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "CoordZ" )->GetText() ) );
				rxElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "RotateX" )->GetText() ) );
				ryElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "RotateY" )->GetText() ) );
				rzElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "RotateZ" )->GetText() ) );
				sxElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "ScaleX" )->GetText() ) );
				syElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "ScaleY" )->GetText() ) );
				szElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "ScaleZ" )->GetText() ) );
				thetaElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "Theta" )->GetText() ) );
				enabledElement->InsertEndChild( xmldocument->NewText( "True" ) );
				parentuidElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "ParentUID" )->GetText() ) );
				uidElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "UID" )->GetText() ) );
				char str[20];
				snprintf(str,20, "%d", queen_geometry->uid);
				geometryuidElement->InsertEndChild( xmldocument->NewText( str ) );
				pickableElement->InsertEndChild( xmldocument->NewText( all_instances[i]->xmlelement->FirstChildElement( "Pickable" )->GetText() ) );
				receiverElement->InsertEndChild( xmldocument->NewText( all_instances[i]->xmlelement->FirstChildElement( "SMReceiver" )->GetText() ) );
				occluderElement->InsertEndChild( xmldocument->NewText( all_instances[i]->xmlelement->FirstChildElement( "SMOccluder" )->GetText() ) );

				all_instances[i]->xmlelement->FirstChildElement( "Enabled" )->FirstChild()->ToText()->SetValue("False");

				tinyxml2::XMLElement *node = xmldocument->NewElement("SceneGeometryInstance");
				node->LinkEndChild(nameElement);
				node->LinkEndChild(cxElement);
				node->LinkEndChild(cyElement);
				node->LinkEndChild(czElement);
				node->LinkEndChild(rxElement);
				node->LinkEndChild(ryElement);
				node->LinkEndChild(rzElement);
				node->LinkEndChild(sxElement);
				node->LinkEndChild(syElement);
				node->LinkEndChild(szElement);
				node->LinkEndChild(thetaElement);
				node->LinkEndChild(enabledElement);
				node->LinkEndChild(parentuidElement);
				node->LinkEndChild(uidElement);
				node->LinkEndChild(geometryuidElement);
				node->LinkEndChild(pickableElement);
				node->LinkEndChild(receiverElement);
				node->LinkEndChild(occluderElement);
		
				all_instances[i]->xmlelement->Parent()->InsertEndChild(node);
				all_instances.push_back( pawn_queen );
				pawn_queen->xmlelement = node;
			}
			Base::log( "engineMove(%.2f,%.2f) done\n", engineMovePos.x, engineMovePos.y );

			break;
		}
	}

	opponent_in_checkmate = (chessengine->checkCheckMate( *data->chessboard, data->cpu_is_white ? BLACK : WHITE ) == CHECKMATE) ? true : false;
	chess_engine_thread = THREADING_INVALID_HANDLE;
	bool opponent_in_check = (chessengine->checkIfCheckUser( *data->chessboard, data->cpu_is_white ? WHITE : BLACK ) == CHECK) ? true : false;

	if ( !found )
	{
		Base::log( "Can't chessboard->move(%d,%d)->(%d,%d)\n", data->move[0].x,data->move[0].y,data->move[1].x,data->move[1].y );
	}
	else if (castling)
	{
		Vec2 castlingpos = geometryInstanceToChessPos( castling );
		bool picked_is_rook = (0 == strncmp(found->getName(), "rook_b1.X_instance", 18)) ? true : false;
		bool castling_left = (data->move[0].x - data->move[1].x < 0) ? (picked_is_rook ? false : true) : (picked_is_rook ? true : false);
		if (picked_is_rook)
		{
			/* Move the other piece, which is the king, to its rocade pos. 
			 * Either to the left or to the right */
			if (castling_left) castlingpos.x += 2;
			else castlingpos.x -= 2;
		}
		else
		{
			/* Move the other piece, which is the rook, to its rocade pos.
			 * Either to the right or to the left */
			if (castling_left) castlingpos.x -= 2;
			else castlingpos.x += 3;
		}
		myvec2 newCastlingpos = chessToGeometryInstancePos( castlingpos );
		castling->matrix[12] = newCastlingpos.x;
		castling->matrix[13] = newCastlingpos.y;

		Vec2 pickedpos = posFrom;
		if (picked_is_rook)
		{
			/* Move the picked piece, which is the rook, to its rocade pos.
			 * Either to the left or to the right */
			if (castling_left) pickedpos.x -= 2;
			else pickedpos.x += 3;
		}
		else
		{
			/* Move the picked piece, which is the king, to its rocade pos.
			 * Either to the right or to the left */
			if (castling_left) pickedpos.x += 2;
			else pickedpos.x -= 2;
		}
		myvec2 newPickedpos = chessToGeometryInstancePos(pickedpos );
		found->matrix[12] = newPickedpos.x;
		found->matrix[13] = newPickedpos.y;
	} 
	delete(data);

	if ( opponent_in_checkmate )
	{
		Base::log("Congratulations for black cpu. White is in Checkmate!\n");
		opponent_in_check = false;
	}

	Base::log("check_chess_engine_done numMoves:%d\n", numMoves );

	//Update local check and checkmate states after move.
	if (black_can_move())
	{
		if (opponent_in_check)
			black_check = true;		
		if (opponent_in_checkmate)
			black_checkmate = true;
	}
	else
	{
		if (opponent_in_check)
			white_check = true;
		if (opponent_in_checkmate)
			white_checkmate = true;
	}

	if (cpu_can_move() && !white_checkmate && !black_checkmate)
	{
		chess_engine_thread_data *thread_data = new chess_engine_thread_data();
		thread_data->chessboard = chessboard;
		thread_data->chessengine = chessengine;
		thread_data->cpu_is_white = !(numMoves & 1);
		thread_data->depth = chess_engine_depth;
		chess_engine_thread = create_thread(chess_engine_thread_func,thread_data);
		Base::log("thread:%d startAlphaBetaPruning for data:%p chessboard:%p\n", chess_engine_thread, thread_data, thread_data->chessboard);
		release_mutex(chessboard_mutex);
		return false;
	}

	//Enable possible player pieces after cpu move
	enable_chess_pieces(white_can_move() && player_is_white(), black_can_move() && player_is_black(), all_instances);
	
	release_mutex( chessboard_mutex );

	return true;
}

bool check_player_in_check()
{
	if (player_is_white() && white_check)
		return true;
	if (player_is_black() && black_check)
		return true;
	return false;
}

bool check_player_in_checkmate()
{
	if (player_is_white() && white_checkmate)
		return true;
	if (player_is_black() && black_checkmate)
		return true;
	return false;
}

bool check_opponent_in_checkmate()
{
	if(white_can_move() && black_checkmate)
		return true;
	if (black_can_move() && white_checkmate)
		return true;
	return false;
}

void init_chess_thread()
{
    if ( THREADING_INVALID_HANDLE != chess_engine_thread && !exit_is_signalled(chess_engine_thread) )
	{
		Base::log("Waiting for chess engine to finish");
	    join_thread(chess_engine_thread);
	    destroy_thread(chess_engine_thread);
		chess_engine_thread = THREADING_INVALID_HANDLE;
	}
}

void enable_chess_pieces( bool white_enabled, bool black_enabled, vector<EE_GeometryInstance*> &instances )
{
	for ( int j = 0; j < instances.size(); j++ )
	{
		if ( is_chess_piece( instances[j]->getName() ) )
		{
			if ( chess_piece_is_white( instances[j]->getName() ) )
			{
				instances[j]->pickable = white_enabled;
			}
			else
			{
				instances[j]->pickable = black_enabled;
			}
		}
	}
}

bool reinit_chess_state( vector<EE_GeometryInstance *> &all_instances )
{
	bool reinited;
	if (NULL != chessboard) 
	{
		grab_mutex( chessboard_mutex );
		enable_chess_pieces(player_is_white() && white_can_move(), player_is_black() && black_can_move(), all_instances);
		release_mutex( chessboard_mutex );
		reinited = true;
	}
	else
	{
		chessboard_mutex = create_mutex("chessboard_mutex");
		reinited = false;
		enable_chess_pieces(player_is_white() && white_can_move(), player_is_black() && black_can_move(), all_instances);
	}

	return reinited;
}

void release_chess_board()
{
	if (chessboard_mutex != nullptr)
	{
		grab_mutex(chessboard_mutex);
		Base::log("release_chess_board chessboard:%d chessengine:%d", chessboard, chessengine);
		chessboard = NULL;
		chessengine = NULL;
		release_mutex(chessboard_mutex);
	}
}

static bool user_move(EE_GeometryInstance *instance, int from_x, int from_y, int to_x, int to_y)
{
	bool failedMove = false;
	bool opponent_checkmate = false;
	bool player_in_check = false;
	EE_GeometryInstance *castling = NULL;
	//pointers not references to avoid destructor
	vector<EE_GeometryInstance *> *all_instances = get_all_instances();
	vector<EE_Geometry*> *all_geometries = get_all_geometries();
	tinyxml2::XMLDocument *xmldocument = (tinyxml2::XMLDocument*)get_xml_document();
	Vec2 from = Vec2(from_x, from_y);
	Vec2 to = Vec2(to_x, to_y);

	bool user_is_black = false;

	if (player_can_move())
	{		
		user_is_black = black_can_move() && player_is_black();
	}
	else if (player_external_can_move())
	{
		user_is_black = black_can_move() && player_external_is_black();
	}

	if ( from.x < 8 && from.y < 8 && to.x < 8 && to.y < 8 && from.x >= 0 && from.y >= 0 && to.x >= 0 && to.y >= 0 )
	{
		Vec2 hitPos = Vec2(-1,-1);
		Board *tmpBoard = chessboard->copyBoard();
		bool move_ok = tmpBoard->move(from, to, hitPos);
		player_in_check = chessengine->checkIfCheckUser( *tmpBoard, user_is_black ? WHITE : BLACK );

		if(!player_in_check && move_ok)
		{
			if (player_external_can_move())
			{
				myvec2 engineMovePos = chessToGeometryInstancePos( to );
				instance->matrix[12] = engineMovePos.x;
				instance->matrix[13] = engineMovePos.y;
				Base::log("Player external moving from %d,%d to %d,%d\n", from.x ,from.y, to.x, to.y);
			}
			chessboard->move(from, to, hitPos);

			opponent_checkmate = (chessengine->checkCheckMate( *chessboard, user_is_black ? WHITE : BLACK) == CHECKMATE) ? true : false;

			if (hitPos.x >= 0 && hitPos.y >= 0)
			{
				int i = 0;
				for (  ; i < all_instances->size(); i++ )
				{
					Vec2 checkPos = geometryInstanceToChessPos( (*all_instances)[i] );

					if (to.x == checkPos.x && to.y == checkPos.y &&
							(*all_instances)[i] != instance &&
							(*all_instances)[i]->enabled)
					{
						//If chess-piece is white and player is black => piece eaten
						//If chess-piece is black and player is white => piece eaten
						//For same color hits we must check for castling
						
						if (chess_piece_is_white((*all_instances)[i]->getName()))
						{

							if (user_is_black) (*all_instances)[i]->enabled = false;
							else  castling = (*all_instances)[i];
						}
						else
						{
							if (!user_is_black) (*all_instances)[i]->enabled = false;
							else castling = (*all_instances)[i];
						}
						break;
					}
				}
				Base::log( "piece at (%d,%d) was hit\n", hitPos.x, hitPos.y );
			}

			if ( chessboard->previousMove() == PURE_MOVE_PAWN_PROMOTE || chessboard->previousMove() == PURE_MOVE_HIT_PAWN_PROMOTE )
			{
				EE_Geometry *queen_geometry = NULL;
				EE_GeometryInstance *pawn_queen = NULL;
				EE_GeometryInstance *queen_instance = NULL;

				pawn_queen = instance->clone();
				for ( int j = 0; j < all_geometries->size(); j++ )
				{
					if ( 0 == strcmp( (*all_geometries)[j]->getName(),"queen.X") )
					{
						queen_geometry = (*all_geometries)[j];
						break;
					}
				}
				for ( int j = 0; j < all_instances->size(); j++ )
				{
					if ( 0 == strncmp( (*all_instances)[j]->getName(),"queen.X_instance",16 ))
					{
						queen_instance = (*all_instances)[j];
						break;
					}
				}
				pawn_queen->geometry = queen_geometry;
				pawn_queen->matrix = queen_instance->matrix;
				pawn_queen->matrix[12] = instance->matrix[12];
				pawn_queen->matrix[13] = instance->matrix[13];
				pawn_queen->pickable = instance->pickable;
				pawn_queen->shadow_occluder = instance->shadow_occluder;
				pawn_queen->shadow_receiver = instance->shadow_receiver;
				pawn_queen->enabled = true;
				instance->enabled = false;
				EE_Node *child = instance->child;
				if ( child == NULL )
				{
					instance->child = pawn_queen;
				}
				else
				{
					while ( child->sibling != NULL ) child = child->sibling;
					child->sibling = pawn_queen;
				}

				tinyxml2::XMLElement* nameElement = instance->xmlelement->FirstChildElement( "Name" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* cxElement = instance->xmlelement->FirstChildElement( "CoordX" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* cyElement = instance->xmlelement->FirstChildElement( "CoordY" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* czElement = instance->xmlelement->FirstChildElement( "CoordZ" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* rxElement = instance->xmlelement->FirstChildElement( "RotateX" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* ryElement = instance->xmlelement->FirstChildElement( "RotateY" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* rzElement = instance->xmlelement->FirstChildElement( "RotateZ" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* sxElement = instance->xmlelement->FirstChildElement( "ScaleX" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* syElement = instance->xmlelement->FirstChildElement( "ScaleY" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* szElement = instance->xmlelement->FirstChildElement( "ScaleZ" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* thetaElement = instance->xmlelement->FirstChildElement( "Theta" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* enabledElement = instance->xmlelement->FirstChildElement( "Enabled" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* parentuidElement = instance->xmlelement->FirstChildElement( "ParentUID" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* uidElement = instance->xmlelement->FirstChildElement( "UID" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* geometryuidElement = instance->xmlelement->FirstChildElement( "GeometryUID" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* pickableElement = instance->xmlelement->FirstChildElement( "Pickable" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* receiverElement = instance->xmlelement->FirstChildElement( "SMReceiver" )->ShallowClone(NULL)->ToElement();
				tinyxml2::XMLElement* occluderElement = instance->xmlelement->FirstChildElement( "SMOccluder" )->ShallowClone(NULL)->ToElement();

				nameElement->InsertEndChild( xmldocument->NewText( pawn_queen->getName() ) );
				cxElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "CoordX" )->GetText() ) );
				cyElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "CoordY" )->GetText() ) );
				czElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "CoordZ" )->GetText() ) );
				rxElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "RotateX" )->GetText() ) );
				ryElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "RotateY" )->GetText() ) );
				rzElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "RotateZ" )->GetText() ) );
				sxElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "ScaleX" )->GetText() ) );
				syElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "ScaleY" )->GetText() ) );
				szElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "ScaleZ" )->GetText() ) );
				thetaElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "Theta" )->GetText() ) );
				enabledElement->InsertEndChild( xmldocument->NewText( "True" ) );
				parentuidElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "ParentUID" )->GetText() ) );
				uidElement->InsertEndChild( xmldocument->NewText( queen_instance->xmlelement->FirstChildElement( "UID" )->GetText() ) );
				char str[20];
				snprintf(str,20, "%d", queen_geometry->uid);
				geometryuidElement->InsertEndChild( xmldocument->NewText( str ) );
				pickableElement->InsertEndChild( xmldocument->NewText( instance->xmlelement->FirstChildElement( "Pickable" )->GetText() ) );
				receiverElement->InsertEndChild( xmldocument->NewText( instance->xmlelement->FirstChildElement( "SMReceiver" )->GetText() ) );
				occluderElement->InsertEndChild( xmldocument->NewText( instance->xmlelement->FirstChildElement( "SMOccluder" )->GetText() ) );

				instance->xmlelement->FirstChildElement( "Enabled" )->FirstChild()->ToText()->SetValue("False");

				tinyxml2::XMLElement *node = xmldocument->NewElement("SceneGeometryInstance");
				node->LinkEndChild(nameElement);
				node->LinkEndChild(cxElement);
				node->LinkEndChild(cyElement);
				node->LinkEndChild(czElement);
				node->LinkEndChild(rxElement);
				node->LinkEndChild(ryElement);
				node->LinkEndChild(rzElement);
				node->LinkEndChild(sxElement);
				node->LinkEndChild(syElement);
				node->LinkEndChild(szElement);
				node->LinkEndChild(thetaElement);
				node->LinkEndChild(enabledElement);
				node->LinkEndChild(parentuidElement);
				node->LinkEndChild(uidElement);
				node->LinkEndChild(geometryuidElement);
				node->LinkEndChild(pickableElement);
				node->LinkEndChild(receiverElement);
				node->LinkEndChild(occluderElement);

				instance->xmlelement->Parent()->InsertEndChild(node);

				chessboard->promotePawn(to, QUEEN);
				all_instances->push_back( pawn_queen );
				pawn_queen->xmlelement = node;
			}

			numMoves += 1;

			if ( !opponent_checkmate && cpu_can_move() )
			{
				chess_engine_thread_data *thread_data = new chess_engine_thread_data();
				thread_data->chessboard = chessboard;
				thread_data->chessengine = chessengine;
				thread_data->cpu_is_white = cpu_is_white();
				thread_data->depth = chess_engine_depth;
				chess_engine_thread = create_thread(chess_engine_thread_func,thread_data);
				Base::log("thread:%d startAlphaBetaPruning for data:%p chessboard:%p\n", chess_engine_thread, thread_data, thread_data->chessboard);
			}

			//Disable inactive pieces after move
			enable_chess_pieces(white_can_move() && player_is_white(), black_can_move() && player_is_black(), *all_instances);
		}
		else
		{
			failedMove = true;
		}
	}
	else
	{
		failedMove = true;
	}

	if (!failedMove && castling)
	{
		Vec2 castlingpos = geometryInstanceToChessPos( castling );
		bool picked_is_rook = (0 == strncmp(instance->getName(), "rook_b1.X_instance", 18)) ? true : false;
		bool castling_left = (from.x - to.x < 0) ? (picked_is_rook ? false : true) : (picked_is_rook ? true : false);
		if (picked_is_rook)
		{
			/* Move the other piece, which is the king, to its rocade pos.
			 * Either to the left or to the right*/
			if (castling_left) castlingpos.x += 2;
			else castlingpos.x -= 2;
		}
		else
		{
			/* Move the other piece, which is the rook, to its rocade pos.
			 * Either to the right or to the left */
			if (castling_left) castlingpos.x -= 2;
			else castlingpos.x += 3;
		}
		myvec2 newCastlingpos = chessToGeometryInstancePos( castlingpos );
		castling->matrix[12] = newCastlingpos.x;
		castling->matrix[13] = newCastlingpos.y;

		Vec2 pickedpos = from;
		if (picked_is_rook)
		{
			/* Move the picked piece, which is the rook, to its rocade pos.
				* Either to the left or to the right */
			if (castling_left) pickedpos.x -= 2;
			else pickedpos.x += 3;
		}
		else
		{
			/* Move the picked piece, which is the king, to its rocade pos.
				* Either to the right or to the left */
			if (castling_left) pickedpos.x += 2;
			else pickedpos.x -= 2;
		}
		myvec2 newPickedpos = chessToGeometryInstancePos( pickedpos );
		instance->matrix[12] = newPickedpos.x;
		instance->matrix[13] = newPickedpos.y;
	}

	//Update local check flags
	if (player_in_check)
	{
		//Currently in check
		Base::log("Player can't move to %d,%d still in check\n", to.x, to.y );
		if (!user_is_black)
			white_check = true;
		if (user_is_black)
			black_check = true;
	}
	else
	{
		//Possibly gone out of check
		if (!user_is_black)
			white_check = false;
		if (user_is_black)
			black_check = false;
	}

	// Update local checkmate flags
	if (opponent_checkmate)
	{
		Base::log("Congratulations for player. Opponent is in Checkmate!\n");
		int opponent_cpu = 0;
		int opponent_external = 0;
		int opponent_local = 0;
		int player_is_white = 0;
		if (!user_is_black) {
			black_checkmate = true;
			player_is_white = 1;
			opponent_cpu = (white_player == TYPE_CPU);
			opponent_external = (white_player == TYPE_PLAYER_EXTERNAL);
			opponent_local = (white_player = TYPE_PLAYER);
		}
		if (user_is_black) {
			white_checkmate = true;
			player_is_white = 0;
			opponent_cpu = (black_player == TYPE_CPU);
			opponent_external = (black_player == TYPE_PLAYER_EXTERNAL);
			opponent_local = (black_player == TYPE_PLAYER);
		}
#if PLATFORM_ARM_ANDROID
		javaPerformWon(opponent_cpu, opponent_external, opponent_local, player_is_white);
#endif
		enable_chess_pieces(false,false,*all_instances);
	}

	if ( failedMove )
	{
		Base::log( "Failed move from (%d,%d) to (%d,%d)\n", from.y, from.x, to.y, to.x );
	}
	else
	{
		if (player_external_can_move())
		{
#if PLATFORM_ARM_ANDROID
			javaPerformMove(from.x, from.y, to.x, to.y);
#endif
		}

		if (player_can_move())
		{
			bool next_move_black = black_can_move();
			get_scene()->camera->setOrbitTo(!next_move_black);
			get_scene()->camera->orbitToHome();	
		}	
	}

	return failedMove;
}


EE_GeometryInstance * get_instance_from_chess_pos(vector<EE_GeometryInstance*> *all_instances, int chess_pos_x, int chess_pos_y) {
	int i;
	for ( i = 0; i < all_instances->size(); i++) {
		Vec2 pos = geometryInstanceToChessPos((*all_instances)[i]);
		if (chess_pos_x == pos.x && chess_pos_y == pos.y && (*all_instances)[i]->enabled &&
			is_chess_piece((*all_instances)[i]->getName())) {
				const char *name = (*all_instances)[i]->getName();
				// Only pick instance that can move
				if ((white_can_move() && chess_piece_is_white(name)) ||
					(black_can_move() && !chess_piece_is_white(name))) {
						return (*all_instances)[i];
					}
			}
	}
	return nullptr;
}

bool external_user_moved(int from_x, int from_y, int to_x, int to_y)
{
	vector<EE_GeometryInstance *> *all_instances = get_all_instances();
	EE_GeometryInstance *instance = get_instance_from_chess_pos(all_instances, from_x, from_y);
	if (nullptr == instance) {
		Base::log("Failed to find instance from user_move(%d,%d) %d %d\n", from_x, from_y);
		return true;
	}
	
	return user_move(instance, from_x, from_y, to_x, to_y);
}

bool user_moved_chess_piece( EE_GeometryInstance *instance )
{
	Vec2 posto = geometryInstanceToChessPos( instance );

	Base::log( "User move from (%d,%d) to(%d,%d) \n", posFrom.x, posFrom.y, posto.x, posto.y );

	if ( check_player_in_checkmate() || check_opponent_in_checkmate() ) return false;

	return user_move(instance, posFrom.x, posFrom.y, posto.x, posto.y);
}

void reset_chess_board()
{
	Base::log("Resetting chess game");
	init_chess_thread();

	grab_mutex( chessboard_mutex );

	delete( chessboard );
	delete( chessengine );
	chessboard = NULL;
	chessengine = NULL;
	numMoves = 0;
	white_checkmate = false;
	black_checkmate = false;
	white_check = false;
	black_check = false;
	posFrom = Vec2(-1,-1);

	release_mutex( chessboard_mutex );
}

void pick_chess_piece( EE_GeometryInstance *instance )
{
	posFrom = geometryInstanceToChessPos( instance );
}

void create_empty_chess_board()
{
	if (NULL == chessboard)
	{
		Base::log("Creating blank chessboard\n");
		chessboard = new Board();
		chessengine = new Engine();
		chessboard->updateBools();
		for (int x = 0; x < 8; x++)
			for (int y = 0; y < 8; y++)
				chessboard->removePiece( Vec2(x,y) );
	}
}

void add_chess_board_piece( Piece *piece )
{
	Base::log("color: %d value: %d type: %d rank: %d firstmove: %d at (%d,%d)\n", piece->color, piece->value, piece->type, piece->rank, piece->firstMove, piece->pos.x, piece->pos.y);
	chessboard->insertPiece( piece->pos, *piece );
}

void export_chess_state( tinyxml2::XMLDocument *xmldocument, tinyxml2::XMLElement *trondObjectElement, vector<EE_GeometryInstance*> &all_instances )
{
	if ( NULL != chessboard )
	{
	    map<Vec2, Piece>::iterator it  = chessboard->pieces.begin();
        while(it != chessboard->pieces.end())
		{

			tinyxml2::XMLElement* pieceObjectElement = xmldocument->NewElement( "Piece" );
			trondObjectElement->InsertEndChild(pieceObjectElement);
			
			tinyxml2::XMLElement* nameElement = xmldocument->NewElement("Name"); 
			tinyxml2::XMLElement* posxElement = xmldocument->NewElement("posx"); 
			tinyxml2::XMLElement* poszElement = xmldocument->NewElement("posz");
			tinyxml2::XMLElement* colorElement = xmldocument->NewElement("color");
			tinyxml2::XMLElement* valueElement = xmldocument->NewElement("value");
			tinyxml2::XMLElement* typeElement = xmldocument->NewElement("type");
			tinyxml2::XMLElement* rankElement = xmldocument->NewElement("rank");
			tinyxml2::XMLElement* firstMoveElement = xmldocument->NewElement("firstMove");
	
			EE_GeometryInstance *g = NULL;
	
			for ( int i = 0; i < all_instances.size(); i++ )
			{
				g = all_instances[i];
				if ( is_chess_piece( g->getName() ) && g->enabled )
				{
					Vec2 r = geometryInstanceToChessPos( g );
					if ( r.x == it->first.x && r.y == it->first.y)
					{
						break;
					}
				}
			}			

			char str[10];
			nameElement->InsertEndChild( xmldocument->NewText( g->getName() ) );
			snprintf( str, 10, "%i", it->second.pos.x);
			posxElement->InsertEndChild( xmldocument->NewText( str ) );
			snprintf( str, 10, "%i", it->second.pos.y);
			poszElement->InsertEndChild( xmldocument->NewText( str ) );
			snprintf( str, 10, "%i", it->second.color);
			colorElement->InsertEndChild( xmldocument->NewText( str ) );
			snprintf(str, 10, "%i", it->second.value);
			valueElement->InsertEndChild( xmldocument->NewText( str ) );
			snprintf(str, 10, "%i", it->second.type);
			typeElement->InsertEndChild( xmldocument->NewText( str ) );
			snprintf(str, 10, "%i", it->second.rank);
			rankElement->InsertEndChild( xmldocument->NewText( str ) );
			snprintf(str, 10, "%i", it->second.firstMove);
			firstMoveElement->InsertEndChild( xmldocument->NewText( str ) );

			pieceObjectElement->InsertEndChild( nameElement );
			pieceObjectElement->InsertEndChild( posxElement );
			pieceObjectElement->InsertEndChild( poszElement );
			pieceObjectElement->InsertEndChild( colorElement );
			pieceObjectElement->InsertEndChild( valueElement );
			pieceObjectElement->InsertEndChild( typeElement );
			pieceObjectElement->InsertEndChild( rankElement );
			pieceObjectElement->InsertEndChild( firstMoveElement );

			it++;
		}	
	}

}

EE_Scene *readXMLChessScene( const char *filename, tinyxml2::XMLDocument *doc, EE_Scene *currentScene )
{
	char *xml;
	FILE *file = fopen( filename, "rb" );
	if ( NULL == file )
	{
		Base::log( "Could not open file %s\n", filename );
		return NULL;
	}

	fseek(file, 0L, SEEK_END);
	size_t sz = ftell(file);
	rewind(file);
	xml = (char*)malloc(sz+1);
	size_t result = fread( xml, 1, sz, file );
	if ( result != sz )
	{
		Base::log( "Could not read file %s expected:%d read:%d\n", filename, result, sz );
		fclose(file);
		free(xml);
		return NULL;
	}
	xml[sz] = 0;

	fclose(file);
	doc->Parse( xml );

	if ( doc->Error() ) 
	{
		Base::log(" There was an error parsing the document '%d'\n", doc->ErrorID() );
		Base::log( "Log: %s", doc->GetErrorStr1() );
		Base::log( "Log2: %s", doc->GetErrorStr2() );
		return NULL;
	}

	tinyxml2::XMLElement *sceneElement = doc->FirstChildElement("scene");
	if ( !sceneElement ) return NULL;
	
	EE_Scene *chess_scene;
	
	if (currentScene)
	{
		chess_scene = currentScene;
	}
	else
	{
		chess_scene = new EE_Scene( "MainScene", 0 );		
	}

	tinyxml2::XMLElement* sceneObjectElement = sceneElement->FirstChildElement( "SceneObject" );
	while ( sceneObjectElement )	
	{
		tinyxml2::XMLElement* urlElement = sceneObjectElement->FirstChildElement( "URL" );
		tinyxml2::XMLElement* nameElement = sceneObjectElement->FirstChildElement( "Name" );
		tinyxml2::XMLElement* uidElement = sceneObjectElement->FirstChildElement( "UID" );
		tinyxml2::XMLElement* parentElement = sceneObjectElement->FirstChildElement( "ParentUID" );
		tinyxml2::XMLElement* typeElement = sceneObjectElement->FirstChildElement( "Type" );
		const char* name = nameElement->GetText();
		const char* url = urlElement->GetText();
		const char* type = typeElement->GetText();
		int uid, parentuid;
		uidElement->QueryIntText(&uid);
		parentElement->QueryIntText(&parentuid);
		Base::log( "Name of SceneObject: %s url:%s uid:%d\n", name, url, uid );
		if ( 0 == strncmp( url, "ee:\\MainCamera", 15 ) )
		{
			EE_Camera *camera;

			if (currentScene)
			{
				camera = currentScene->camera;
			}
			else
			{
				camera = new EE_Camera( name, uid );
				chess_scene->camera = camera;
			}
			tinyxml2::XMLElement* cxElement = sceneObjectElement->FirstChildElement( "CoordX" );
			tinyxml2::XMLElement* cyElement = sceneObjectElement->FirstChildElement( "CoordY" );
			tinyxml2::XMLElement* czElement = sceneObjectElement->FirstChildElement( "CoordZ" );
			tinyxml2::XMLElement* rxElement = sceneObjectElement->FirstChildElement( "RotateX" );
			tinyxml2::XMLElement* ryElement = sceneObjectElement->FirstChildElement( "RotateY" );
			tinyxml2::XMLElement* rzElement = sceneObjectElement->FirstChildElement( "RotateZ" );

			tinyxml2::XMLElement* thetaElement = sceneObjectElement->FirstChildElement( "Theta" );
			float cx,cy,cz,rx,ry,rz;
			float theta;

			const char* name = nameElement->GetText();
			cxElement->QueryFloatText(&cx);
			cyElement->QueryFloatText(&cy);
			czElement->QueryFloatText(&cz);
			rxElement->QueryFloatText(&rx);
			ryElement->QueryFloatText(&ry);
			rzElement->QueryFloatText(&rz);
			thetaElement->QueryFloatText(&theta);

			/* DX rotation rx,ry,rz = GL rotation rx,rz,ry */
			mymat4 rotation = mymat4::rotation( theta*(180.0f/M_PI), myvec3( rx, rz, ry ) );
			mymat4 translation = mymat4::translation( cx, cy, cz );

			camera->matrix = translation * rotation ;
		} else if ( 0 == strncmp( url, "ee:\\", 4 ) ) 
		{
			if ( 0 == strncmp( type, "EE_TRANSFORM_NODE", 17 ) )
			{
				tinyxml2::XMLElement* cxElement = sceneObjectElement->FirstChildElement( "CoordX" );
				tinyxml2::XMLElement* cyElement = sceneObjectElement->FirstChildElement( "CoordY" );
				tinyxml2::XMLElement* czElement = sceneObjectElement->FirstChildElement( "CoordZ" );
				tinyxml2::XMLElement* rxElement = sceneObjectElement->FirstChildElement( "RotateX" );
				tinyxml2::XMLElement* ryElement = sceneObjectElement->FirstChildElement( "RotateY" );
				tinyxml2::XMLElement* rzElement = sceneObjectElement->FirstChildElement( "RotateZ" );
				tinyxml2::XMLElement* sxElement = sceneObjectElement->FirstChildElement( "ScaleX" );
				tinyxml2::XMLElement* syElement = sceneObjectElement->FirstChildElement( "ScaleY" );
				tinyxml2::XMLElement* szElement = sceneObjectElement->FirstChildElement( "ScaleZ" );
				tinyxml2::XMLElement* thetaElement = sceneObjectElement->FirstChildElement( "Theta" );
				tinyxml2::XMLElement* parentuidElement = sceneObjectElement->FirstChildElement( "ParentUID" );
				float cx,cy,cz,rx,ry,rz,sx,sy,sz;
				float theta;
				int parentuid;
				int uid;

				const char* name = nameElement->GetText();
				cxElement->QueryFloatText(&cx);
				cyElement->QueryFloatText(&cy);
				czElement->QueryFloatText(&cz);
				rxElement->QueryFloatText(&rx);
				ryElement->QueryFloatText(&ry);
				rzElement->QueryFloatText(&rz);
				sxElement->QueryFloatText(&sx);
				syElement->QueryFloatText(&sy);
				szElement->QueryFloatText(&sz);
				thetaElement->QueryFloatText(&theta);
				parentuidElement->QueryIntText( &parentuid );
				uidElement->QueryIntText( &uid );

				/* DX rotation rx,ry,rz = GL rotation rx,rz,ry */
				mymat4 rotation = (theta == 0) ? mymat4::identity() : mymat4::rotation( theta*(180.0f/M_PI), myvec3( rx, rz, ry ) );
				mymat4 translation = mymat4::translation( cx, cy, cz );
				mymat4 scale = mymat4::scale( sx, sy, sz );

				EE_Transform *transform = nullptr;
				//Check whether transform exists  
				for (int i = 0; currentScene && i < chess_scene->transforms.size(); i++)
				{
					if (chess_scene->transforms[i]->uid == uid)
					{	
						transform = chess_scene->transforms[i];
						break;
					} 
				}
				//Or add a new transform
				if (transform == nullptr)
				{
					transform = new EE_Transform( name, uid, parentuid );
					chess_scene->transforms.push_back( transform );
				}
				transform->matrix = translation * rotation * scale;
			}
		}
		else
		{
			EE_Geometry *geometry = nullptr;

			//Check whether geometry exists, as then we do nothing  
			for (int i = 0; currentScene && i < chess_scene->geometries.size(); i++)
			{
				if (chess_scene->geometries[i]->uid == uid)
				{	
					geometry = chess_scene->geometries[i];
					break;
				} 
			}
			//Or add a new geometry
			if (geometry == nullptr)
			{
				char *os_url = new_os_url( url );
				geometry = new EE_Geometry( name, os_url, uid );
				chess_scene->geometries.push_back( geometry );
				free( os_url );
			}
		}
		sceneObjectElement = sceneObjectElement->NextSiblingElement( "SceneObject" );
	}
	tinyxml2::XMLElement* sceneGeometryInstanceElement = sceneElement->FirstChildElement( "SceneGeometryInstance" );
	while ( sceneGeometryInstanceElement )	
	{
		tinyxml2::XMLElement* nameElement = sceneGeometryInstanceElement->FirstChildElement( "Name" );
		tinyxml2::XMLElement* cxElement = sceneGeometryInstanceElement->FirstChildElement( "CoordX" );
		tinyxml2::XMLElement* cyElement = sceneGeometryInstanceElement->FirstChildElement( "CoordY" );
		tinyxml2::XMLElement* czElement = sceneGeometryInstanceElement->FirstChildElement( "CoordZ" );
		tinyxml2::XMLElement* rxElement = sceneGeometryInstanceElement->FirstChildElement( "RotateX" );
		tinyxml2::XMLElement* ryElement = sceneGeometryInstanceElement->FirstChildElement( "RotateY" );
		tinyxml2::XMLElement* rzElement = sceneGeometryInstanceElement->FirstChildElement( "RotateZ" );
		tinyxml2::XMLElement* sxElement = sceneGeometryInstanceElement->FirstChildElement( "ScaleX" );
		tinyxml2::XMLElement* syElement = sceneGeometryInstanceElement->FirstChildElement( "ScaleY" );
		tinyxml2::XMLElement* szElement = sceneGeometryInstanceElement->FirstChildElement( "ScaleZ" );
		tinyxml2::XMLElement* thetaElement = sceneGeometryInstanceElement->FirstChildElement( "Theta" );
		tinyxml2::XMLElement* enabledElement = sceneGeometryInstanceElement->FirstChildElement( "Enabled" );
		tinyxml2::XMLElement* parentuidElement = sceneGeometryInstanceElement->FirstChildElement( "ParentUID" );
		tinyxml2::XMLElement* uidElement = sceneGeometryInstanceElement->FirstChildElement( "UID" );
		tinyxml2::XMLElement* geometryuidElement = sceneGeometryInstanceElement->FirstChildElement( "GeometryUID" );
		tinyxml2::XMLElement* pickableElement = sceneGeometryInstanceElement->FirstChildElement( "Pickable" );
		tinyxml2::XMLElement* receiverElement = sceneGeometryInstanceElement->FirstChildElement( "SMReceiver" );
		tinyxml2::XMLElement* occluderElement = sceneGeometryInstanceElement->FirstChildElement( "SMOccluder" );
		float cx,cy,cz,rx,ry,rz,sx,sy,sz;
		bool enabled;
		bool pickable;
		int uid, parentuid, geometryuid;
		float theta;
		bool shadow_receiver;
		bool shadow_occluder;

		const char* name = nameElement->GetText();
		cxElement->QueryFloatText(&cx);
		cyElement->QueryFloatText(&cy);
		czElement->QueryFloatText(&cz);
		rxElement->QueryFloatText(&rx);
		ryElement->QueryFloatText(&ry);
		rzElement->QueryFloatText(&rz);
		sxElement->QueryFloatText(&sx);
		syElement->QueryFloatText(&sy);
		szElement->QueryFloatText(&sz);
		thetaElement->QueryFloatText(&theta);
		uidElement->QueryIntText( &uid );
		parentuidElement->QueryIntText( &parentuid );
		geometryuidElement->QueryIntText( &geometryuid );
		if ( 0 == strncmp(enabledElement->GetText(), "True",4) ) enabled = true;
		else enabled = false;
		if ( 0 == strncmp(pickableElement->GetText(), "True",4) ) pickable = true;
		else pickable = false;
		if ( 0 == strncmp(receiverElement->GetText(), "True",4) ) shadow_receiver = true;
		else shadow_receiver = false;
		if ( 0 == strncmp(occluderElement->GetText(), "True",4) ) shadow_occluder = true;
		else shadow_occluder = false;

		Base::log( "Name of SceneObject: %s geometryuid: %d parentuid:%d uid:%d enabled: %d pickable: %d occluder: %d receiver: %d\n", name, geometryuid, parentuid, uid, enabled, pickable, shadow_occluder, shadow_receiver );

		EE_GeometryInstance *geometryInstance = nullptr;
		
		//Check whether geometry exists, as then we do nothing  
		for (int i = 0; currentScene && i < chess_scene->geometryinstances.size(); i++)
		{
			if (chess_scene->geometryinstances[i]->uid == uid)
			{	
				geometryInstance = chess_scene->geometryinstances[i];
				break;
			} 
		}
		//Or add a new geometry
		if (geometryInstance == nullptr)
		{
			geometryInstance = new EE_GeometryInstance( name, geometryuid, parentuid, uid);
			chess_scene->geometryinstances.push_back( geometryInstance );
		}
		
		geometryInstance->shadow_occluder = shadow_occluder;
		geometryInstance->shadow_receiver = shadow_receiver;

		/* DX rotation rx,ry,rz = GL rotation rx,rz,ry */
		mymat4 rotation = (theta == 0) ? mymat4::identity() : mymat4::rotation( theta*(180.f/M_PI), myvec3( rx, rz, ry ) );
		mymat4 translation = mymat4::translation( cx, cy, cz );
		mymat4 scale = mymat4::scale( sx, sy, sz );
		geometryInstance->matrix = rotation * translation * scale;
		geometryInstance->enabled = enabled;
		geometryInstance->pickable = pickable;
		geometryInstance->xmlelement = sceneGeometryInstanceElement;

		sceneGeometryInstanceElement = sceneGeometryInstanceElement->NextSiblingElement( "SceneGeometryInstance" );
	}

	Base::log("Reading TrondEngine");
	chess_scene->getSnapToGrid() = myvec3( 50, 50, -1 );

	tinyxml2::XMLElement* trondObjectElement = doc->FirstChildElement( "TrondEngine" );
	while ( trondObjectElement )	
	{
		tinyxml2::XMLElement* numMovesElement = trondObjectElement->FirstChildElement( "NumMoves" );
		numMovesElement->QueryIntText( &numMoves );
		Base::log("Reading numMoves: %d\n", numMoves );

		tinyxml2::XMLElement* pieceObjectElement = trondObjectElement->FirstChildElement( "Piece" );
		while ( pieceObjectElement )
		{
			tinyxml2::XMLElement* nameElement = pieceObjectElement->FirstChildElement( "Name" );
			tinyxml2::XMLElement* posxElement = pieceObjectElement->FirstChildElement( "posx" );
			tinyxml2::XMLElement* poszElement = pieceObjectElement->FirstChildElement( "posz" );
			tinyxml2::XMLElement* colorElement = pieceObjectElement->FirstChildElement( "color" );
			tinyxml2::XMLElement* valueElement = pieceObjectElement->FirstChildElement( "value" );
			tinyxml2::XMLElement* typeElement = pieceObjectElement->FirstChildElement( "type" );
			tinyxml2::XMLElement* rankElement = pieceObjectElement->FirstChildElement( "rank" );
			tinyxml2::XMLElement* firstMoveElement = pieceObjectElement->FirstChildElement( "firstMove" );

			const char* name = nameElement->GetText();
			int posx;
			int posz;
			int color;
			int value;
			int type;
			int rank;
			int firstMove;

			posxElement->QueryIntText( &posx );
			poszElement->QueryIntText( &posz );
			colorElement->QueryIntText( &color );
			valueElement->QueryIntText( &value );
			typeElement->QueryIntText( &type );
			rankElement->QueryIntText( &rank );
			firstMoveElement->QueryIntText( &firstMove );

			for (int i = 0; i < chess_scene->geometryinstances.size(); i++)
			{
				if( !strcmp(chess_scene->geometryinstances[i]->getName(), name) )
				{
					create_empty_chess_board();
					Vec2 pos = Vec2(posx,posz);
					Piece *piece = new Piece();
					piece->type = type;
					piece->value = value;
					piece->color = color;
					piece->pos = pos;
					piece->rank = rank;
					piece->firstMove = firstMove;
					add_chess_board_piece( piece );
					//Can't use helpers as get_scene() is not defined yet.
					//So we calculate directly the chesspiece's node position.
					myvec2 wpos = myvec2( (7-pos.y)*chess_scene->getSnapToGrid().x, (pos.x-4)*chess_scene->getSnapToGrid().y );

					chess_scene->geometryinstances[i]->matrix[12] = wpos.x;
					chess_scene->geometryinstances[i]->matrix[13] = wpos.y;
					break;
				}
			}
			pieceObjectElement = pieceObjectElement->NextSiblingElement( "Piece" );
		}
		trondObjectElement = trondObjectElement->NextSiblingElement( "TrondEngine" );		
	}

	if (!currentScene)
	{
		chess_scene->link();
		chess_scene->load();
	}

	free(xml);

	return chess_scene;
}

void saveXMLChessScene( const char *filename, EE_Scene *scene, tinyxml2::XMLDocument *xmldocument )
{
	if ( NULL == xmldocument ) return;
	
	Base::log("Saving game to: %s\n", filename );

	for (int i = 0; i < scene->geometryinstances.size(); i++ )
	{
		EE_GeometryInstance *g = scene->geometryinstances[i];
		tinyxml2::XMLElement * e = g->xmlelement->FirstChildElement( "Enabled" );
		if ( g->enabled && e )
		{
			e->FirstChild()->ToText()->SetValue("True");
		}
		else if ( e ) 
		{
			e->FirstChild()->ToText()->SetValue("False");
		}
	}

	Base::log("Saving chess state");

	tinyxml2::XMLElement* trondObjectElement = xmldocument->FirstChildElement( "TrondEngine" );
	if ( trondObjectElement )	
	{
		trondObjectElement->DeleteChildren();
		tinyxml2::XMLElement *numMovesElement = xmldocument->NewElement("NumMoves");
		trondObjectElement->InsertEndChild( numMovesElement);
		char str[4];
		snprintf(str,4, "%d", numMoves);
		Base::log("Saving num moves: %d\n", numMoves);
		numMovesElement->InsertEndChild(xmldocument->NewText( str ) );
		export_chess_state( xmldocument, trondObjectElement, scene->geometryinstances );
	}

	if( tinyxml2::XML_NO_ERROR != xmldocument->SaveFile( filename  ))
	{
		Base::log("Failed to save game to %s\n", filename );
	}
	else
	{
		Base::log("Successfully saved game to %s\n", filename );
	}
}
