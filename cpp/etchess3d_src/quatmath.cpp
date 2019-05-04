#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "quatmath.h"

void quatInitMult(Quat *q) {
	q->w = 1.0f;
	q->x = 0.0f;
	q->y = 0.0f;
	q->z = 0.0f;
};

void matQuat(float m[16], Quat *q) {
	float diag = 1 + m[0] + m[5] + m[10];
	float scale;
	if(diag > 0.000000001) {
		//Positiv Diagonal
		scale = sqrt(diag)*2;
		q->x = ( m[9] - m[6] ) / scale;
		q->y = ( m[2] - m[8] ) / scale;
		q->z = ( m[4] - m[1] ) / scale;
		q->w = 0.25 * scale;
	} else {
		/* Diagonal Negativ eller 0.0. */
	    if ( m[0] > m[5] && m[0] > m[10] )  {

			scale  = sqrt( 1.0 + m[0] - m[5] - m[10] ) * 2;
			q->x = 0.25 * scale;
			q->y = (m[4] + m[1] ) / scale;
			q->z = (m[2] + m[8] ) / scale;
			q->w = (m[9] - m[6] ) / scale;
		} else if ( m[5] > m[10] ) {
			scale  = sqrt( 1.0 + m[5] - m[0] - m[10] ) * 2;
			q->x = (m[4] + m[1] ) / scale;
			q->y = 0.25 * scale;
			q->z = (m[9] + m[6] ) / scale;
			q->w = (m[2] - m[8] ) / scale;
		} else { 
			scale  = sqrt( 1.0 + m[10] - m[0] - m[5] ) * 2;
			q->x = (m[2] + m[8] ) / scale;
			q->y = (m[9] + m[6] ) / scale;
			q->z = 0.25 * scale;
			q->w = (m[4] - m[1] ) / scale;
		}
	}
};

void quatMat(Quat q, float *m) {
	float wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2; 
	float *mtemp;
	mtemp = m;

	x2 = q.x + q.x; y2 = q.y + q.y; 
	z2 = q.z + q.z;
	xx = q.x * x2; xy = q.x * y2; xz = q.x * z2;
	yy = q.y * y2; yz = q.y * z2; zz = q.z * z2;
	wx = q.w * x2; wy = q.w * y2; wz = q.w * z2;

	*mtemp = 1.0 - (yy + zz); 
	*(++mtemp) = xy - wz;
	*(++mtemp) = xz + wy; 
	++mtemp;	
	*(++mtemp) = xy + wz; 
	*(++mtemp) = 1.0 - (xx + zz);
	*(++mtemp) = yz - wx; 
	++mtemp;

	*(++mtemp) = xz - wy; 
	*(++mtemp) = yz + wx;
	*(++mtemp) = 1.0 - (xx + yy); 
	++mtemp;
};

float quatMag(Quat q) {
	return sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
};

void quatNorm(Quat *q) {
	float qm = quatMag(*q);
	if(float(fabs(float(1.0f - qm))) > QUAT_TOLERANCE) {
		q->x = q->x / qm;
		q->y = q->y / qm;
		q->z = q->z / qm;
		q->w = q->w / qm;
	}
};

void quatAxis(Quat q, float *rotvec, float *theta) {
	float qm = quatMag(q);
	if(qm != 0.0f) {
		rotvec[0] = q.x / qm;
		rotvec[1] = q.y / qm;
		rotvec[2] = q.z / qm;
		(*theta) = 2*acos(q.w);
	} else {
		rotvec[0] = 0.0f;
		rotvec[1] = 0.0f;
		rotvec[2] = 1.0f;
		(*theta) = 0.0f;
	}

};

void axisQuat(float rotvec[4], float theta, Quat *resultat) {
	float halftheta = theta / 2;
	halftheta = (halftheta*M_PI)/180;
	float msin = sin(halftheta);
	resultat->w = cos(halftheta);
	resultat->x = rotvec[0] * msin;
	resultat->y = rotvec[1] * msin;
	resultat->z = rotvec[2] * msin;
};


void quatMult(Quat q1, Quat q2, Quat *resultat) {
	/* q1*q2 = [w1*w2 - v1.dot.v2, v1.x.v2 + w1*v2 + w2*v1]
	 * Dvs. quaternion multiplikasjon gir quaternion-skalar w' = skalar1*skalar2 - 
	 * vector1 dotprodukt vector2. Og at quaternion-vektor v' = vector1 kryssprodukt vector2 +
	 * skalar1*vector2 + skalar2*vector1.
	 */
	float *v1dotv2 = vecDotProd(&q1.x,&q2.x,3);
	float *v1xv2 = vecCrossProd3(&q1.x,&q2.x);
	resultat->w = q1.w*q2.w - (v1dotv2[0] + v1dotv2[1] + v1dotv2[2]);
	resultat->x = v1xv2[0] + q1.w*q2.x + q2.w*q1.x;
	resultat->y = v1xv2[1] + q1.w*q2.y + q2.w*q1.y;
	resultat->z = v1xv2[2] + q1.w*q2.z + q2.w*q1.z;
	delete(v1xv2);
	delete(v1dotv2);
};

void slerp(Quat qa, Quat qb, double t, Quat *qm ) 
{

	// Calculate angle between them.
	double cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
	// if qa=qb or qa=-qb then theta = 0 and we can return qa
	if (abs(cosHalfTheta) >= 1.0){
		qm->w = qa.w;qm->x = qa.x;qm->y = qa.y;qm->z = qa.z;
		return;
	}
	// Calculate temporary values.
	double halfTheta = acos(cosHalfTheta);
	double sinHalfTheta = sqrt(1.0 - cosHalfTheta*cosHalfTheta);
	// if theta = 180 degrees then result is not fully defined
	// we could rotate around any axis normal to qa or qb
	if (fabs(sinHalfTheta) < 0.001){ // fabs is floating point absolute
		qm->w = (qa.w * 0.5 + qb.w * 0.5);
		qm->x = (qa.x * 0.5 + qb.x * 0.5);
		qm->y = (qa.y * 0.5 + qb.y * 0.5);
		qm->z = (qa.z * 0.5 + qb.z * 0.5);
		return;
	}
	double ratioA = sin((1 - t) * halfTheta) / sinHalfTheta;
	double ratioB = sin(t * halfTheta) / sinHalfTheta; 
	//calculate Quaternion.
	qm->w = (qa.w * ratioA + qb.w * ratioB);
	qm->x = (qa.x * ratioA + qb.x * ratioB);
	qm->y = (qa.y * ratioA + qb.y * ratioB);
	qm->z = (qa.z * ratioA + qb.z * ratioB);
	return;
}


