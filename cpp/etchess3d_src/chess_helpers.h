#ifndef _CHESS_HELPERS_H_
#define _CHESS_HELPERS_H_

#include "math_helpers.h"
#include "simple_vector.h"

class Vec2;
class Piece;
class EE_GeometryInstance;
class EE_Geometry;
class XMLDocument;
class EE_Scene;

bool white_can_move();
bool player_can_move();
void player_external_move(int from_x, int from_y, int to_x, int to_y);
void set_white_is_cpu();
void set_black_is_cpu();
void set_white_is_player();
void set_black_is_player();
void set_white_is_external_player();
void set_black_is_external_player();
void set_engine_search_depth(int depth);
bool is_chess_piece( const char *name );
bool chess_piece_is_white( const char *name );
Vec2 geometryInstanceToChessPos( EE_GeometryInstance *p );
myvec2 chessToGeometryInstancePos( Vec2 chessPos );
bool check_chess_engine_done( vector<EE_GeometryInstance *> &all_instances );
bool check_player_in_check();
bool check_player_in_checkmate();
bool check_opponent_in_checkmate();
void enable_chess_pieces( bool white_enabled, bool black_enabled, vector<EE_GeometryInstance*> &instances );
void release_chess_board();
bool external_user_moved(int from_x, int from_y, int to_x, int to_y);
void reset_chess_board();
bool user_moved_chess_piece( EE_GeometryInstance *instance);
void pick_chess_piece( EE_GeometryInstance *instance );
void set_chess_engine_first( bool first );
void create_empty_chess_board();
void add_chess_board_piece( Piece *piece );
bool check_chess_engine_done( vector<EE_GeometryInstance *> &all_instances, vector<EE_Geometry *> &all_geometries, tinyxml2::XMLDocument *xmldocument );
void export_chess_state( tinyxml2::XMLDocument *xmldocument, tinyxml2::XMLElement *trondObjectElement, vector<EE_GeometryInstance*> &all_instances );
bool reinit_chess_state( vector<EE_GeometryInstance *> &all_instances );
void init_chess_thread();
EE_Scene *readXMLChessScene( const char *filename, tinyxml2::XMLDocument *doc,  EE_Scene *currentScene );
void saveXMLChessScene( const char *filename, EE_Scene *scene, tinyxml2::XMLDocument *xmldocument );
#endif /* _CHESS_HELPERS_H_ */