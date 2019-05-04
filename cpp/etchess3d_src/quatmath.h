#include "mymath.h"

//Quaternion toleranse. Pga flyttalls feil.
#define QUAT_TOLERANCE 0.000001f

typedef struct DEF_QUAT {
  float w, x, y, z;
} Quat;

void quatInitMult(Quat *q);
void matQuat(float m[16], Quat *q);
void quatMat(Quat q, float *m);
float quatMag(Quat q);
void quatNorm(Quat *q);
void quatAxis(Quat q, float *rotVec, float *theta);
void axisQuat(float rotvec[4], float theta, Quat *resultat);
void quatMult(Quat q1, Quat q2, Quat *resultat);
void slerp(Quat qa, Quat qb, double t, Quat *qm );
