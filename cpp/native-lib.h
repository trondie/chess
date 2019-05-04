//
// Created by Eskil on 07.11.2016.
//

#ifndef ETCHESS3D_NATIVE_LIB_H
#define ETCHESS3D_NATIVE_LIB_H
void *get_jvm();
void *get_activity();
void detach_main_thread();
void javaPerformMove(int from_x, int from_y, int to_x, int to_y);
void javaPerformWon(int opponent_is_cpu, int opponent_is_external, int opponent_is_local, int player_is_white);

#endif //ETCHESS3D_NATIVE_LIB_H
