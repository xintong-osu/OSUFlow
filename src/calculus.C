#include "calculus.h"


float angle_b_vector(VECTOR3& v1, VECTOR3& v2)
{
	float angle;
	float mag1 = sqrt(pow(v1[0],2.0f)+pow(v1[1],2.0f)+pow(v1[2],2.0f));
	float mag2 = sqrt(pow(v2[0],2.0f)+pow(v2[1],2.0f)+pow(v2[2],2.0f));
	angle = acos((v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])/(mag1*mag2));
	return angle;
}

VECTOR3 mvmult(MATRIX3& matrix, VECTOR3& vector)
{
	VECTOR3 resultVector;
    // For each row in the matrix
    for (int i = 0; i < 3; i++)
    {
       // Ensure the matrix entry starts at zero
       resultVector[i] = 0.0f;

       // For each column-row of the inner vectors
       for (int j = 0; j < 3; j++)
       {
          resultVector[i] += matrix[i][j] * vector[j];
       }
     }
	return resultVector;
}

VECTOR3 crossProduct(VECTOR3& v1, VECTOR3& v2)
{
	VECTOR3 result;
	result[0] = v1[1]*v2[2] - v1[2] * v2[1];
	result[1] = v1[2]*v2[0] - v1[0] * v2[2];
	result[2] = v1[0]*v2[1] - v1[1] * v2[0];
	return result;
}

float dotProduct(VECTOR3& v1, VECTOR3& v2)
{
	float result;
	result = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
	return result;
}

void vector2float(vector<vector<float> >& m1, float** m2)
{
    int row=m1.size();
    int col=m1[0].size();
    m2 = (float**)malloc(row*sizeof(float*));
    for(int i=0 ;i<m1.size(); i++)
    {
        m2[i] = (float*)malloc(col*sizeof(float));
    }
    for(int i=0 ;i<row; i++)
        for(int j=0;i<col;j++)
        {
            m2[i][j] = m1[i][j];
        }
}

float standardDeviation(const vector<float>& v)
{
    int j;
    float result;
    float mean = 0.0f;
    for(j=0;j<v.size();j++)
    {
            mean = mean + v[j];
    }
    mean = mean/(float)v.size();

    result = 0.0f;
    for(j=0;j<v.size();j++)
    {
            result = result + pow((v[j] - mean),2.0f);
    }
     result = sqrt(result/(float)v.size());
     return result;
}

// <summary>
// Normalized the whold matrix to be between 0 and 1
// </summary>
// <param></param>
// <returns></returns>
void normalized(vector<vector<float> > &data)
{
    float max = MIN_VAL;
    vector<float>::iterator it;
    int i,j;
    for(i=0;i<data.size();i++)
        for(it=data[i].begin();it!=data[i].end();it++)
        {
            if(max < *it)
                max = *it;
        }
    if(max > 0)
    {
        for(i=0;i<data.size();i++)
            for(j=0;j<data[i].size();j++)
            {
                data[i][j] = data[i][j] / max;
            }

    }
}

