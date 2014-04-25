#include <FieldLine.h>
#include "StreamSurface.h"
#include <time.h>
#include <fstream>
//#include "cp_time.h"
#define MAX_ANGLE 30
#define MIN_ANGLE 5
#define MIN_DIS 1
#define PI 3.1416
#define ERROR 0.1
#define PROBABILITY .85
#define X_DIM 126
#define Y_DIM 126
#define Z_DIM 512
#define GARTH 0
#define QUAD 1
#define CALCULATE_GLOBAL_ERROR 0	// 1 for global, 2 for local, 0 for no error
#define BIN_HIST 100
double ANGLE_THRESHOLD = 0;
float scalar[X_DIM][Y_DIM][Z_DIM][2]={0.0};
//#define FACTOR 255.0/37.89
float lowF[3]; //= {0, 0, 0};
float highF[3]; //= {3, 3, 7};
VECTOR3 data[128];
int nBin = 360;
int nDim = 3;
//ITL_field_regular<VECTOR3> *vectorField = NULL;
//ITL_globalentropy<VECTOR3> *globalEntropyComputer = NULL;
float hist2[BIN_HIST]={0};
double sum_area=0.0,local_area=0.0;

float a_length(VECTOR3 pt1, VECTOR3 pt2)
{
	return sqrt((pt1[0]-pt2[0])*(pt1[0]-pt2[0])+(pt1[1]-pt2[1])*(pt1[1]-pt2[1])+(pt1[2]-pt2[2])*(pt1[2]-pt2[2]));
}

VECTOR3 crossproduct_new(VECTOR3 v2,VECTOR3 v1)
{
        VECTOR3 vec;
        float val;
        vec[0]=v2[1]*v1[2]-v2[2]*v1[1];
        vec[1]=v2[2]*v1[0]-v2[0]*v1[2];
        vec[2]=v2[0]*v1[1]-v2[1]*v1[0];

        return vec;
}

double vectorvalue(VECTOR3 vec)
{

        return fabs(sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]));
}

bool equal(vtParticleInfo* A,vtParticleInfo* B)
{
  float	error=.000001;
	if((fabs(A->m_pointInfo.phyCoord[0]-B->m_pointInfo.phyCoord[0])<error)&&(fabs(A->m_pointInfo.phyCoord[1]-B->m_pointInfo.phyCoord[1])<error)&&(fabs(A->m_pointInfo.phyCoord[2]-B->m_pointInfo.phyCoord[2])<error))
	return 1;
	else return 0;
}

void rgbmap(float val,float &r,float &g,float &b)
{
	//r=0.0;g=0.0;b=1.0;
	//return;
	if(val<0 || val>255)
	cout<<"why would u want to do that?? give rgb man..!!"<<endl;
	//val=val-12;
	if(val<=128)
	{
		r=0;
		g=(float)val/(float)128;
		b=(float)(128-val)/(float)128;
	}
	else
	{
                r=(float)(val-128)/(float)128;
                g=(float)(255-val)/(float)128;
		b=0;
	}
}

VECTOR3 normalized(VECTOR3 vec)
{
	float val;
	val=fabs(sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]));
	if(val==0.0f)
	  val=1.0f;
	vec[0]=vec[0]/val;
	vec[1]=vec[1]/val;
	vec[2]=vec[2]/val;	

	return vec;
}
float dotproduct(VECTOR3 v1,VECTOR3 v2)
{
	return (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]);
}

VECTOR3 crossproduct(VECTOR3 v2,VECTOR3 v1)
{
	VECTOR3 vec;
	float val;
	vec[0]=v2[1]*v1[2]-v2[2]*v1[1];
	vec[1]=v2[2]*v1[0]-v2[0]*v1[2];
	vec[2]=v2[0]*v1[1]-v2[1]*v1[0];
	val=fabs(sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]));
	if(val==0.0f)
	  val=1.0f;
	vec[0]=vec[0]/val;
	vec[1]=vec[1]/val;
	vec[2]=vec[2]/val;	

	return vec;
}

void calculateNormal(VECTOR3 *pts,VECTOR3 * norms)
{
	VECTOR3 vec1,vec2;
	vec1[0]=pts[3][0]-pts[0][0];
	vec1[1]=pts[3][1]-pts[0][1];
	vec1[2]=pts[3][2]-pts[0][2];

	vec2[0]=pts[1][0]-pts[0][0];
	vec2[1]=pts[1][1]-pts[0][1];
	vec2[2]=pts[1][2]-pts[0][2];

	norms[0]= crossproduct(vec2,vec1);

	vec1[0]=pts[0][0]-pts[1][0];
	vec1[1]=pts[0][1]-pts[1][1];
	vec1[2]=pts[0][2]-pts[1][2];

	vec2[0]=pts[2][0]-pts[1][0];
	vec2[1]=pts[2][1]-pts[1][1];
	vec2[2]=pts[2][2]-pts[1][2];

	norms[1]= crossproduct(vec2,vec1);

	vec1[0]=pts[1][0]-pts[2][0];
	vec1[1]=pts[1][1]-pts[2][1];
	vec1[2]=pts[1][2]-pts[2][2];

	vec2[0]=pts[3][0]-pts[2][0];
	vec2[1]=pts[3][1]-pts[2][1];
	vec2[2]=pts[3][2]-pts[2][2];

	norms[2]= crossproduct(vec2,vec1);

	vec1[0]=pts[2][0]-pts[3][0];
	vec1[1]=pts[2][1]-pts[3][1];
	vec1[2]=pts[2][2]-pts[3][2];

	vec2[0]=pts[0][0]-pts[3][0];
	vec2[1]=pts[0][1]-pts[3][1];
	vec2[2]=pts[0][2]-pts[3][2];

	norms[3]= crossproduct(vec2,vec1);

}

float errorAnalysisTriangle(VECTOR3 *points, CVectorField* m_pField)
{
	float a00,a11,a22,a01,a10,a20,a02,a12,a21;
	float xcen,ycen,zcen;
	float matrix[3][3], evals[3],evecs[3][3];
	float a,b,c,d;		// for plane ax+by+cz+d=0
	float least_eval=65536;
	int count=0;
	float distance[4];
	VECTOR3 newPoints[4],ptA,ptB,ptF,normal;
	int randomPoints=60;
	double randf;
	VECTOR3 nodeData;
	VECTOR3 vec1,vec2,vec3;
	PointInfo pointInfo;
	int res;
	float sumDots=0;
	float area=0.0,s;
	float maxModErr=0.0;
	float rand1=0.0,rand2=0.0,rand3=0.0;

	//calculate the area of the triangle  
        a=a_length(points[0],points[1]);
        b=a_length(points[0],points[2]);
        c=a_length(points[2],points[1]);
        s=(a+b+c)/2.0;
        area=sqrt(s*(s-a)*(s-b)*(s-c));

      //  VECTOR3 vec1,vec2;

        vec1[0]=points[2][0]-points[0][0];
       	vec1[1]=points[2][1]-points[0][1];
       	vec1[2]=points[2][2]-points[0][2];

       	vec2[0]=points[1][0]-points[0][0];
       	vec2[1]=points[1][1]-points[0][1];
       	vec2[2]=points[1][2]-points[0][2];

        normal= crossproduct(vec2,vec1);


	//cout<<"The plane is A="<<a<<" B="<<b<<" C="<<c<<" D="<<d<<endl;
	for( int i=0;i<randomPoints;i++)
	{
		//randf=1;		
		while(1)
		{
			rand1=(double)rand()/(RAND_MAX);
			rand2=(double)rand()/(RAND_MAX);
			if( (rand1+rand2) < 1.0)
			break;
		}
		rand3 = 1 - ( rand1 + rand2 );


		ptF[0]=rand1*points[0][0] + rand2*points[1][0] + rand3*points[2][0] ;
		ptF[1]=rand1*points[0][1] + rand2*points[1][1] + rand3*points[2][1] ;
		ptF[2]=rand1*points[0][2] + rand2*points[1][2] + rand3*points[2][2] ;
		//cout<<"The pointF is evaluated to "<<a*ptF[0]+b*ptF[1]+c*ptF[2]-d<<endl;

		pointInfo.phyCoord=ptF;
		pointInfo.interpolant[0]=0;	// interpolation coefficients
		pointInfo.interpolant[1]=0;	// interpolation coefficients
		pointInfo.interpolant[2]=0;	// interpolation coefficients
		pointInfo.fromCell=-1;	// advecting result from which cell, mainly used for unstructured grid
		pointInfo.inCell=-1;
		res=m_pField->at_phys(-1, ptF, pointInfo, 0, nodeData);
		//cout<<"location is "<<pointInfo.phyCoord[0]<<" "<<pointInfo.phyCoord[1]<<" "<<pointInfo.phyCoord[2]<<endl;
		
		nodeData=normalized(nodeData);
		data[i]=nodeData;
		//cout<<"NodeData at this point is "<<data[i][0]<<" "<<data[i][1]<<" "<<data[i][2]<<endl;
		
		sumDots+=fabs((nodeData[0]*normal[0]+nodeData[1]*normal[1]+nodeData[2]*normal[2])*(nodeData[0]*normal[0]+nodeData[1]*normal[1]+nodeData[2]*normal[2]));
		//cout<<"The dotproduct is "<<sumDots<<endl;
	}
	
	float retval=sqrt(sumDots/randomPoints);
//	if(retval*area > maxModErr)
//	maxModErr=retval*area;
	if(!isnan(area))
	{
		local_area=area;
		sum_area+=area;
	}
	else local_area=0.0;

	if(isnan(retval))
	{
		cout<<"retval is bad"<<endl;
	}
	if(isnan(local_area))
	{
		cout<<"local area is bad "<<local_area<<endl;
	}
	//cout<<"area weighted value is "<<retval<<endl;
	return retval;
}

