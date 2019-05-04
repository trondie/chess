#ifndef MYMATH_H
#define MYMATH_H

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif
#define DEGTORAD( x ) ( (M_PI * (x)) / 180.0 )
#define RADTODEG( x ) ( 360.0f*(x) / (2*M_PI) )
#define MAX( a, b ) (((a) > (b)) ? (a) : (b))
#define MIN( a, b ) (((a) < (b)) ? (a) : (b))
#define CLAMP(a,b,i) (((i)<(a))?(a):(((i)>(b))?(b):(i)))
#define EE_GOLDEN_RATIO 1.61803399
#define FLOOR( x ) ((int)(x))
#define ROUND( x ) ((x) < 0 ? FLOOR( (x) - 0.5 ) : FLOOR( (x) + 0.5 ) )

typedef unsigned short int fp16;

void matInit(float *m, int size);
float *matMult(float *mA, float *vx, int vecsize);
float *matTransp(float *mA, int matsize);
float *vecNorm(float *pv,int vecsize);
float vecLength(float *pv, int vecsize);
float *vecCrossProd3(float u[3], float v[3]);
float *vecDotProd(float *u, float *v, int size);
int fastsgn(float f);
fp16 _float_to_fp16( float f );
float _fp16_to_float( fp16 f );
float get_random_float();
float lerp( float between, float a, float b);

#endif //MYMATH_H

