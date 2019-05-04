#include <stdio.h>
#include <math.h>
#include "mymath.h"
#include <stdlib.h>

float lerp( float between, float a, float b)
{
	return (1-between)*a + between*b;
}

float get_random_float()
{
	return 0.5f+0.5f*(float)rand()/(float)RAND_MAX;
}

float _fp16_to_float( fp16 v16 )
{
	int sign = v16 >> 15;
	int exponent = (v16 >> 10) & 0x1F;
	int mantissa = v16 & 0x3FF;
	float e, v = 0.f;
	int   i;

	for (i=9; i >= 0; i--)
	{
		v += (float)((mantissa & 0x1) * powf( 2.0f, -(i + 1.0f) ));
		mantissa >>= 1;
	}

 	if ( 0 != exponent )
	{
		v += 1.0f;

		e = (float)powf( 2.0f, (float)(exponent - 15) );

		v *= e;
	}

	if ( 0 != sign )
	{
		v = -v;
	}

	return v;
}

/* float-to-fp16 conversion without overflow handling */
fp16 _float_to_fp16( float f )
{
	union {
		float f;
		unsigned int i;
	} u;
	unsigned int f_bits;
	unsigned int fp16_sig;
	signed int xpo;
	unsigned int man;
	signed int fp16_xpo;
	unsigned int fp16_man;

	u.f = f;
	f_bits = u.i;
	fp16_sig = (f_bits >> (31-15)) & 0x8000;
	xpo = (f_bits >> 23) & 0xff;
	man = f_bits & 0x007fffff;

	fp16_xpo = xpo + (15-127);
	/* round to nearest, away from zero in case of a tie */
	fp16_man = (man + 0x1000) >> 13;
	/* mantissa carry */
	fp16_xpo += fp16_man >> 10;
	fp16_man &= 0x3ff;
	if (fp16_xpo <= 0) {
		/* underflow or zero */
		fp16_xpo = 0;
		fp16_man = 0;
	}

	return fp16_sig | (fp16_xpo << 10) | fp16_man;
}

void matInit(float *m,int size) {
	float *tm = m;
	for(int i = 0; i < size; i++)
		for(int j = 0; j < size; j++)
			*tm++ = (i == j);
};

//mA*vx = vb
float *matMult(float *mA, float *vx, int vecsize) {
	float *vb = new float[vecsize];
	for(int i = 0; i < vecsize; i++) {
		vb[i] = 0;
		for(int j = 0; j < vecsize; j++)
			vb[i] += vx[j]*mA[vecsize*j + i];
	}
	return vb;
};

float *matTransp(float *mA, int matsize) {
	float *mB = new float[matsize*matsize];
	for(int i = 0; i < matsize; i++)
		for(int j = 0; j < matsize; j++)
			mB[i*matsize + j] = mA[j*matsize + i];
	return mB;
};

float vecLength(float *pv,int vecsize) {
	float sumcartesian = 0;
	for(int i = 0; i < vecsize; i++)
		sumcartesian += pv[i]*pv[i];
	return sqrt(sumcartesian);
};

float *vecNorm(float *pv, int vecsize) {
	float l = vecLength(pv,vecsize);
	if(l != 0)
		for(int i = 0; i < vecsize; i++)
			pv[i] /= l;
	return pv;
}

int fastsgn(float f) {
	return 1+(((*(int*)&f)>>31)<<1);
}

float *vecCrossProd3(float u[3], float v[3]) {
	float *w = new float[3];
	w[0] = u[1]*v[2] - u[2]*v[1];
	w[1] = u[2]*v[0] - u[0]*v[2];
	w[2] = u[0]*v[1] - u[1]*v[0];
	return w;
};

float *vecDotProd(float *u, float *v,int size) {
	float *w = new float[size];
	for(int i = 0; i < size; i++)
		w[i] = u[i]*v[i];
	return w;
};