float errorAnalysisQuad(VECTOR3 *points, CVectorField* m_pField)
{
	float a00,a11,a22,a01,a10,a20,a02,a12,a21;
	float xcen,ycen,zcen;
	float matrix[3][3], evals[3],evecs[3][3];
	float a,b,c,d;		// for plane ax+by+cz+d=0
	float least_eval=65536;
	int count=0;
	float distance[4];
	VECTOR3 newPoints[4],ptA,ptB,ptF,normals[4],ptNA,ptNB,ptNF;
	int randomPoints=60;
	double randf;
	VECTOR3 nodeData;
	VECTOR3 vec1,vec2,vec3;
	PointInfo pointInfo;
	int res;
	float sumDots=0;
	float area=0.0,s;
	float maxModErr=0.0;

      	//calculate the area of 4 input points
	a=a_length(points[0],points[1]);
	b=a_length(points[0],points[2]);
	c=a_length(points[2],points[1]);
	s=(a+b+c)/2.0;
	area=sqrt(s*(s-a)*(s-b)*(s-c));

	a=a_length(points[0],points[3]);
	b=a_length(points[0],points[2]);
	c=a_length(points[2],points[3]);
	s=(a+b+c)/2.0;
	area+=sqrt(s*(s-a)*(s-b)*(s-c));

	srand ( time(NULL) );

	calculateNormal(points,normals);


	sumDots=0;
	//cout<<"The plane is A="<<a<<" B="<<b<<" C="<<c<<" D="<<d<<endl;
	for( int i=0;i<randomPoints;i++)
	{
		//randf=1;
		randf=(double)rand()/(RAND_MAX);// TO-DO change to randf=(double)rand()/(RAND_MAX);

		ptA[0]=points[0][0]+ (points[3][0]-points[0][0])*randf;
		ptA[1]=points[0][1]+ (points[3][1]-points[0][1])*randf;
		ptA[2]=points[0][2]+ (points[3][2]-points[0][2])*randf;

		ptNA[0]=normals[0][0]+ (normals[3][0]-normals[0][0])*randf;
		ptNA[1]=normals[0][1]+ (normals[3][1]-normals[0][1])*randf;
		ptNA[2]=normals[0][2]+ (normals[3][2]-normals[0][2])*randf;

		ptB[0]=points[1][0]+ (points[2][0]-points[1][0])*randf;
		ptB[1]=points[1][1]+ (points[2][1]-points[1][1])*randf;
		ptB[2]=points[1][2]+ (points[2][2]-points[1][2])*randf;

		ptNB[0]=normals[1][0]+ (normals[2][0]-normals[1][0])*randf;
		ptNB[1]=normals[1][1]+ (normals[2][1]-normals[1][1])*randf;
		ptNB[2]=normals[1][2]+ (normals[2][2]-normals[1][2])*randf;

		randf=(double)rand()/(RAND_MAX); // TO-DO change to randf=(double)rand()/(RAND_MAX);

		ptF[0]=ptA[0]+ (ptB[0]-ptA[0])*randf;
		ptF[1]=ptA[1]+ (ptB[1]-ptA[1])*randf;
		ptF[2]=ptA[2]+ (ptB[2]-ptA[2])*randf;

		ptNF[0]=ptNA[0]+ (ptNB[0]-ptNA[0])*randf;
		ptNF[1]=ptNA[1]+ (ptNB[1]-ptNA[1])*randf;
		ptNF[2]=ptNA[2]+ (ptNB[2]-ptNA[2])*randf;
		//cout<<"The pointF is evaluated to "<<a*ptF[0]+b*ptF[1]+c*ptF[2]-d<<endl;

		pointInfo.phyCoord=ptF;
		pointInfo.interpolant[0]=0;	// interpolation coefficients
		pointInfo.interpolant[1]=0;	// interpolation coefficients
		pointInfo.interpolant[2]=0;	// interpolation coefficients
		pointInfo.fromCell=-1;	// advecting result from which cell, mainly used for unstructured grid
		pointInfo.inCell=-1;
		res=m_pField->at_phys(-1, ptF, pointInfo, 0, nodeData);
		//cout<<"location is "<<pointInfo.phyCoord[0]<<" "<<pointInfo.phyCoord[1]<<" "<<pointInfo.phyCoord[2]<<endl;

		nodeData=normalized(nodeData);
		data[i]=nodeData;
		//cout<<"NodeData at this point is "<<data[i][0]<<" "<<data[i][1]<<" "<<data[i][2]<<endl;

		sumDots+=fabs((nodeData[0]*ptNF[0]+nodeData[1]*ptNF[1]+nodeData[2]*ptNF[2])*(nodeData[0]*ptNF[0]+nodeData[1]*ptNF[1]+nodeData[2]*ptNF[2]));
		//cout<<"The dotproduct is "<<sumDots<<endl;
	}
	//cout<<"The sum of dotproducts is "<<sumDots<<endl;

	float retval=sqrt(sumDots/randomPoints);
//	if(retval*area > maxModErr)
//	maxModErr=retval*area;
	if(!isnan(area))
	{
		local_area=area;
		sum_area+=area;
	}
	else local_area=0.0;

	if(isnan(retval))
	{
		cout<<"retval is bad"<<endl;
	}
	if(isnan(local_area))
	{
		cout<<"local area is bad "<<local_area<<endl;
	}
	//cout<<"area weighted value is "<<retval<<endl;
	return retval;
}

float errorAnalysis(VECTOR3 *points, CVectorField* m_pField)
{
	float a00,a11,a22,a01,a10,a20,a02,a12,a21;
	float xcen,ycen,zcen;
	float matrix[3][3], evals[3],evecs[3][3];
	float a,b,c,d;		// for plane ax+by+cz+d=0
	float least_eval=65536;
	int count=0;
	float distance[4];
	VECTOR3 newPoints[4],ptA,ptB,ptF;
	int randomPoints=60;
	double randf;
	VECTOR3 nodeData;
	VECTOR3 vec1,vec2,vec3;
	PointInfo pointInfo;
	int res;
	float sumDots=0;
	float area=0.0,s;
	float maxModErr=0.0;
      	//calculate the area of 4 input points  
	a=a_length(points[0],points[1]);	
	b=a_length(points[0],points[2]);	
	c=a_length(points[2],points[1]);	
	s=(a+b+c)/2.0;
	area=sqrt(s*(s-a)*(s-b)*(s-c));

	a=a_length(points[0],points[3]);	
	b=a_length(points[0],points[2]);	
	c=a_length(points[2],points[3]);	
	s=(a+b+c)/2.0;
	area+=sqrt(s*(s-a)*(s-b)*(s-c));
	
	srand ( time(NULL) );

	xcen=(points[0][0]+points[1][0]+points[2][0]+points[3][0])/4;
	ycen=(points[0][1]+points[1][1]+points[2][1]+points[3][1])/4;
	zcen=(points[0][2]+points[1][2]+points[2][2]+points[3][2])/4;
	
	a00=((points[0][0]-xcen)*(points[0][0]-xcen)+(points[1][0]-xcen)*(points[1][0]-xcen)+(points[2][0]-xcen)*(points[2][0]-xcen)+(points[3][0]-xcen)*(points[3][0]-xcen))/4;
	a11=((points[0][1]-ycen)*(points[0][1]-ycen)+(points[1][1]-ycen)*(points[1][1]-ycen)+(points[2][1]-ycen)*(points[2][1]-ycen)+(points[3][1]-ycen)*(points[3][1]-ycen))/4;
	a22=((points[0][2]-zcen)*(points[0][2]-zcen)+(points[1][2]-zcen)*(points[1][2]-zcen)+(points[2][2]-zcen)*(points[2][2]-zcen)+(points[3][2]-zcen)*(points[3][2]-zcen))/4;
	a01=((points[0][0]-xcen)*(points[0][1]-ycen)+(points[1][0]-xcen)*(points[1][1]-ycen)+(points[2][0]-xcen)*(points[2][1]-ycen)+(points[3][0]-xcen)*(points[3][1]-ycen))/4;
	a10=a01;
	a02=((points[0][0]-xcen)*(points[0][2]-zcen)+(points[1][0]-xcen)*(points[1][2]-zcen)+(points[2][0]-xcen)*(points[2][2]-zcen)+(points[3][0]-xcen)*(points[3][2]-zcen))/4;
	a20=a02;
	a12=((points[0][1]-ycen)*(points[0][2]-zcen)+(points[1][1]-ycen)*(points[1][2]-zcen)+(points[2][1]-ycen)*(points[2][2]-zcen)+(points[3][1]-ycen)*(points[3][2]-zcen))/4;
	a21=a12;

	matrix[0][0]= a00;
	matrix[0][1]= a01;
	matrix[0][2]= a02;
	matrix[1][0]= a10; 
	matrix[1][1]= a11;
	matrix[1][2]= a12;
	matrix[2][0]= a20;
	matrix[2][1]= a21;
	matrix[2][2]= a22;

	compute_eigenvalues(matrix, evals);	
	compute_real_eigenvectors(matrix, evals, evecs);
	for(int i=0;i<3;i++)
	{
		if(least_eval>evals[i])
			{
				least_eval=evals[i];
				count=i;
			}
	}
	a=evecs[count][0];
	b=evecs[count][1];
	c=evecs[count][2];
	d=a*xcen+b*ycen+c*zcen;
	//cout<<"The plane values are "<<a<<" "<<b<<" "<<c<<" "<<d<<endl;

	distance[0]=(points[0][0]*a+points[0][1]*b+points[0][2]*c-d)/sqrt(a*a+b*b+c*c);
	distance[1]=(points[1][0]*a+points[1][1]*b+points[1][2]*c-d)/sqrt(a*a+b*b+c*c);
	distance[2]=(points[2][0]*a+points[2][1]*b+points[2][2]*c-d)/sqrt(a*a+b*b+c*c);
	distance[3]=(points[3][0]*a+points[3][1]*b+points[3][2]*c-d)/sqrt(a*a+b*b+c*c);

	newPoints[0][0]=points[0][0]-distance[0]*a/sqrt(a*a+b*b+c*c);
	newPoints[0][1]=points[0][1]-distance[0]*b/sqrt(a*a+b*b+c*c);
	newPoints[0][2]=points[0][2]-distance[0]*c/sqrt(a*a+b*b+c*c);
	//cout<<"The new point 0 is evaluated to "<<a*newPoints[0][0]+b*newPoints[0][1]+c*newPoints[0][2]-d<<endl;

	newPoints[1][0]=points[1][0]-distance[1]*a/sqrt(a*a+b*b+c*c);
	newPoints[1][1]=points[1][1]-distance[1]*b/sqrt(a*a+b*b+c*c);
	newPoints[1][2]=points[1][2]-distance[1]*c/sqrt(a*a+b*b+c*c);
	//cout<<"The new point 1 is evaluated to "<<a*newPoints[1][0]+b*newPoints[1][1]+c*newPoints[1][2]-d<<endl;

	newPoints[2][0]=points[2][0]-distance[2]*a/sqrt(a*a+b*b+c*c);
	newPoints[2][1]=points[2][1]-distance[2]*b/sqrt(a*a+b*b+c*c);
	newPoints[2][2]=points[2][2]-distance[2]*c/sqrt(a*a+b*b+c*c);
	//cout<<"The new point 2 is evaluated to "<<a*newPoints[2][0]+b*newPoints[2][1]+c*newPoints[2][2]-d<<endl;

	newPoints[3][0]=points[3][0]-distance[3]*a/sqrt(a*a+b*b+c*c);
	newPoints[3][1]=points[3][1]-distance[3]*b/sqrt(a*a+b*b+c*c);
	newPoints[3][2]=points[3][2]-distance[3]*c/sqrt(a*a+b*b+c*c);
	//cout<<"The new point 3 is evaluated to "<<a*newPoints[3][0]+b*newPoints[3][1]+c*newPoints[3][2]-d<<endl;
	
	vec1[0]=points[1][0]-points[0][0];
	vec1[1]=points[1][1]-points[0][1];
	vec1[2]=points[1][2]-points[0][2];

	vec2[0]=points[3][0]-points[0][0];
	vec2[1]=points[3][1]-points[0][1];
	vec2[2]=points[3][2]-points[0][2];

	vec3=crossproduct(vec1,vec2);
	int bad=0;
	if(a*vec3[0]+b*vec3[1]+c*vec3[2]+0.00005<0)
	{
		//cout<<"Normal looks bad"<<endl;
		bad++;
		a=-a;
		b=-b;
		c=-c;
	}
	//cout<<"Bad normals "<<bad<<endl;
	sumDots=0;
	//cout<<"The plane is A="<<a<<" B="<<b<<" C="<<c<<" D="<<d<<endl;
	for( int i=0;i<randomPoints;i++)
	{
		//randf=1;		
		randf=(double)rand()/(RAND_MAX+1);// TO-DO change to randf=(double)rand()/(RAND_MAX);

		ptA[0]=newPoints[0][0]+ (newPoints[3][0]-newPoints[0][0])*randf;
		ptA[1]=newPoints[0][1]+ (newPoints[3][1]-newPoints[0][1])*randf;
		ptA[2]=newPoints[0][2]+ (newPoints[3][2]-newPoints[0][2])*randf;

		ptB[0]=newPoints[1][0]+ (newPoints[2][0]-newPoints[1][0])*randf;
		ptB[1]=newPoints[1][1]+ (newPoints[2][1]-newPoints[1][1])*randf;
		ptB[2]=newPoints[1][2]+ (newPoints[2][2]-newPoints[1][2])*randf;

		randf=(double)rand()/(RAND_MAX+1); // TO-DO change to randf=(double)rand()/(RAND_MAX);

		ptF[0]=ptA[0]+ (ptB[0]-ptA[0])*randf;
		ptF[1]=ptA[1]+ (ptB[1]-ptA[1])*randf;
		ptF[2]=ptA[2]+ (ptB[2]-ptA[2])*randf;
		//cout<<"The pointF is evaluated to "<<a*ptF[0]+b*ptF[1]+c*ptF[2]-d<<endl;

		pointInfo.phyCoord=ptF;
		pointInfo.interpolant[0]=0;	// interpolation coefficients
		pointInfo.interpolant[1]=0;	// interpolation coefficients
		pointInfo.interpolant[2]=0;	// interpolation coefficients
		pointInfo.fromCell=-1;	// advecting result from which cell, mainly used for unstructured grid
		pointInfo.inCell=-1;
		res=m_pField->at_phys(-1, ptF, pointInfo, 0, nodeData);
		//cout<<"location is "<<pointInfo.phyCoord[0]<<" "<<pointInfo.phyCoord[1]<<" "<<pointInfo.phyCoord[2]<<endl;
		
		nodeData=normalized(nodeData);
		data[i]=nodeData;
		//cout<<"NodeData at this point is "<<data[i][0]<<" "<<data[i][1]<<" "<<data[i][2]<<endl;
		
		sumDots+=fabs((nodeData[0]*a+nodeData[1]*b+nodeData[2]*c)*(nodeData[0]*a+nodeData[1]*b+nodeData[2]*c));
		//cout<<"The dotproduct is "<<sumDots<<endl;
	}
	//cout<<"The sum of dotproducts is "<<sumDots<<endl;
	/*lowF[0] = lowF[1] = lowF[2] = 0;
	highF[0] = 3; highF[1] = 3; highF[2] = 7;
	ITL_field_regular<VECTOR3> *vectorField= new ITL_field_regular<VECTOR3>(data,nDim,lowF,highF);
	ITL_globalentropy<VECTOR3> *globalEntropyComputer= new ITL_globalentropy<VECTOR3>(vectorField);
	//ITL_field_regular<VECTOR3> vectorField(data,nDim,lowF,highF);
	//ITL_globalentropy<VECTOR3> globalEntropyComputer(&vectorField);
	
	globalEntropyComputer->computeHistogramBinField( "vector", 360 );
	globalEntropyComputer->computeGlobalEntropyOfField( 360, true );
        int freqList[360];
	globalEntropyComputer->getHistogramFrequencies( 360, freqList );
	for( int i=0; i<360; i++ )
		cout << freqList[i] << "\t";
	cout << endl;
	if(globalEntropyComputer->getGlobalEntropy()>0.00000005)	
	cout<<"\nEntropy is "<<globalEntropyComputer->getGlobalEntropy()<< endl;

	delete globalEntropyComputer;
	delete vectorField;*/
	float retval=sqrt(sumDots/randomPoints);
//	if(retval*area > maxModErr)
//	maxModErr=retval*area;
	if(!isnan(area))
	{
		local_area=area;
		sum_area+=area;
	}
	else local_area=0.0;

	if(isnan(retval))
	{
		cout<<"retval is bad"<<endl;
	}
	if(isnan(local_area))
	{
		cout<<"local area is bad "<<local_area<<endl;
	}
	//cout<<"area weighted value is "<<retval<<endl;
	return retval;
}

