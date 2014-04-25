#ifndef _CALCULUS_H_
#define _CALCULUS_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "OSUFlow.h"



#define MAX_VAL 10000000.0f
#define MIN_VAL -10000000.0f

float angle_b_vector(VECTOR3& v1, VECTOR3& v2);
VECTOR3 mvmult(MATRIX3& matrix, VECTOR3& vector);
VECTOR3 crossProduct(VECTOR3& v1, VECTOR3& v2);
float dotProduct(VECTOR3& v1, VECTOR3& v2);
void vector2float(vector<vector<float> > m1, float** m2);
float standardDeviation(const vector<float>& v);
void normalized(vector<vector<float> >& data);

#endif