float vtCStreamSurface::errorAnalysisGlobal(VECTOR3 *points, vtListParticle m_lSeeds1, 
									float dt, int min_steps)
{
	float a00,a11,a22,a01,a10,a20,a02,a12,a21;
	float xcen,ycen,zcen;
	float matrix[3][3], evals[3],evecs[3][3];
	float a,b,c,d;		// for plane ax+by+cz+d=0
	float least_eval=65536;
	int count=0;
	float distance[4];
	VECTOR3 newPoints[4],ptA,ptB,ptF;
	int randomPoints=60;
	double randf;
	VECTOR3 nodeData;
	VECTOR3 vec1,vec2,vec3;
	PointInfo pointInfo;
	int res;
	float sumDots=0;
	vtListParticleIter sIter;
	//cout<<"computing histogram"<<endl;
      
	float area=0.0,s;
        float maxModErr=0.0;
	float integ_error=0.0;
        //calculate the area of 4 input points  
        a=a_length(points[0],points[1]);
        b=a_length(points[0],points[2]);
        c=a_length(points[2],points[1]);
        s=(a+b+c)/2.0;
        area=sqrt(s*(s-a)*(s-b)*(s-c));

        a=a_length(points[0],points[3]);
        b=a_length(points[0],points[2]);
        c=a_length(points[2],points[3]);
        s=(a+b+c)/2.0;
        area+=sqrt(s*(s-a)*(s-b)*(s-c));
  

	srand ( time(NULL) );

	xcen=(points[0][0]+points[1][0]+points[2][0]+points[3][0])/4;
	ycen=(points[0][1]+points[1][1]+points[2][1]+points[3][1])/4;
	zcen=(points[0][2]+points[1][2]+points[2][2]+points[3][2])/4;
	
	a00=((points[0][0]-xcen)*(points[0][0]-xcen)+(points[1][0]-xcen)*(points[1][0]-xcen)+(points[2][0]-xcen)*(points[2][0]-xcen)+(points[3][0]-xcen)*(points[3][0]-xcen))/4;
	a11=((points[0][1]-ycen)*(points[0][1]-ycen)+(points[1][1]-ycen)*(points[1][1]-ycen)+(points[2][1]-ycen)*(points[2][1]-ycen)+(points[3][1]-ycen)*(points[3][1]-ycen))/4;
	a22=((points[0][2]-zcen)*(points[0][2]-zcen)+(points[1][2]-zcen)*(points[1][2]-zcen)+(points[2][2]-zcen)*(points[2][2]-zcen)+(points[3][2]-zcen)*(points[3][2]-zcen))/4;
	a01=((points[0][0]-xcen)*(points[0][1]-ycen)+(points[1][0]-xcen)*(points[1][1]-ycen)+(points[2][0]-xcen)*(points[2][1]-ycen)+(points[3][0]-xcen)*(points[3][1]-ycen))/4;
	a10=a01;
	a02=((points[0][0]-xcen)*(points[0][2]-zcen)+(points[1][0]-xcen)*(points[1][2]-zcen)+(points[2][0]-xcen)*(points[2][2]-zcen)+(points[3][0]-xcen)*(points[3][2]-zcen))/4;
	a20=a02;
	a12=((points[0][1]-ycen)*(points[0][2]-zcen)+(points[1][1]-ycen)*(points[1][2]-zcen)+(points[2][1]-ycen)*(points[2][2]-zcen)+(points[3][1]-ycen)*(points[3][2]-zcen))/4;
	a21=a12;

	matrix[0][0]= a00;
	matrix[0][1]= a01;
	matrix[0][2]= a02;
	matrix[1][0]= a10; 
	matrix[1][1]= a11;
	matrix[1][2]= a12;
	matrix[2][0]= a20;
	matrix[2][1]= a21;
	matrix[2][2]= a22;

	compute_eigenvalues(matrix, evals);	
	compute_real_eigenvectors(matrix, evals, evecs);
	for(int i=0;i<3;i++)
	{
		if(least_eval>evals[i])
			{
				least_eval=evals[i];
				count=i;
			}
	}
	a=evecs[count][0];
	b=evecs[count][1];
	c=evecs[count][2];
	d=a*xcen+b*ycen+c*zcen;
	//cout<<"The plane values are "<<a<<" "<<b<<" "<<c<<" "<<d<<endl;

	distance[0]=(points[0][0]*a+points[0][1]*b+points[0][2]*c-d)/sqrt(a*a+b*b+c*c);
	distance[1]=(points[1][0]*a+points[1][1]*b+points[1][2]*c-d)/sqrt(a*a+b*b+c*c);
	distance[2]=(points[2][0]*a+points[2][1]*b+points[2][2]*c-d)/sqrt(a*a+b*b+c*c);
	distance[3]=(points[3][0]*a+points[3][1]*b+points[3][2]*c-d)/sqrt(a*a+b*b+c*c);

	newPoints[0][0]=points[0][0]-distance[0]*a/sqrt(a*a+b*b+c*c);
	newPoints[0][1]=points[0][1]-distance[0]*b/sqrt(a*a+b*b+c*c);
	newPoints[0][2]=points[0][2]-distance[0]*c/sqrt(a*a+b*b+c*c);
	//cout<<"The new point 0 is evaluated to "<<a*newPoints[0][0]+b*newPoints[0][1]+c*newPoints[0][2]-d<<endl;

	newPoints[1][0]=points[1][0]-distance[1]*a/sqrt(a*a+b*b+c*c);
	newPoints[1][1]=points[1][1]-distance[1]*b/sqrt(a*a+b*b+c*c);
	newPoints[1][2]=points[1][2]-distance[1]*c/sqrt(a*a+b*b+c*c);
	//cout<<"The new point 1 is evaluated to "<<a*newPoints[1][0]+b*newPoints[1][1]+c*newPoints[1][2]-d<<endl;

	newPoints[2][0]=points[2][0]-distance[2]*a/sqrt(a*a+b*b+c*c);
	newPoints[2][1]=points[2][1]-distance[2]*b/sqrt(a*a+b*b+c*c);
	newPoints[2][2]=points[2][2]-distance[2]*c/sqrt(a*a+b*b+c*c);
	//cout<<"The new point 2 is evaluated to "<<a*newPoints[2][0]+b*newPoints[2][1]+c*newPoints[2][2]-d<<endl;

	newPoints[3][0]=points[3][0]-distance[3]*a/sqrt(a*a+b*b+c*c);
	newPoints[3][1]=points[3][1]-distance[3]*b/sqrt(a*a+b*b+c*c);
	newPoints[3][2]=points[3][2]-distance[3]*c/sqrt(a*a+b*b+c*c);
	//cout<<"The new point 3 is evaluated to "<<a*newPoints[3][0]+b*newPoints[3][1]+c*newPoints[3][2]-d<<endl;
	
	vec1[0]=points[1][0]-points[0][0];
	vec1[1]=points[1][1]-points[0][1];
	vec1[2]=points[1][2]-points[0][2];

	vec2[0]=points[3][0]-points[0][0];
	vec2[1]=points[3][1]-points[0][1];
	vec2[2]=points[3][2]-points[0][2];

	vec3=crossproduct(vec1,vec2);
	int bad=0;
	if(a*vec3[0]+b*vec3[1]+c*vec3[2]+0.00005<0)
	{
		//cout<<"Normal looks bad"<<endl;
		bad++;
		a=-a;
		b=-b;
		c=-c;
	}
	//cout<<"Bad normals "<<bad<<endl;
	sumDots=0;

        double sigma=0.0,mean=0.0;
        double globalError=0.0;
        double old_Gprob=0.0,new_Gprob=0.0;
        double min_distance=999999999999999.9,curve_min,grid_min,dist1,min_st_dis;
        double t_min=9999999999999.9;
        int OUT_OF_BOUNDS=0;
        double val1=0.0,val2=0.0;
        vtParticleInfo* vertex = new vtParticleInfo();
	TIME_DIR dir;
	float curTime = m_fCurrentTime;
        int total_effective_samples=0, istat, good=0;
	vtParticleInfo* L0 = new vtParticleInfo();
        vtParticleInfo* L1 = new vtParticleInfo();
	double rndval;
	//vtListParticle m_lSeeds1;
	//cout<<"The plane is A="<<a<<" B="<<b<<" C="<<c<<" D="<<d<<endl;
        dir=BACKWARD;
	sIter= m_lSeeds1.begin();
        *L0 = *(*sIter);
        sIter= m_lSeeds1.end();
        --sIter;
        *L1 = *(*sIter);
	good=0;

	for( int i=0;i<randomPoints;i++)
	{
		//randf=1;
		//rndval=(double)rand();		
		randf=(double)rand()/(RAND_MAX);

		ptA[0]=newPoints[0][0]+ (newPoints[3][0]-newPoints[0][0])*randf;
		ptA[1]=newPoints[0][1]+ (newPoints[3][1]-newPoints[0][1])*randf;
		ptA[2]=newPoints[0][2]+ (newPoints[3][2]-newPoints[0][2])*randf;

		ptB[0]=newPoints[1][0]+ (newPoints[2][0]-newPoints[1][0])*randf;
		ptB[1]=newPoints[1][1]+ (newPoints[2][1]-newPoints[1][1])*randf;
		ptB[2]=newPoints[1][2]+ (newPoints[2][2]-newPoints[1][2])*randf;

		randf=(double)rand()/(RAND_MAX);

		ptF[0]=ptA[0]+ (ptB[0]-ptA[0])*randf;
		ptF[1]=ptA[1]+ (ptB[1]-ptA[1])*randf;
		ptF[2]=ptA[2]+ (ptB[2]-ptA[2])*randf;
		//cout<<"The pointF is evaluated to "<<a*ptF[0]+b*ptF[1]+c*ptF[2]-d<<endl;

		pointInfo.phyCoord=ptF;
		pointInfo.interpolant[0]=0;	// interpolation coefficients
		pointInfo.interpolant[1]=0;	// interpolation coefficients
		pointInfo.interpolant[2]=0;	// interpolation coefficients
		pointInfo.fromCell=-1;	// advecting result from which cell, mainly used for unstructured grid
		pointInfo.inCell=-1;
		res=m_pField->at_phys(-1, ptF, pointInfo, 0, nodeData);
		//cout<<"location is "<<pointInfo.phyCoord[0]<<" "<<pointInfo.phyCoord[1]<<" "<<pointInfo.phyCoord[2]<<endl;
		
		nodeData=normalized(nodeData);
		data[i]=nodeData;
		//cout<<"NodeData at this point is "<<data[i][0]<<" "<<data[i][1]<<" "<<data[i][2]<<endl;
		
		sumDots+=(nodeData[0]*a+nodeData[1]*b+nodeData[2]*c)*(nodeData[0]*a+nodeData[1]*b+nodeData[2]*c);
		//cout<<"The dotproduct is "<<sumDots<<endl;
		//check to see how close this point goes to the seeding curve
		vertex->m_pointInfo.phyCoord[0]=pointInfo.phyCoord[0];
                vertex->m_pointInfo.phyCoord[1]=pointInfo.phyCoord[1];
                vertex->m_pointInfo.phyCoord[2]=pointInfo.phyCoord[2];
		OUT_OF_BOUNDS=0;

#if 0
		//TESTING ADAPTIVE STEP DISTANCE BY INTEGRATING FORWARD AND THEN BACKWARD
		//FORWARD 100 STEPS
		dir=FORWARD;
		for(int i=0; i<100; i++)
		oneStepEmbedded(RK45, dir, STEADY, vertex->m_pointInfo, &curTime, &dt);
		//BACKWARD 100 STEPS
		dir=BACKWARD;
		for(int i=0; i<100; i++)
		oneStepEmbedded(RK45, dir, STEADY, vertex->m_pointInfo, &curTime, &dt);
#endif



		for(int i=0;i<min_steps;i++)
		{
//			istat = runge_kutta45(dir, STEADY, vertex->m_pointInfo, &curTime, dt,&integ_error);
			
			//istat = runge_kutta4(dir, STEADY, vertex->m_pointInfo, &curTime, dt);
			oneStepEmbedded(RK45, dir, STEADY, vertex->m_pointInfo, &curTime, &dt);
			if(istat==OUT_OF_BOUND)
                        {
                                cout<<"out of bound while tracing back__check for ERROR!!"<<endl;
                                OUT_OF_BOUNDS++;
                        }
		}

		int intg_steps=10;
		curve_min=999999.999;
		for(int i=0;i<intg_steps*2;i++)
		{
		VECTOR3 x0,x2,x1,x1_x0,x2_x1,x0_x2,x0_x1;
			
		istat = runge_kutta4(dir, STEADY, vertex->m_pointInfo, &curTime, dt/intg_steps);
		if(istat==OUT_OF_BOUND)
                {
                      cout<<"out of bound while tracing back__check for ERROR!!"<<endl;
                      OUT_OF_BOUNDS++;
                }
		if(OUT_OF_BOUNDS>0)
		continue;

                x1=L0->m_pointInfo.phyCoord;
                x2=L1->m_pointInfo.phyCoord;
                x0=vertex->m_pointInfo.phyCoord;

                x2_x1[0]=x2[0]-x1[0];
                x2_x1[1]=x2[1]-x1[1];
                x2_x1[2]=x2[2]-x1[2];

                x1_x0[0]=x1[0]-x0[0];
                x1_x0[1]=x1[1]-x0[1];
                x1_x0[2]=x1[2]-x0[2];

                x0_x2[0]=x0[0]-x2[0];
                x0_x2[1]=x0[1]-x2[1];
                x0_x2[2]=x0[2]-x2[2];

                x0_x1[0]=-x1_x0[0];
                x0_x1[1]=-x1_x0[1];
                x0_x1[2]=-x1_x0[2];
               // for the L0L1 piece of the curve, we calculate min dis
                t_min=-dotproduct(x1_x0,x2_x1)/dotproduct(x2_x1,x2_x1);
                if(t_min>=0.0 && t_min<=1.0)
                    min_distance=vectorvalue(crossproduct_new(x0_x1,x0_x2))/vectorvalue(x2_x1);
                else
                {
                    if(vectorvalue(x0_x1)<vectorvalue(x0_x2))
                       min_distance=vectorvalue(x0_x1);
                    else min_distance=vectorvalue(x0_x2);
                }
		if(min_distance<curve_min)
                  curve_min=min_distance;
		}
		if(curve_min<0.5)
		  good++;
		float lgval;
		//lgval=log10(curve_min*area);
		lgval=log10(curve_min);
		if(lgval>2.990)
		lgval=2.990;
		if(lgval<-7.0)
		lgval=-7.0;
		hist2[(int)(floor(BIN_HIST*(lgval+7)*1.0/10.0))]+=area;
	}
	if(isnan(area))
         area=0.0;

	sum_area+=area;
	return (1.0 - ((float)good)/randomPoints);
//	return sqrt(sumDots/randomPoints);
}

vtCStreamSurface::vtCStreamSurface(CVectorField* pField):vtCFieldLine(pField)
{}
vtCStreamSurface::~vtCStreamSurface(void)
{}

void vtCStreamSurface::setRange(float val)
{
	data_range=val;
}

void vtCStreamSurface::setForwardTracing(int enabled)
{
         m_itsTraceDir=FORWARD_DIR; 
}

void vtCStreamSurface::setBackwardTracing(int enabled)
{
         m_itsTraceDir=BACKWARD_DIR; 
}

int vtCStreamSurface::getForwardTracing(void)
{
        if( m_itsTraceDir==FORWARD_DIR || m_itsTraceDir==BACKWARD_AND_FORWARD )
                return 1;
        else
                return 0;
}

int vtCStreamSurface::getBackwardTracing(void)
{
        if( m_itsTraceDir==BACKWARD_DIR || m_itsTraceDir==BACKWARD_AND_FORWARD )
                return 1;
        else
                return 0;
}


void vtCStreamSurface::execute(const void* userData,
                            list<VECTOR3>& listSeedTraces,
                                        list<int64_t> *listSeedIds)
{
        m_fCurrentTime = *(float *)userData;
	
	//Timer timer1;

	//timer1.start();
	if(QUAD)
        computeStreamSurfaceQuad(userData, listSeedTraces, listSeedIds);
	else computeStreamSurface(userData, listSeedTraces, listSeedIds);
	 //timer1.end();
         //cout<<"The time taken is "<<timer1.getElapsedMS()<<"MS"<<endl;
}
void vtCStreamSurface::computeStreamSurface(const void* userdata, 
                        list<VECTOR3>& listSeedTraces,
                                list<int64_t> *listSeedIds)
{
	vtListParticleIter sIter;
	vtListParticle m_lSeeds1;
        list<int64_t>::iterator sIdIter;
	list<VECTOR3>::iterator sIdIter1;
	float FACTOR=255.0/data_range;
	
	VECTOR3 points[4],normals[4];

        float hist[100]={0};
        int bins=100;
	//Timer timer1;
	
	int istat,i,res;
	int triangles=0;
	VECTOR3 nodeData,point,vec1,vec2;
	VECTOR3 color;
	float inner_angle;
	float dt=1.0;
	float curTime = m_fCurrentTime;
	float red,green,blue,value;
	ofstream file_op("mesh.off");
	//file_op<<"OFF\n"<<"3000 1000 3000\n";	
	file_op<<"#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n";	
	//timer1.start();
	vtParticleInfo* L0 = new vtParticleInfo();
	vtParticleInfo* L1 = new vtParticleInfo();
	vtParticleInfo* R0 =new vtParticleInfo();
	vtParticleInfo* R1 = new vtParticleInfo();
	vtParticleInfo* R2 = new vtParticleInfo();
	vtParticleInfo* R3 = new vtParticleInfo();
	vtParticleInfo* newSeed,*newSeed1;// = new vtParticleInfo();
	if( !m_lSeedIds.empty() )
                        sIdIter = m_lSeedIds.begin();

	for(sIter = m_lSeeds.begin(); sIter != m_lSeeds.end(); ++sIter)
        {
        vtParticleInfo* L12 = new vtParticleInfo();
                *L12 = (*sIter);
                m_lSeeds1.push_back(L12);
        }

	cout<<"The size of the list is "<<m_lSeeds.size()<<endl;
	sIter = m_lSeeds.begin();
	++sIter;
	for(; (++sIter) != m_lSeeds.end(); ++sIter)
	{
		--sIter;
		vtParticleInfo* prt1 = new vtParticleInfo();
		*prt1=*(*sIter);
		m_lSeeds.insert(sIter,prt1);
	}	
	cout<<"The size of the list is "<<m_lSeeds.size()<<endl;

	for(i=0;i<m_nMaxsize*2;)
      {
	for(sIter = m_lSeeds.begin(); sIter != m_lSeeds.end(); ++sIter)	
	{
		vtParticleInfo* thisSeed = *sIter;
		 *L0 = *(*sIter);
		 *L1 = *(*sIter);
		 *R0 =*(*(++sIter));
		 *R1 = *(*sIter);
		if(++sIter!=m_lSeeds.end()) 
		{
			*R2 =**(sIter);
			*R3 =**(sIter);
		}
		sIter--;
		
		//istat = runge_kutta4(FORWARD, STEADY, R3->m_pointInfo, &curTime, dt);
		oneStepEmbedded(RK45, FORWARD, STEADY, R3->m_pointInfo, &curTime, &dt);
		R3->itsNumStepsAlive++;
		float left_dg,right_dg,m1,m2;
//	if(++sIter==m_lSeeds.end() || equal(R0,R2)||equal(R0,R3))
	if(1)
	{
	//	sIter--;
		sIter--;
		
		//istat = runge_kutta4(FORWARD, STEADY, L1->m_pointInfo, &curTime, dt);
		oneStepEmbedded(RK45, FORWARD, STEADY, L1->m_pointInfo, &curTime, &dt);
		L1->itsNumStepsAlive++;
		if(istat==OUT_OF_BOUND)
		{
			cout<<"out of bound"<<endl;
		}
		//istat = runge_kutta4(FORWARD, STEADY, R1->m_pointInfo, &curTime, dt);
		oneStepEmbedded(RK45, FORWARD, STEADY, R1->m_pointInfo, &curTime, &dt);
		R1->itsNumStepsAlive++;
		if(istat==OUT_OF_BOUND)
		{
			cout<<"out of bound"<<endl;
		}
		left_dg=length(L1->m_pointInfo,R0->m_pointInfo);
		right_dg=length(L0->m_pointInfo,R1->m_pointInfo);
	//if((!GARTH) && (length(L1->m_pointInfo,R1->m_pointInfo)>2*length(L0->m_pointInfo,L1->m_pointInfo))) //splitting
	if((length(L1->m_pointInfo,R1->m_pointInfo)>2.0*length(L0->m_pointInfo,L1->m_pointInfo))) //splitting
	{
		//cout<<"In hult..should not come when u r using Garth"<<endl;
		//vtParticleInfo* newSeed;
		newSeed= new vtParticleInfo();
		newSeed1= new vtParticleInfo();
		newSeed->m_pointInfo.phyCoord[0]=(L1->m_pointInfo.phyCoord[0]+R1->m_pointInfo.phyCoord[0])/2;
		newSeed->m_pointInfo.phyCoord[1]=(L1->m_pointInfo.phyCoord[1]+R1->m_pointInfo.phyCoord[1])/2;
		newSeed->m_pointInfo.phyCoord[2]=(L1->m_pointInfo.phyCoord[2]+R1->m_pointInfo.phyCoord[2])/2;
		point=newSeed->m_pointInfo.phyCoord;
		newSeed->itsNumStepsAlive=L1->itsNumStepsAlive;
		newSeed->m_fStartTime=L1->m_fStartTime;
		newSeed->ptId=-5;
		res=m_pField->at_phys(-1, point, newSeed->m_pointInfo, newSeed->m_fStartTime, nodeData);
		newSeed->itsValidFlag = 1;
		*(*sIter)=*L1;
		sIter++;
		*(*sIter)=*R1;
		*newSeed1=*newSeed;
		m_lSeeds.insert(sIter,newSeed);
		m_lSeeds.insert(sIter,newSeed1);
		
		
		points[0]=L0->m_pointInfo.phyCoord;
                points[1]=R0->m_pointInfo.phyCoord;
                points[2]=newSeed->m_pointInfo.phyCoord;
                points[3]=newSeed->m_pointInfo.phyCoord;

		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisTriangle(points,m_pField);
		else value=0;

                //hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
                rgbmap(fabs(value)*255.0,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
		listSeedTraces.push_back(L0->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(R0->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(newSeed->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(newSeed->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);

		//res=m_pField->at_phys(L0->m_pointInfo.fromCell,L0->m_pointInfo.phyCoord, L0->m_pointInfo, L0->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
		file_op<<L0->m_pointInfo.phyCoord[0]<<" "<<L0->m_pointInfo.phyCoord[1]<<" "<<L0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(R0->m_pointInfo.fromCell,R0->m_pointInfo.phyCoord, R0->m_pointInfo, R0->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
		file_op<<R0->m_pointInfo.phyCoord[0]<<" "<<R0->m_pointInfo.phyCoord[1]<<" "<<R0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(newSeed->m_pointInfo.fromCell,newSeed->m_pointInfo.phyCoord, newSeed->m_pointInfo, newSeed->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
		file_op<<newSeed->m_pointInfo.phyCoord[0]<<" "<<newSeed->m_pointInfo.phyCoord[1]<<" "<<newSeed->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";


		points[0]=L0->m_pointInfo.phyCoord;
                points[1]=newSeed->m_pointInfo.phyCoord;
                points[2]=L1->m_pointInfo.phyCoord;
                points[3]=L1->m_pointInfo.phyCoord;

		//value=errorAnalysisGlobal(points,m_lSeeds1,dt,L0->itsNumStepsAlive);
		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisTriangle(points,m_pField);
		else value=0;
                //hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
                rgbmap(fabs(value)*255.0,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
		listSeedTraces.push_back(L0->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(newSeed->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);

		//res=m_pField->at_phys(L0->m_pointInfo.fromCell,L0->m_pointInfo.phyCoord, L0->m_pointInfo, L0->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
		file_op<<L0->m_pointInfo.phyCoord[0]<<" "<<L0->m_pointInfo.phyCoord[1]<<" "<<L0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(newSeed->m_pointInfo.fromCell,newSeed->m_pointInfo.phyCoord, newSeed->m_pointInfo, newSeed->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
		file_op<<newSeed->m_pointInfo.phyCoord[0]<<" "<<newSeed->m_pointInfo.phyCoord[1]<<" "<<newSeed->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(L1->m_pointInfo.fromCell,L1->m_pointInfo.phyCoord, L1->m_pointInfo, L1->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
		file_op<<L1->m_pointInfo.phyCoord[0]<<" "<<L1->m_pointInfo.phyCoord[1]<<" "<<L1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		

		points[0]=R0->m_pointInfo.phyCoord;
                points[1]=R1->m_pointInfo.phyCoord;
                points[2]=newSeed->m_pointInfo.phyCoord;
                points[3]=newSeed->m_pointInfo.phyCoord;

		//value=errorAnalysisGlobal(points,m_lSeeds1,dt,R0->itsNumStepsAlive);
		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisTriangle(points,m_pField);
		else value=0;
                //hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
                rgbmap(fabs(value)*255.0,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
		listSeedTraces.push_back(L0->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(R0->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);

		//res=m_pField->at_phys(R0->m_pointInfo.fromCell,R0->m_pointInfo.phyCoord, R0->m_pointInfo, R0->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
		file_op<<R0->m_pointInfo.phyCoord[0]<<" "<<R0->m_pointInfo.phyCoord[1]<<" "<<R0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(R1->m_pointInfo.fromCell,R1->m_pointInfo.phyCoord, R1->m_pointInfo, L0->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
		file_op<<R1->m_pointInfo.phyCoord[0]<<" "<<R1->m_pointInfo.phyCoord[1]<<" "<<R1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(newSeed->m_pointInfo.fromCell,newSeed->m_pointInfo.phyCoord, newSeed->m_pointInfo, newSeed->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
		file_op<<newSeed->m_pointInfo.phyCoord[0]<<" "<<newSeed->m_pointInfo.phyCoord[1]<<" "<<newSeed->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
	//	sIter--;
		triangles+=3;
	}
	else if(0) // merging
	{
		
	}
	else{
		if(right_dg>left_dg)
		{
			//cout<<"Triangulate the points L0 R0 L1"<<endl;
		points[0]=L0->m_pointInfo.phyCoord;
                points[1]=R0->m_pointInfo.phyCoord;
                points[2]=L1->m_pointInfo.phyCoord;
                points[3]=L1->m_pointInfo.phyCoord;

		//value=errorAnalysisGlobal(points,m_lSeeds1,dt,L0->itsNumStepsAlive);
		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisTriangle(points,m_pField);
		else value=0;
                //hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
                rgbmap(fabs(value)*255.0,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
		listSeedTraces.push_back(L0->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(R0->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);

		//res=m_pField->at_phys(L0->m_pointInfo.fromCell,L0->m_pointInfo.phyCoord, L0->m_pointInfo, L0->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
			file_op<<L0->m_pointInfo.phyCoord[0]<<" "<<L0->m_pointInfo.phyCoord[1]<<" "<<L0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(R0->m_pointInfo.fromCell,R0->m_pointInfo.phyCoord, R0->m_pointInfo, R0->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
			file_op<<R0->m_pointInfo.phyCoord[0]<<" "<<R0->m_pointInfo.phyCoord[1]<<" "<<R0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(L1->m_pointInfo.fromCell,L1->m_pointInfo.phyCoord, L1->m_pointInfo, L1->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
			file_op<<L1->m_pointInfo.phyCoord[0]<<" "<<L1->m_pointInfo.phyCoord[1]<<" "<<L1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

			*(*sIter)=*L1;
			sIter++;
		}
		else
		{
			
		//	cout<<"Triangulate the points L0 R0 R1"<<endl;
		points[0]=L0->m_pointInfo.phyCoord;
                points[1]=R0->m_pointInfo.phyCoord;
                points[2]=R1->m_pointInfo.phyCoord;
                points[3]=R1->m_pointInfo.phyCoord;

		//value=errorAnalysisGlobal(points,m_lSeeds1,dt,R0->itsNumStepsAlive);
		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisTriangle(points,m_pField);
		else value=0;
                //hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
                rgbmap(fabs(value)*255.0,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
		listSeedTraces.push_back(L0->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(R0->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
                listSeedTraces.push_back(color);
		//res=m_pField->at_phys(L0->m_pointInfo.fromCell,L0->m_pointInfo.phyCoord, L0->m_pointInfo, L0->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
			file_op<<L0->m_pointInfo.phyCoord[0]<<" "<<L0->m_pointInfo.phyCoord[1]<<" "<<L0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(R0->m_pointInfo.fromCell,R0->m_pointInfo.phyCoord, R0->m_pointInfo, R0->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
			file_op<<R0->m_pointInfo.phyCoord[0]<<" "<<R0->m_pointInfo.phyCoord[1]<<" "<<R0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";

		//res=m_pField->at_phys(R1->m_pointInfo.fromCell,R1->m_pointInfo.phyCoord, R1->m_pointInfo, R1->m_fStartTime, nodeData);
		//value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		//rgbmap(value,red,green,blue);
			file_op<<R1->m_pointInfo.phyCoord[0]<<" "<<R1->m_pointInfo.phyCoord[1]<<" "<<R1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
			*(*(++sIter))=*R1;
		}
		triangles++;
	     }
	  
	    }//end if for valid
	else
	{
		sIter--;
	}//end else for valid
	}
	if(GARTH)
	{
	for(sIter = m_lSeeds.begin(); sIter != m_lSeeds.end(); ++sIter)	
	{
	//if the front is propagated, then check for divergence
		vtParticleInfo* s1= new vtParticleInfo();
		vtParticleInfo* s2= new vtParticleInfo();
		vtParticleInfo* s3= new vtParticleInfo();
		vtParticleInfo* s4= new vtParticleInfo();
		double d=0.0;

		*s1 = *(*sIter);
		if(sIter == m_lSeeds.end())
		break;	
		*s2 =*(*(++sIter));
		if(sIter == m_lSeeds.end())
		break;	
		*s3 =*(*(++sIter));
		if(sIter == m_lSeeds.end())
		break;	
		*s4 =*(*(++sIter));
		if(sIter == m_lSeeds.end())
		break;	
		
		
	if((s1->itsNumStepsAlive==s2->itsNumStepsAlive) && (s2->itsNumStepsAlive==s3->itsNumStepsAlive) && (s3->itsNumStepsAlive==s4->itsNumStepsAlive))
	{
	//calculate the adjacent angle
	vec1[0]=s1->m_pointInfo.phyCoord[0]-s2->m_pointInfo.phyCoord[0];
	vec1[1]=s1->m_pointInfo.phyCoord[1]-s2->m_pointInfo.phyCoord[1];
	vec1[2]=s1->m_pointInfo.phyCoord[2]-s2->m_pointInfo.phyCoord[2];

	vec2[0]=s4->m_pointInfo.phyCoord[0]-s3->m_pointInfo.phyCoord[0];
	vec2[1]=s4->m_pointInfo.phyCoord[1]-s3->m_pointInfo.phyCoord[1];
	vec2[2]=s4->m_pointInfo.phyCoord[2]-s3->m_pointInfo.phyCoord[2];
	inner_angle=dotproduct(vec1,vec2)/(vectorvalue(vec1)*vectorvalue(vec2));
	if(inner_angle*inner_angle>1)
	cout<<"Inner angle went wrong"<<endl;
	inner_angle = acos(inner_angle)* 180.0 / PI;	

	d = sqrt(vec1[0]*vec1[0]+vec1[1]*vec1[1]+vec1[2]*vec1[2]);
	d+= sqrt(vec2[0]*vec2[0]+vec2[1]*vec2[1]+vec2[2]*vec2[2]);

	//if(length(L1->m_pointInfo,R1->m_pointInfo)>2*length(L0->m_pointInfo,L1->m_pointInfo)) //splitting
	if(180.0 - (inner_angle) > MAX_ANGLE) //splitting
	{
		//cout<<"New seed insertion"<<endl;
		//vtParticleInfo* newSeed;
		newSeed= new vtParticleInfo();
		newSeed1= new vtParticleInfo();
		newSeed->m_pointInfo.phyCoord[0]=(s1->m_pointInfo.phyCoord[0]+s2->m_pointInfo.phyCoord[0])/2;
		newSeed->m_pointInfo.phyCoord[1]=(s1->m_pointInfo.phyCoord[1]+s2->m_pointInfo.phyCoord[1])/2;
		newSeed->m_pointInfo.phyCoord[2]=(s1->m_pointInfo.phyCoord[2]+s2->m_pointInfo.phyCoord[2])/2;
		point=newSeed->m_pointInfo.phyCoord;
		newSeed->m_fStartTime=s1->m_fStartTime;
		newSeed->ptId=-5;
		newSeed->itsNumStepsAlive=s1->itsNumStepsAlive;
		res=m_pField->at_phys(-1, point, newSeed->m_pointInfo, newSeed->m_fStartTime, nodeData);
		newSeed->itsValidFlag = 1;
		sIter--;
		sIter--;
		//*(*sIter)=*R1;
		*newSeed1=*newSeed;
		m_lSeeds.insert(sIter,newSeed);
		m_lSeeds.insert(sIter,newSeed1);
		
		sIter++;
		sIter++;

		newSeed= new vtParticleInfo();
                newSeed1= new vtParticleInfo();
                newSeed->m_pointInfo.phyCoord[0]=(s3->m_pointInfo.phyCoord[0]+s4->m_pointInfo.phyCoord[0])/2;
                newSeed->m_pointInfo.phyCoord[1]=(s3->m_pointInfo.phyCoord[1]+s4->m_pointInfo.phyCoord[1])/2;
                newSeed->m_pointInfo.phyCoord[2]=(s3->m_pointInfo.phyCoord[2]+s4->m_pointInfo.phyCoord[2])/2;
                point=newSeed->m_pointInfo.phyCoord;
                newSeed->m_fStartTime=s1->m_fStartTime;
                newSeed->ptId=-5;
		newSeed->itsNumStepsAlive=s1->itsNumStepsAlive;
                res=m_pField->at_phys(-1, point, newSeed->m_pointInfo, newSeed->m_fStartTime, nodeData);
                newSeed->itsValidFlag = 1;
                //*(*sIter)=*R1;
                *newSeed1=*newSeed;
                m_lSeeds.insert(sIter,newSeed);
                m_lSeeds.insert(sIter,newSeed1);
                //sIter--;
                //sIter--;
	
	}
	else if((180.0 - (inner_angle) < MIN_ANGLE) && (d < MIN_DIS)) //splitting
	{
		sIter--;
		sIter--;
		sIter=m_lSeeds.erase(sIter);	
		sIter=m_lSeeds.erase(sIter);	
	}
	else
	{
		sIter--;
		sIter--;
	}

	}
	else
	{
		sIter--;
		sIter--;
	}
	}
	}//endif for garth algo
	i++;
    }
	 //timer1.end();
         //cout<<"The time taken for st-surface calculation is "<<timer1.getElapsedMS()<<"MS"<<endl;
	cout<<"Size of the list "<<m_lSeeds.size()<<endl;
	//delete temporary seeds
/*	for(sIter = m_lSeeds.begin(); sIter != m_lSeeds.end();)
	{
		if((*sIter)->ptId==-5)
		m_lSeeds.erase(sIter);
		else ++sIter;
	}*/
	cout<<"Size of the list after deletion "<<m_lSeeds.size()<<endl;
	for(int j=0;j<triangles*3;)
	{
		file_op<<"3 "<<j<<" "<<j+1<<" "<<j+2<<"\n";
		j+=3;
	}
	file_op.seekp(ios_base::beg);
	file_op<<"COFF\n"<<triangles*3<<" "<<triangles<<" "<<triangles*3<<"\n";
	file_op.close();
	cout<<"Total triangles generated "<<triangles<<endl;

	//write to a file
        ofstream file_write("streamsur3_triangles.out",ios::out | ios::binary);
        int numPoints=listSeedTraces.size()/2;
        file_write.write((char *)&numPoints,sizeof(int));
        for(sIdIter1 = listSeedTraces.begin();sIdIter1 != listSeedTraces.end();sIdIter1++ )
        {
                VECTOR3 point;
                point=*sIdIter1;
                file_write.write((char *)&point[0],sizeof(float));
                file_write.write((char *)&point[1],sizeof(float));
                file_write.write((char *)&point[2],sizeof(float));
                sIdIter1++;
        }

        file_write.close();
	cout<<"sum of area is "<<sum_area<<endl;
	/*for(int i=0;i<bins;i++)
        {
                cout<<hist[i]/sum_area<<endl;
        }*/
	/*cout<<"printing second histogram"<<endl;
	for(int i=0;i<BIN_HIST;i++)
        {
                cout<<hist2[i]/sum_area<<endl;
        }*/	
	
}

void vtCStreamSurface::computeStreamSurfaceQuad(const void* userdata, 
                        list<VECTOR3>& listSeedTraces,
                                list<int64_t> *listSeedIds)
{
	vtListParticleIter sIter,sIter1;
        vtListParticle m_lSeeds1;
	list<int64_t>::iterator sIdIter;
	list<VECTOR3>::iterator sIdIter1;
	float FACTOR=255.0/data_range;
	VECTOR3 points[4],normals[4];
	
	float hist[100]={0.0};
	int bins=100;
	//Timer timer1;
	//timer1.start();

	int istat,i,res;
	int quads=0;
	VECTOR3 nodeData,point;
	VECTOR3 vec1,vec2,vec3,vec4;
	VECTOR3 color;
	TIME_DIR dir;//=BACKWARD;
	float old_dis=65365,new_dis=0,new_alpha,new_beta,old_alpha;
	float curTime = m_fCurrentTime,dt=1.0;
	float red,green,blue,value;
	int count_OOB=0;
	ofstream file_op("quad.off");
	ofstream file_op1("quad.obj");
	//file_op<<"OFF\n"<<"3000 1000 3000\n";	
	file_op<<"#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n#\n";	
	vtParticleInfo* L0 = new vtParticleInfo();
	vtParticleInfo* L1 = new vtParticleInfo();
	vtParticleInfo* R0 =new vtParticleInfo();
	vtParticleInfo* R1 = new vtParticleInfo();
	vtParticleInfo* R2 = new vtParticleInfo();
	vtParticleInfo* newSeed,*newSeed1;// = new vtParticleInfo();
	if( !m_lSeedIds.empty() )
                        sIdIter = m_lSeedIds.begin();
	for(sIter = m_lSeeds.begin(); sIter != m_lSeeds.end(); ++sIter)
        {
	vtParticleInfo* L12 = new vtParticleInfo();
		*L12 = (*sIter);
		m_lSeeds1.push_back(L12);	
	}
	if(m_itsTraceDir== BACKWARD_DIR)
	dir=BACKWARD;
	else if(m_itsTraceDir== FORWARD_DIR)
	dir=FORWARD;
	cout<<"Trace direction is "<<dir<<endl;

	dt=1.0; //set this to maximum step size and let the adaptive rk4 decide what is best

	for(i=0;i<m_nMaxsize;)
      {
		sIter = m_lSeeds.begin();	
		 *L0 = *(*sIter);
		 *L1 = *(*sIter);
		istat = runge_kutta4(dir, STEADY, L1->m_pointInfo, &curTime, dt);
		//oneStepEmbedded(RK45, dir, STEADY, L1->m_pointInfo, &curTime, &dt);

		L1->itsNumStepsAlive++;
		count_OOB=0;
		if(istat==OUT_OF_BOUND)
		{
			cout<<"out of bound"<<endl;
			count_OOB++;
		}
		**sIter=*L1;
		++sIter;
	for(; sIter != m_lSeeds.end(); ++sIter)	
	{
		vtParticleInfo* thisSeed = *sIter;
		 *R0 =*(*sIter);
		 *R1 = *(*sIter);
		sIter1=sIter;
		
		float left_dg,right_dg,m1,m2;
	
		istat = runge_kutta4(dir, STEADY, R1->m_pointInfo, &curTime,dt);
		//oneStepEmbedded(RK45, dir, STEADY, R1->m_pointInfo, &curTime, &dt);
		R1->itsNumStepsAlive++;
		if(istat==OUT_OF_BOUND)
		{
			cout<<"out of bound"<<endl;
			count_OOB++;
		}
		**sIter=*R1;
	  new_dis=length(L0->m_pointInfo,R0->m_pointInfo);
	   vec1[0]=L1->m_pointInfo.phyCoord[0]-L0->m_pointInfo.phyCoord[0];
	   vec1[1]=L1->m_pointInfo.phyCoord[1]-L0->m_pointInfo.phyCoord[1];
	   vec1[2]=L1->m_pointInfo.phyCoord[2]-L0->m_pointInfo.phyCoord[2];

	   vec2[0]=R0->m_pointInfo.phyCoord[0]-L0->m_pointInfo.phyCoord[0];
	   vec2[1]=R0->m_pointInfo.phyCoord[1]-L0->m_pointInfo.phyCoord[1];
	   vec2[2]=R0->m_pointInfo.phyCoord[2]-L0->m_pointInfo.phyCoord[2];

	   vec3[0]=-vec2[0];
	   vec3[1]=-vec2[1];
	   vec3[2]=-vec2[2];

	   vec4[0]=R1->m_pointInfo.phyCoord[0]-R0->m_pointInfo.phyCoord[0];
	   vec4[1]=R1->m_pointInfo.phyCoord[1]-R0->m_pointInfo.phyCoord[1];
	   vec4[2]=R1->m_pointInfo.phyCoord[2]-R0->m_pointInfo.phyCoord[2];
	   new_alpha=dotproduct(vec1,vec2);
	   new_beta=dotproduct(vec3,vec4);
	if(new_dis>1) //splitting  TO-DO: Fix this hard coded threshold value
	{
	   if(new_alpha<ANGLE_THRESHOLD && new_beta<ANGLE_THRESHOLD)
	   {		
		newSeed= new vtParticleInfo();
		newSeed1= new vtParticleInfo();
		newSeed->m_pointInfo.phyCoord[0]=(L0->m_pointInfo.phyCoord[0]+R0->m_pointInfo.phyCoord[0])/2;
		newSeed->m_pointInfo.phyCoord[1]=(L0->m_pointInfo.phyCoord[1]+R0->m_pointInfo.phyCoord[1])/2;
		newSeed->m_pointInfo.phyCoord[2]=(L0->m_pointInfo.phyCoord[2]+R0->m_pointInfo.phyCoord[2])/2;
		point=newSeed->m_pointInfo.phyCoord;
		newSeed->m_fStartTime=L1->m_fStartTime;
		newSeed->ptId=-5;
		*newSeed1=*newSeed;
		res=m_pField->at_phys(-1, point, newSeed->m_pointInfo, newSeed->m_fStartTime, nodeData);
		newSeed->itsValidFlag = 1;
		*(*sIter)=*L1;
		istat = runge_kutta4(dir, STEADY, newSeed->m_pointInfo, &curTime,dt);
		//oneStepEmbedded(RK45, dir, STEADY, newSeed->m_pointInfo, &curTime, &dt);
		newSeed->itsNumStepsAlive = L1->itsNumStepsAlive;
		if(istat==OUT_OF_BOUND)
		{
			cout<<"out of bound"<<endl;
			count_OOB++;
		}
		*(*sIter)=*R1;
		m_lSeeds.insert(sIter,newSeed);
		
	        new_dis=length(newSeed1->m_pointInfo,R0->m_pointInfo);
		
		points[0]=L0->m_pointInfo.phyCoord;
		points[1]=newSeed1->m_pointInfo.phyCoord;
		points[2]=newSeed->m_pointInfo.phyCoord;
		points[3]=L1->m_pointInfo.phyCoord;
		calculateNormal(points,normals);
		
		
	/*	res=m_pField->at_phys(L0->m_pointInfo.fromCell,L0->m_pointInfo.phyCoord, L0->m_pointInfo, L0->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[0]);*/

		//value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);
		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);//L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisQuad(points,m_pField);
		else value=0;
		//hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
		rgbmap(fabs(value)*255.0,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
		file_op<<L0->m_pointInfo.phyCoord[0]<<" "<<L0->m_pointInfo.phyCoord[1]<<" "<<L0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(L0->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

	/*	res=m_pField->at_phys(newSeed1->m_pointInfo.fromCell,newSeed1->m_pointInfo.phyCoord, newSeed1->m_pointInfo, newSeed1->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[1]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
		file_op<<newSeed1->m_pointInfo.phyCoord[0]<<" "<<newSeed1->m_pointInfo.phyCoord[1]<<" "<<newSeed1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(newSeed1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

	/*	res=m_pField->at_phys(newSeed->m_pointInfo.fromCell,newSeed->m_pointInfo.phyCoord, newSeed->m_pointInfo, newSeed->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[2]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
		file_op<<newSeed->m_pointInfo.phyCoord[0]<<" "<<newSeed->m_pointInfo.phyCoord[1]<<" "<<newSeed->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(newSeed->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);



	/*	res=m_pField->at_phys(L1->m_pointInfo.fromCell,L1->m_pointInfo.phyCoord, L1->m_pointInfo, L1->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[3]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
		file_op<<L1->m_pointInfo.phyCoord[0]<<" "<<L1->m_pointInfo.phyCoord[1]<<" "<<L1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);
		
		points[0]=newSeed1->m_pointInfo.phyCoord;
		points[1]=R0->m_pointInfo.phyCoord;
		points[2]=R1->m_pointInfo.phyCoord;
		points[3]=newSeed->m_pointInfo.phyCoord;
		calculateNormal(points,normals);
		
	/*	res=m_pField->at_phys(newSeed1->m_pointInfo.fromCell,newSeed1->m_pointInfo.phyCoord, newSeed1->m_pointInfo, newSeed1->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[0]);*/
		
		//value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);
		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);//L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisQuad(points,m_pField);
		else value=0;
		//hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
		file_op<<newSeed1->m_pointInfo.phyCoord[0]<<" "<<newSeed1->m_pointInfo.phyCoord[1]<<" "<<newSeed1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(newSeed1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

	/*	res=m_pField->at_phys(R0->m_pointInfo.fromCell,R0->m_pointInfo.phyCoord, R0->m_pointInfo, R0->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[1]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
		file_op<<R0->m_pointInfo.phyCoord[0]<<" "<<R0->m_pointInfo.phyCoord[1]<<" "<<R0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(R0->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

	/*	res=m_pField->at_phys(R1->m_pointInfo.fromCell,R1->m_pointInfo.phyCoord, R1->m_pointInfo, L0->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[2]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
		file_op<<R1->m_pointInfo.phyCoord[0]<<" "<<R1->m_pointInfo.phyCoord[1]<<" "<<R1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(R1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

	/*	res=m_pField->at_phys(newSeed->m_pointInfo.fromCell,newSeed->m_pointInfo.phyCoord, newSeed->m_pointInfo, newSeed->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[3]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
		file_op<<newSeed->m_pointInfo.phyCoord[0]<<" "<<newSeed->m_pointInfo.phyCoord[1]<<" "<<newSeed->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(newSeed->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);
		

	//	sIter--;
		quads+=2;
	   }
	  else
	  {
		points[0]=L0->m_pointInfo.phyCoord;
		points[1]=R0->m_pointInfo.phyCoord;
		points[2]=R1->m_pointInfo.phyCoord;
		points[3]=L1->m_pointInfo.phyCoord;
		calculateNormal(points,normals);
				
			//cout<<"Quad mesh with the points L0 R0 R1 L1"<<endl;
	/*	res=m_pField->at_phys(L0->m_pointInfo.fromCell,L0->m_pointInfo.phyCoord, L0->m_pointInfo, L0->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[0]);*/
		
		//value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);
		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);//L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisQuad(points,m_pField);
		else value=0;
		//hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
			file_op<<L0->m_pointInfo.phyCoord[0]<<" "<<L0->m_pointInfo.phyCoord[1]<<" "<<L0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(L0->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

	/*	res=m_pField->at_phys(R0->m_pointInfo.fromCell,R0->m_pointInfo.phyCoord, R0->m_pointInfo, R0->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[1]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
			file_op<<R0->m_pointInfo.phyCoord[0]<<" "<<R0->m_pointInfo.phyCoord[1]<<" "<<R0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(R0->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);
		
				
	/*	res=m_pField->at_phys(R1->m_pointInfo.fromCell,R1->m_pointInfo.phyCoord, R1->m_pointInfo, R1->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[2]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
			file_op<<R1->m_pointInfo.phyCoord[0]<<" "<<R1->m_pointInfo.phyCoord[1]<<" "<<R1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(R1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

	/*	res=m_pField->at_phys(L1->m_pointInfo.fromCell,L1->m_pointInfo.phyCoord, L1->m_pointInfo, L1->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[3]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
			file_op<<L1->m_pointInfo.phyCoord[0]<<" "<<L1->m_pointInfo.phyCoord[1]<<" "<<L1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);
		
		quads++;
	  }
	}
	else if(--sIter1!=m_lSeeds.begin() && (old_dis+new_dis<2) && (old_alpha>0) && (new_beta>0)) // merging
	{
//		if((old_dis+new_dis<2) && (old_alpha>0) && (new_beta>0))
//		{
			sIter--;
			sIter=m_lSeeds.erase(sIter);
			new_dis=old_dis+new_dis;
		
		points[0]=L0->m_pointInfo.phyCoord;
		points[1]=R0->m_pointInfo.phyCoord;
		points[2]=R1->m_pointInfo.phyCoord;
		points[3]=L1->m_pointInfo.phyCoord;
		calculateNormal(points,normals);
			
			//cout<<"Quad mesh with the points L0 R0 R1 L1"<<endl;
      /*          res=m_pField->at_phys(L0->m_pointInfo.fromCell,L0->m_pointInfo.phyCoord, L0->m_pointInfo, L0->m_fStartTime, nodeData);
                value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[0]);*/
		
		//value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);
		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);//L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisQuad(points,m_pField);
		else value=0;
		//hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
		rgbmap(fabs(value)*255.0,red,green,blue);
                //rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
                        file_op<<L0->m_pointInfo.phyCoord[0]<<" "<<L0->m_pointInfo.phyCoord[1]<<" "<<L0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(L0->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

        /*        res=m_pField->at_phys(R0->m_pointInfo.fromCell,R0->m_pointInfo.phyCoord, R0->m_pointInfo, R0->m_fStartTime, nodeData);
                value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[1]);
		rgbmap(fabs(value)*255.0,red,green,blue);
                //rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
                        file_op<<R0->m_pointInfo.phyCoord[0]<<" "<<R0->m_pointInfo.phyCoord[1]<<" "<<R0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(R0->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);


         /*       res=m_pField->at_phys(R1->m_pointInfo.fromCell,R1->m_pointInfo.phyCoord, R1->m_pointInfo, R1->m_fStartTime, nodeData);
                value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[2]);
		rgbmap(fabs(value)*255.0,red,green,blue);
                //rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
                        file_op<<R1->m_pointInfo.phyCoord[0]<<" "<<R1->m_pointInfo.phyCoord[1]<<" "<<R1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(R1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

         /*       res=m_pField->at_phys(L1->m_pointInfo.fromCell,L1->m_pointInfo.phyCoord, L1->m_pointInfo, L1->m_fStartTime, nodeData);
                value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[3]);
		rgbmap(fabs(value)*255.0,red,green,blue);
                //rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
                        file_op<<L1->m_pointInfo.phyCoord[0]<<" "<<L1->m_pointInfo.phyCoord[1]<<" "<<L1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

                quads++;
//		}
	}
	else{
		
		points[0]=L0->m_pointInfo.phyCoord;
		points[1]=R0->m_pointInfo.phyCoord;
		points[2]=R1->m_pointInfo.phyCoord;
		points[3]=L1->m_pointInfo.phyCoord;
		calculateNormal(points,normals);
		
			//cout<<"Quad mesh with the points L0 R0 R1 L1"<<endl;
	/*	res=m_pField->at_phys(L0->m_pointInfo.fromCell,L0->m_pointInfo.phyCoord, L0->m_pointInfo, L0->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[0]);*/
		
		//value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);
		if(CALCULATE_GLOBAL_ERROR==1)
		value=errorAnalysisGlobal(points,m_lSeeds1,dt,i);//L0->itsNumStepsAlive);
		else if(CALCULATE_GLOBAL_ERROR==2)
		value=errorAnalysisQuad(points,m_pField);
		else value=0;
		//hist[int(value*bins)]++;
                hist[int(value*bins)]+=local_area;
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;
			file_op<<L0->m_pointInfo.phyCoord[0]<<" "<<L0->m_pointInfo.phyCoord[1]<<" "<<L0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(L0->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

	/*	res=m_pField->at_phys(R0->m_pointInfo.fromCell,R0->m_pointInfo.phyCoord, R0->m_pointInfo, R0->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[1]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
			file_op<<R0->m_pointInfo.phyCoord[0]<<" "<<R0->m_pointInfo.phyCoord[1]<<" "<<R0->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(R0->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);
		
				
	/*	res=m_pField->at_phys(R1->m_pointInfo.fromCell,R1->m_pointInfo.phyCoord, R1->m_pointInfo, R1->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[2]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
			file_op<<R1->m_pointInfo.phyCoord[0]<<" "<<R1->m_pointInfo.phyCoord[1]<<" "<<R1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(R1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);

	/*	res=m_pField->at_phys(L1->m_pointInfo.fromCell,L1->m_pointInfo.phyCoord, L1->m_pointInfo, L1->m_fStartTime, nodeData);
		value=sqrt(nodeData[0]*nodeData[0]+nodeData[1]*nodeData[1]+nodeData[2]*nodeData[2])*(float)FACTOR;
		nodeData=normalized(nodeData);
		value=dotproduct(nodeData,normals[3]);
		rgbmap(fabs(value)*255.0,red,green,blue);
		//rgbmap(value,red,green,blue);
		color[0]=red;color[1]=green;color[2]=blue;*/
			file_op<<L1->m_pointInfo.phyCoord[0]<<" "<<L1->m_pointInfo.phyCoord[1]<<" "<<L1->m_pointInfo.phyCoord[2]<<" "<<red<<" "<<green<<" "<<blue<<" 1\n";
		listSeedTraces.push_back(L1->m_pointInfo.phyCoord);
		listSeedTraces.push_back(color);
		
		quads++;
		}
	*L0=*R0;
	*L1=*R1;
	old_alpha=new_alpha;
	old_dis=new_dis;
	}
	i++;
	if(count_OOB>=m_lSeeds.size())
	break;
    }
	 //timer1.end();
         //cout<<"The time taken is "<<timer1.getElapsedMS()<<"MS"<<endl;
	//cout<<"Size of the  seed list "<<m_lSeeds.size()<<endl;
	//cout<<"Size of the  trace list "<<listSeedTraces.size()<<endl;
	int cnt_quads;
	for(sIdIter1 = listSeedTraces.begin();sIdIter1 != listSeedTraces.end();sIdIter1++ )
        {
        	VECTOR3 point;
                point=*sIdIter1;
		file_op1<<"v "<<point[0]<<" "<<point[1]<<" "<<point[2]<<"\n";
                sIdIter1++;
        }

	for(int j=0;j<quads*4;)
	{
		file_op<<"4 "<<j<<" "<<j+1<<" "<<j+2<<" "<<j+3<<"\n";
		file_op1<<"f "<<j+1<<" "<<j+2<<" "<<j+3<<" "<<j+4<<"\n";
		j+=4;
	}
	file_op.seekp(ios_base::beg);
	file_op<<"COFF\n"<<quads*4<<" "<<quads<<" "<<quads*4;//<<"\n";
	file_op.close();
	file_op1.close();
	cout<<"Total quads generated "<<quads<<endl;

	//cout<<"sum of area is "<<sum_area<<endl;
	//cout<<"RAND_MAX+1 is "<<RAND_MAX+1<<endl;
	//cout<<"RAND_MAX is "<<RAND_MAX<<endl;

/*	for(int i=0;i<bins;i++)
	{
		cout<<hist[i]/sum_area<<endl;
	}
	cout<<"printing second histogram"<<endl;
	for(int i=0;i<BIN_HIST;i++)
        {
                cout<<hist2[i]/sum_area<<endl;
        }	

  */      
	
	//write to a file
        ofstream file_write("streamsur03.out",ios::out | ios::binary);
        int numPoints=quads*4;
        file_write.write((char *)&numPoints,sizeof(int));
	for(sIdIter1 = listSeedTraces.begin();sIdIter1 != listSeedTraces.end();sIdIter1++ )
        {
	        VECTOR3 point;
                point=*sIdIter1;
                file_write.write((char *)&point[0],sizeof(float));
                file_write.write((char *)&point[1],sizeof(float));
                file_write.write((char *)&point[2],sizeof(float));
		sIdIter1++;
        }

        file_write.close();

/*	
	double sigma=0.0,mean=0.0;
        double globalError=0.0;
        double old_Gprob=0.0,new_Gprob=0.0;
        double min_distance=999999999999999.9,curve_min,grid_min,dist1,min_st_dis;
        double t_min=9999999999999.9;
        int OUT_OF_BOUNDS=0;
        double val1=0.0,val2=0.0;
        vtParticleInfo* vertex = new vtParticleInfo();
        dir=BACKWARD;
	int total_effective_samples=0;

	for(int z_dim=350;z_dim<430;z_dim++)
           {for(int y_dim=0;y_dim<Y_DIM;y_dim++)
              for(int x_dim=0;x_dim<X_DIM;x_dim++)
              {
		vertex->m_pointInfo.phyCoord[0]=x_dim;
                vertex->m_pointInfo.phyCoord[1]=y_dim;
                vertex->m_pointInfo.phyCoord[2]=z_dim;
		grid_min=999999.999;

                for(int rib_length=0;rib_length<m_nMaxsize;rib_length++)
                {
                	OUT_OF_BOUNDS=0;
                	//do m_nMaxsize integrations backwards
                	istat = runge_kutta4(dir, STEADY, vertex->m_pointInfo, &curTime, dt);
                	if(istat==OUT_OF_BOUND)
                	{
                	        //cout<<"out of bound"<<endl;
                	        OUT_OF_BOUNDS++;
                	}
                	//if OUT_OF_BOUND, then terminate
                	if(OUT_OF_BOUNDS>0)
                	break;
			curve_min=99999999.99;
			sIter= m_lSeeds1.begin();
                       	*L0 = *(*sIter);
			sIter= m_lSeeds1.end();
			--sIter;
                       	*L1 = *(*sIter);
		//	for(sIter = m_lSeeds1.begin();(++sIter) != m_lSeeds1.end(); )
                	{
                        //	--sIter;
                        //	*L0 = *(*sIter);
                        //	++sIter;
                        //	*L1 = *(*sIter);

                        	VECTOR3 x0,x2,x1,x1_x0,x2_x1,x0_x2,x0_x1;
                        	x1=L0->m_pointInfo.phyCoord;
                        	x2=L1->m_pointInfo.phyCoord;
                        	x0=vertex->m_pointInfo.phyCoord;
	
        	                x2_x1[0]=x2[0]-x1[0];
                	        x2_x1[1]=x2[1]-x1[1];
                	        x2_x1[2]=x2[2]-x1[2];
	
        	                x1_x0[0]=x1[0]-x0[0];
        	                x1_x0[1]=x1[1]-x0[1];
                	        x1_x0[2]=x1[2]-x0[2];

                        	x0_x2[0]=x0[0]-x2[0];
                        	x0_x2[1]=x0[1]-x2[1];
                        	x0_x2[2]=x0[2]-x2[2];

                        	x0_x1[0]=-x1_x0[0];
                        	x0_x1[1]=-x1_x0[1];
                        	x0_x1[2]=-x1_x0[2];
                        	// for the L0L1 piece of the curve, we calculate min dis
                        	t_min=-dotproduct(x1_x0,x2_x1)/dotproduct(x2_x1,x2_x1);
				if(t_min>=0.0 && t_min<=1.0)
                        	min_distance=vectorvalue(crossproduct_new(x0_x1,x0_x2))/vectorvalue(x2_x1);
				else
				{
					if(vectorvalue(x0_x1)<vectorvalue(x0_x2))
					min_distance=vectorvalue(x0_x1);
					else min_distance=vectorvalue(x0_x2);
				}
				
			//new try
				double T,s,a,b,c;
				a=vectorvalue(x2_x1);
				b=vectorvalue(x0_x1);
				c=vectorvalue(x0_x2);
				s=(a+b+c)/2.0;
				T=sqrt(s*(s-a)*(s-b)*(s-c));
			//new try
				min_distance=2*T/a;

				if(min_distance<curve_min)
				curve_min=min_distance;
			}
			if(curve_min<grid_min)
			grid_min=curve_min;
	        }
		// now we have closest distance from the seeding curve for the grid point
		// we now calculate the minimum distance from the stream surface
		min_st_dis=9999999.99;
		list<VECTOR3>::iterator sIdIter;
		for(sIdIter = listSeedTraces.begin();sIdIter != listSeedTraces.end();sIdIter++ )
		{
			VECTOR3 point;
			point=*sIdIter;
			dist1=sqrt((point[0]-x_dim)*(point[0]-x_dim)+(point[1]-y_dim)*(point[1]-y_dim)+(point[2]-z_dim)*(point[2]-z_dim));

			if(dist1<min_st_dis)
			min_st_dis=dist1;

			++sIdIter;	// skip the color information
		}
		// push back grid_min and min_st_dis
		if(grid_min<0.75)// || min_st_dis<1.0)
		{	
			scalar[x_dim][y_dim][z_dim][0]=grid_min;	
			scalar[x_dim][y_dim][z_dim][1]=min_st_dis;
			total_effective_samples++;	
		}
		else
		{
			scalar[x_dim][y_dim][z_dim][0]=-1.0;	
			scalar[x_dim][y_dim][z_dim][1]=-1.0;
		}
	      }// end of x,y loop
	//	cout<<"value of X is "<<x_dim<<" Y is "<<y_dim<<endl;
		cout<<"value of Z is "<<z_dim<<endl;
	     }
	cout<<"value of effective count is "<<total_effective_samples<<endl;
	cout<<"Now calculating the correlation coefficient"<<endl;
	double sumX=0.0,sumY=0.0,sumXY=0.0,sumXX=0.0,sumYY=0.0;

	//write to a file
        ofstream file_write1("streamsur5_.75.out",ios::out | ios::binary);
	numPoints=total_effective_samples;
        file_write1.write((char *)&numPoints,sizeof(int));
	
	for(int z_dim=0;z_dim<Z_DIM;z_dim++)
           for(int y_dim=0;y_dim<Y_DIM;y_dim++)
              for(int x_dim=0;x_dim<X_DIM;x_dim++)
		{
			if(scalar[x_dim][y_dim][z_dim][0]>0.00005)
			{
				VECTOR3 point;
				point[0]=(float)x_dim;
				point[1]=(float)y_dim;
				point[2]=(float)z_dim;
//				cout<<"x y z is "<<x_dim<<" "<<y_dim<<" "<<z_dim<<endl;
                		file_write1.write((char *)&point[0],sizeof(float));
                		file_write1.write((char *)&point[1],sizeof(float));
                		file_write1.write((char *)&point[2],sizeof(float));
//				if(scalar[x_dim][y_dim][z_dim][0]>10.0)
			cout<<"grid min is "<<scalar[x_dim][y_dim][z_dim][0]<<" min st dis is "<<scalar[x_dim][y_dim][z_dim][1]<<" x y z is "<<x_dim<<" "<<y_dim<<" "<<z_dim<<endl;
				sumX+=scalar[x_dim][y_dim][z_dim][0];
				sumY+=scalar[x_dim][y_dim][z_dim][1];
				sumXY+=scalar[x_dim][y_dim][z_dim][0]*scalar[x_dim][y_dim][z_dim][1];
				sumXX+=scalar[x_dim][y_dim][z_dim][0]*scalar[x_dim][y_dim][z_dim][0];
				sumYY+=scalar[x_dim][y_dim][z_dim][1]*scalar[x_dim][y_dim][z_dim][1];
			}
		}	
        file_write1.close();
	double coeff;
	coeff=(sumXY-sumX*sumY/total_effective_samples)/sqrt((sumXX-sumX*sumX/total_effective_samples)*(sumYY-sumY*sumY/total_effective_samples));

	cout<<"The coeff is "<<coeff<<endl;	
//*/	
	
}

double vtCStreamSurface::length(PointInfo x1,PointInfo x2)
{
	double len;
	len=sqrt((x1.phyCoord[0]-x2.phyCoord[0])*(x1.phyCoord[0]-x2.phyCoord[0])+
		(x1.phyCoord[1]-x2.phyCoord[1])*(x1.phyCoord[1]-x2.phyCoord[1])+
		(x1.phyCoord[2]-x2.phyCoord[2])*(x1.phyCoord[2]-x2.phyCoord[2]));
	return len;
	
}
