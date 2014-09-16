#include <math.h>
#include <stdio.h>
#include <modes.h>
#include <time.h>
#include "cutil_math.h"
#include "thrust\device_vector.h"
#include "thrust\reduce.h"
#include "thrust\count.h"
#include "thrust\scan.h"
#include "thrust\unique.h"
#include "thrust\remove.h"
#include "thrust\adjacent_difference.h"
#include "TransformMgr.h"
#include "DeformGPU.h"
//#include "StreamDeform.h"


//set it large to avoid flashing in the end of deformation
//if it is too large, the points may not recover when out of target region
#define EPS 1E-9		
#define M_PI       3.14159265358979323846
#define MAX_NUM_GROUP 16

//paramters
float _para;

//window
__device__ __constant__ int WIN_WIDTH;
__device__ __constant__ int WIN_HEIGHT;
int _h_winWidth;
int _h_winHeight;
__device__ __constant__ float MODELVIEW[ 16 ]; 
__device__ __constant__ float INVMODELVIEW[ 16 ];    
__device__ __constant__ float PROJECTION[ 16 ];    
__device__ __constant__ float INVPROJECTION[ 16 ]; 
float _h_modelview[16];
float _h_projection[16];
float *_invModelView;
float *_invProjection;
bool *_deformOn;

thrust::device_vector<hull_type> d_vec_hullSet;

//convex hull
thrust::device_vector<float2> _d_hull;
std::vector<ellipse>* _h_vec_ellipseSet;

//streamline
int _nv;
__device__ __constant__ int NV;

//these two device_ptr are the OpenGL pointer
thrust::device_ptr<float3> _d_ptr_tangent;
thrust::device_ptr<float4> _d_ptr_posClip;
thrust::device_ptr<int> _d_ptr_translucent;

thrust::device_vector<int> d_vec_streamlineLengths;
thrust::device_vector<int> d_vec_streamlineOffsets;
thrust::device_vector<int> d_vec_streamlineLengthsOrig;
thrust::device_vector<int> d_vec_streamlineOffsetsOrig;

thrust::device_vector<float4> _d_vec_pos;
thrust::device_vector<float4> _d_vec_prePos;
thrust::device_vector<float4> _d_vec_origPos;

thrust::device_vector<float2> d_vec_posScreen;

thrust::device_vector<int> _d_vec_lineIndex;
thrust::device_vector<int> _d_vec_lineIndexOrig;
thrust::device_vector<bool> _d_vec_vertexIsFocus;

thrust::device_vector<int> _d_vec_cutPointsMarkOrig;	
thrust::device_vector<int> _d_vec_cutPointsMark;	


thrust::counting_iterator<int> counting_zero(0);
thrust::host_vector<int> *_pickedLineSet;

//lens
float *_lens_center;
//float *_lensDepth_clip;
float4* _lensCenterObject;

DEFORM_MODE *_deformMode;
SOURCE_MODE *_sourceMode;

//StreamDeform* _deformLine;
//
//void SetStreamDeform(void* sd)
//{
//	_deformLine = (StreamDeform*)sd;
//}

__device__ __host__ inline float2 GetXY(float4 pos)
{
	return make_float2(pos.x, pos.y);
}

__device__ inline float3 GetXYZ(float4 v)
{
	return make_float3(v.x, v.y, v.z);
}

__device__ inline float2 rotate(float2 p, float deg)
{
	return make_float2(p.x * cos(deg) - p.y * sin(deg), p.x * sin(deg) + p.y * cos(deg));
}

//Object space-->Camera space-->Clip space-->Screen space
//multiply projection and modelview matrix
__device__ inline float4 Clip2Object(float4 p)//, float modelview[16], float projection[16])
{
	return Clip2Object(p, INVMODELVIEW, INVPROJECTION);
}

__device__ __host__ inline float4 Object2Clip(float4 pos)//, float modelview[16], float projection[16])
{
#ifdef __CUDA_ARCH__ // __CUDA_ARCH__ // 
	return Object2Clip(pos, MODELVIEW, PROJECTION);
#else
	return Object2Clip(pos, _h_modelview, _h_projection);
#endif
}

//only multiply projection matrix
__device__ __host__ inline float4 Camera2Clip(float4 pos)
{
#ifdef __CUDA_ARCH__ 
	return Camera2Clip(pos, PROJECTION);
#else
	return Camera2Clip(pos, _h_projection);
#endif
}

//multiply modelview matrix
__device__ __host__ inline float4 Object2Camera(float4 pos)//, float modelview[16], float projection[16])
{
#ifdef __CUDA_ARCH__ 
	return Object2Camera(pos, MODELVIEW);
#else
	return Object2Camera(pos, _h_modelview);
#endif
}

__device__ __host__ inline float2 Clip2Screen(float2 p)
{
#ifdef __CUDA_ARCH__
	return Clip2Screen(p, WIN_WIDTH, WIN_HEIGHT);
#else
	return Clip2Screen(p, _h_winWidth, _h_winHeight);
#endif
}

__device__ __host__ inline float2 Screen2Clip(float2 p)
{
#ifdef __CUDA_ARCH__
	return Screen2Clip(p, WIN_WIDTH, WIN_HEIGHT);
#else
	return Screen2Clip(p, _h_winWidth, _h_winHeight);
#endif
}

__device__ __host__ inline float2 Object2Screen(float4 p)
{
	return Clip2Screen(GetXY(Object2Clip(p)));
}

//__device__ float2 force_linear(float2 distance, float c)
//{
//	float2 force = c * distance;
//	return force;
//}

__device__ inline float3 GetNormal(int i, int *lineIndex, float4* pos)
{
	float4 A, B, C;
	float4 pos_camera = Object2Camera(pos[i]);
	if(i >= (NV - 2) || lineIndex[i] != lineIndex[i + 1])	//last point of a streamline
	{
		A = Object2Camera(pos[i - 2]);
		//B = Object2Camera(pos[i - 1]);
		C = pos_camera;
	}
	else if(i <= 1 || lineIndex[i] != lineIndex[i - 1])	//first point of a streamline
	{
		A = pos_camera;
		//B = Object2Camera(pos[i + 1]);
		C = Object2Camera(pos[i + 2]);
	}
	else
	{
		A = Object2Camera(pos[i - 1]);
		//B = pos_camera;
		C = Object2Camera(pos[i + 1]);
	}
//	float3 BC = GetXYZ(C - B);
//	float3 AB = make_float3(B - A);
//	float3 binormal = cross(AB, BC);//AB X BC
	return normalize(GetXYZ(C - A));// crossProduct(AB, binormal);
}

struct functor_computeNormal
{
	int* lineIndex;
	float4* pos;
	template <typename T>
	__device__ void operator() (T t)
	{
		thrust::get<1>(t) = GetNormal(thrust::get<0>(t), lineIndex, pos);//make_float3(0,1,0);//
	}

	functor_computeNormal(int *_lineIndex, float4* _pos)
	{
		lineIndex = _lineIndex;
		pos = _pos;
	}
};

__device__ inline float4 Recover3DPosition(float4 pos, float4 origPos, float moveSpeed)
{
	float4 dir = origPos - pos;
	float4 temp;
	if(length(dir) > 0.1)
	{
		float4 moveStep =  5 * moveSpeed * dir;
		temp = pos + moveStep;
	}
	else
		temp = origPos;

	return temp;
}

__device__ inline float2 CatmullRom_Evaluate(float2 p0, float2 p1, float2 p2, float2 p3, float t)
{
	const float c0 = ((-t + 2.0f) * t - 1.0f) * t * 0.5f;
	const float c1 = (((3.0f * t - 5.0f) * t) * t + 2.0f) * 0.5f;
	const float c2 = ((-3.0f * t + 4.0f) * t + 1.0f) * t * 0.5f;
	const float c3 = ((t - 1.0f) * t * t) * 0.5f;
	
	float2 ret = c0 * p0 + c1 * p1 + c2 * p2 + c3 * p3;
	return ret;
}

__device__ float2 ProjectPoint2Line(float2 p, float2 p1, float2 p2)
{
	float2 v = p - p1;
	float2 v2 = p2 - p1;
	return p1 + dot(v, v2) / dot(v2, v2) * v2;
}

//http://geomalgorithms.com/a02-_lines.html
//Distance of a Point to an finite Line
__device__ inline float Point2Line(float2 p, float2 p0, float2 p1)
{
	float ret;
	float m = p0.y - p1.y;
	float n = p1.x - p0.x;
	float a = m * p.x + n * p.y + (p0.x * p1.y - p1.x * p0.y);
	float b = sqrtf(m * m + n * n);
	if(dot(p - p0, p1 - p0) < 0)
		ret = length(p - p0);
	else if( dot(p - p1, p0 - p1) < 0)
		ret = length(p - p1);
	else
		ret = abs(a / b);
	return ret;
}


__device__ inline float Vector2Angle(float2 v)
{
	float t = atan2(v.y, v.x);
	if(t < 0)
		t += 2 * M_PI; 
	return t;
}

__device__ inline float radius_ellipse(ellipse e, float2 dir2Center)
{
	//Polar form relative to center: http://en.wikipedia.org/wiki/Ellipse
	float t = Vector2Angle(dir2Center);

	//rotate to canonical position
	t = t - e.angle;	
	return e.a * e.b / sqrt(pow(e.b * cos(t),2) + pow(e.a * sin(t),2));
}

__device__ inline float radius_blade(ellipse e, float2 center)
{
	float parallelDist = 0.85 * length(center - make_float2(e.x, e.y));
	return 1.25 * e.b * ( tanh(- 8.0 * parallelDist / e.a + 6.0) + 1) * 0.5;
}

__device__ inline bool inOneEllipse(ellipse e, float2 p, float radius)
{
	float2 center = make_float2(e.x, e.y);
	float2 dir2Center = p - center;
	return length(dir2Center) < radius;
}

__device__ inline bool inOneEllipse(ellipse e, float2 p)
{
	float2 center = make_float2(e.x, e.y);
	float2 dir2Center = p - center;
	float radius = radius_ellipse(e, dir2Center);
	return length(dir2Center) < radius;
}

__device__ inline bool inAnyGroup(ellipse* ellipseSet, float* radiusOuter, int cnt, float4 p)
{
	for(int i = 0; i < cnt; i++)
		if(inOneEllipse(ellipseSet[i], Object2Screen(p), radiusOuter[i]))//length(make_float2(ellipseSet[i].x,ellipseSet[i].y) - Object2Screen(p)) < radiusOuter[i])
			return true;
	return false;
}

__device__ inline bool inOneBlade(ellipse e, float radius, float2 p)
{
	float2 line[2];
	line[0] = make_float2(e.x - e.a * cos(e.angle), e.y - e.a * sin(e.angle));
	line[1] = make_float2(e.x + e.a * cos(e.angle), e.y + e.a * sin(e.angle));
	float2 center = ProjectPoint2Line(p, line[0], line[1]);
	return length(center - p) < radius;
}

__device__ inline bool inOneBlade(ellipse e, float2 p)
{
	float2 line[2];
	line[0] = make_float2(e.x - e.a * cos(e.angle), e.y - e.a * sin(e.angle));
	line[1] = make_float2(e.x + e.a * cos(e.angle), e.y + e.a * sin(e.angle));
	float2 center = ProjectPoint2Line(p, line[0], line[1]);
	float radius = radius_blade(e, center);
	return length(center - p) < radius;
}

__device__ inline bool inAnyGroupLine(ellipse* ellipseSet, float* radiusOuter, int cnt, float4 p)
{
	for(int i = 0; i < cnt; i++)
	{
		ellipse e = ellipseSet[i];
		//float2 line[2];
		//line[0] = make_float2(e.x - e.a * cos(e.angle), e.y - e.a * sin(e.angle));
		//line[1] = make_float2(e.x + e.a * cos(e.angle), e.y + e.a * sin(e.angle));
		//float2 v_screen = Object2Screen(p);
		//float2 center = ProjectPoint2Line(v_screen, line[0], line[1]);
		if(inOneBlade(e, radiusOuter[i], Object2Screen(p)))
			return true;
	}
	return false;
}

__device__ inline bool IsBetweenAngles(float v, float a, float b)
{
	float a_b = 0;
	if((a - b) > M_PI)
		return (v >= a || v < b);
	else if((b - a) > M_PI)
		return (v >= b || v < a);
	else
		return (v >= a && v < b) || (v >= b && v < a);
}

__device__ inline float CatmullRomAngle(float a0, float2 center,
	float2 p0, float2 p1, float2 p2, float2 p3)
{
	const int ndivs = 16;
	float angles[ndivs];
	float a_diff_min = FLT_MAX;
	float a_closest = 0;
	float2 iP_closest;
	for(int i = 0; i < ndivs; i++)
	{
		float t = (float)(i) / (float)(ndivs- 1);
		float2 iP = CatmullRom_Evaluate(p0, p1, p2, p3, t);
		float a = Vector2Angle(iP - center);
		if(abs(a - a0) < a_diff_min)
		{
			a_diff_min = abs(a - a0);
			iP_closest = iP;
		}
	}
	return length(iP_closest - center);
}

__device__ inline float radius_hull(hull_type hull, float2 hullCenter, float2 p)
{
	//float2 closest;
	//GetClosestHullPointCatmullRom(closest, p, hull.v, hull.nv);
	float ret = 0;
	float t0 = Vector2Angle(p - hullCenter);
	float hullAngles[HULL_SIZE];
	for(int i = 0; i < hull.nv; i++)
	{
		hullAngles[i] = Vector2Angle(hull.v[i] - hullCenter);
	}
	for(int i = 0; i < hull.nv;i++)
	{
		if(IsBetweenAngles(t0, hullAngles[i], hullAngles[(i + 1) % hull.nv]))
		{
			ret = CatmullRomAngle(t0, hullCenter, 
				hull.v[(i - 1 + hull.nv)% hull.nv], hull.v[i], 
				hull.v[(i + 1) % hull.nv], hull.v[(i + 2) % hull.nv]);
			//ret = length(hull.v[i] - hullCenter);
			break;
		}
	}
	return ret;
	//return length(closest - hullCenter);
}

__device__ inline float G(float x, float r)
{
	return pow((r - 1), 2) / ( - r * r * x + r) + 2 - 1 / r;
}

//kernels
__global__ void kernel_convex(float4* pos, float4* pos_clip, float2* pos_screen,
	float4* prevPos, float4* origPos, int *lineIndex, bool *vertexIsFocus, int _nv, 
	ellipse* ellipseSet, hull_type* hullSet, int nEllipse, 
	DEFORM_MODE deformMode, float pa)//, unsigned int nv)
{
    unsigned int i = blockIdx.x*blockDim.x + threadIdx.x;
	if(i >= _nv)		//if more than the number of vertices
		return;

	float2 v_screen = pos_screen[i];
	float2 orig_screen= Object2Screen(origPos[i]);
	float2 forceAll= make_float2(0, 0);
	float moveSpeed = 0.01;
	const float transRatio = 0.2;
	float generalSize = (ellipseSet[0].a + ellipseSet[0].b) * 0.5;

	float radiusOuterAll[MAX_NUM_GROUP]; //including the focus and transition region

	bool inAnyEllipse = false;
	for(int ie = 0; ie < nEllipse; ie++)
	{
		float2 center;
		ellipse e = ellipseSet[ie];
		if(DEFORM_MODE::MODE_LINE == deformMode)
		{
			float2 line[2];
			line[0] = make_float2(e.x - e.a * cos(e.angle), e.y - e.a * sin(e.angle));
			line[1] = make_float2(e.x + e.a * cos(e.angle), e.y + e.a * sin(e.angle));
			center = ProjectPoint2Line(v_screen, line[0], line[1]);
		}
		else
			center = make_float2(e.x, e.y);
		float2 dir2Center = v_screen - center;	//current distance to the center

		float radius;

		if(DEFORM_MODE::MODE_HULL == deformMode)
			radius = 1.2 * radius_hull(hullSet[ie], center, v_screen);
		else if(DEFORM_MODE::MODE_LINE == deformMode)
		{
			//float parallelDist = 0.85 * length(center - make_float2(e.x, e.y));
			//radius = 1.25 * e.b * ( tanh(- 8.0 * parallelDist / e.a + 6.0) + 1) * 0.5;
			radius = radius_blade(e, center);
		}
		else//		if(DEFORM_MODE::MODE_ELLIPSE == deformMode)
			radius = 1.2 * radius_ellipse(e, dir2Center);
		//float transWidth = generalSize * transRatio; //size of transition region, _d_ / _r_
		float r = 0.5;
		radiusOuterAll[ie] = radius / r;// (radius + transWidth) ;
		
		float origDist2Center = length(orig_screen - center);	//original distance to the center
		if(origDist2Center <= radiusOuterAll[ie] && (false == vertexIsFocus[i]))
		{
			inAnyEllipse = true;
			//force from neighboring vertices
			if(deformMode == DEFORM_MODE::MODE_ELLIPSE)
			{
				if(i != 0 && i != (_nv - 1))
				{
					if((lineIndex[i] == lineIndex[i + 1]) && (lineIndex[i] == lineIndex[i - 1])) //if it is not the first vertex or last vertex on the line
					{
						float2 pre_v1_screen = Object2Screen(prevPos[i + 1]);//projection * modelview * v;
						float2 pre_v_1_screen = Object2Screen(prevPos[i - 1]);//projection * modelview * v;
						float2 edge_v1 = pre_v1_screen - v_screen;
						float2 edge_v_1 = pre_v_1_screen - v_screen;// mul(10,minus(pre_v_1_screen, v));
						float edgeLength_v1 = length(edge_v1);
						float edgeLength_v_1 = length(edge_v_1);
						// if the parameter is too small, the lines are jaggy
						// if the parameter is too large, the lines tend to be straight
						if(edgeLength_v1 > (edgeLength_v_1 * 1.001) )
							forceAll += edge_v1 * 2;	
						else if(edgeLength_v_1 > (edgeLength_v1 * 1.001) )
							forceAll += edge_v_1 * 2;
					}
				}
			}
		
			float2 dir = normalize(dir2Center);

			float desiDist2Center = G(origDist2Center / radiusOuterAll[ie], r) * radiusOuterAll[ie];//radius + origDist2Center / (radius / transWidth + 1.0);		//distance to the center for the destination position
			float dist2Center = length(dir2Center);	//current distance to the center 
			float dist2Desire = desiDist2Center - dist2Center;
			forceAll += (dir * dist2Desire);
		}
	}
	if(length(forceAll) >= generalSize * 0.1)
	{
		v_screen += moveSpeed * forceAll;

		float2 v = Screen2Clip(v_screen);
		pos_clip[i].x = v.x;//ELLIPSE_CENTER.x / WIN_WIDTH * 2.0 - 1.0;//v.x;
		pos_clip[i].y = v.y;//ELLIPSE_CENTER.y / WIN_HEIGHT * 2.0 - 1.0;//v.y;
		pos[i] = Clip2Object(pos_clip[i]);
	}
	if(	!inAnyEllipse|| vertexIsFocus[i])
	{
		//make sure it would not recover into the deformation region
		pos[i] = Recover3DPosition(pos[i], origPos[i], moveSpeed);;
		pos_clip[i] = Object2Clip(pos[i]);
	}
}

void RestorePos()
{
	thrust::copy(_d_vec_origPos.begin(), _d_vec_origPos.end(), _d_vec_pos.begin());
}

void RestoreConnectivity()
{
	d_vec_streamlineLengths = d_vec_streamlineLengthsOrig;
	d_vec_streamlineOffsets = d_vec_streamlineOffsetsOrig;
	_d_vec_cutPointsMark = _d_vec_cutPointsMarkOrig;
}

struct functor_Object2Clip
{
	__device__ float4 operator() (float4 p)
	{
		return Object2Clip(p);
	}
};

struct functor_Object2Screen
{
	__device__ float2 operator() (float4 p)
	{
		return Object2Screen(p);
	}
};


struct functor_Clip2Screen
{
	__device__ float2 operator() (float4 p)
	{
		return Clip2Screen(GetXY(p));
	}
};

struct functor_UpdateVertexIsFocusByLens
{
	bool* vertexIsFocus;
	float4* posClip;
	float2* posScreen;
	ellipse e;
	float lens_z_clip;
	template <typename T>
	__device__ void operator() (T t)
	{
		int offset = thrust::get<0>(t);
		int len = thrust::get<1>(t);
		bool passed = false;
		
		for(int i = 0; i < len; i++)
		{
			float2 p_screen = posScreen[offset + i];
			float p_depth = posClip[offset + i].z;
			float2 dirFromCenter = p_screen - make_float2(e.x, e.y);
			float radius = radius_ellipse(e, dirFromCenter);
			if(length(dirFromCenter) < radius && p_depth < lens_z_clip)
				passed = true;
		}
		for(int i = 0; i < len; i++)
			vertexIsFocus[offset + i] = !passed;
	}

	functor_UpdateVertexIsFocusByLens(bool* _vertexIsFocus, float4* _posClip, float2* _posScreen, 
		ellipse _e, float _lens_z_clip)
	{
		vertexIsFocus = _vertexIsFocus;
		posClip = _posClip;
		posScreen = _posScreen;
		e = _e;
		lens_z_clip = _lens_z_clip;
	}
};

void ResetVertexIsFocus()
{
	_d_vec_vertexIsFocus.assign(_nv, false);
}

void UpdateVertexIsFocusByLens()
{
	float4 lensCenterClip = Object2Clip(*_lensCenterObject);
	float lensDepth_clip = lensCenterClip.z;

	thrust::device_vector<float2> d_vec_origPosScreen(_nv);
	thrust::device_vector<float4> d_vec_origPosClip(_nv);
	thrust::transform(_d_vec_origPos.begin(), _d_vec_origPos.end(), d_vec_origPosClip.begin(), functor_Object2Clip());
	thrust::transform(d_vec_origPosClip.begin(), d_vec_origPosClip.end(), d_vec_origPosScreen.begin(), functor_Clip2Screen());

	////http://stackoverflow.com/questions/3717226/radius-of-projected-sphere
	//use the original position to solve the vibrating problem, because when deformed streamline changes depth
	if(_h_vec_ellipseSet->size() > 0)
	{
		ellipse ell = _h_vec_ellipseSet->front();
		thrust::for_each(
			thrust::make_zip_iterator(thrust::make_tuple(d_vec_streamlineOffsetsOrig.begin(), d_vec_streamlineLengthsOrig.begin(), counting_zero)),
			thrust::make_zip_iterator(thrust::make_tuple(d_vec_streamlineOffsetsOrig.end(), d_vec_streamlineLengthsOrig.end(), counting_zero + d_vec_streamlineOffsets.size())),
			functor_UpdateVertexIsFocusByLens(
				thrust::raw_pointer_cast(_d_vec_vertexIsFocus.data()),
				thrust::raw_pointer_cast(d_vec_origPosClip.data()), 
				thrust::raw_pointer_cast(d_vec_origPosScreen.data()),
				ell,
				lensDepth_clip));
	}
}

struct functor_GetLineIndexInRange 
{
	float min[3];
	float max[3];
	template <typename T>
	__device__ void operator() (T t)
	{
		int idx = thrust::get<0>(t);
		float4 p = thrust::get<1>(t);
		if(p.x >= min[0] && p.x < max[0]
		&&	p.y >= min[1] && p.y < max[1]
		&&	p.z >= min[2] && p.z < max[2])
			thrust::get<2>(t) = idx;
		else
			thrust::get<2>(t) = -1;
	}

	functor_GetLineIndexInRange(float _min[3], float _max[3])
	{
		for(int i = 0; i < 3; i++)
		{
			min[i] = _min[i];
			max[i] = _max[i];
		}
	}
};

thrust::host_vector<int> PickStreamByBlockCUDA(float min[3], float max[3])
{
	vector<int> picked;
	thrust::device_vector<int> result(_nv, -1);
	thrust::for_each(
		thrust::make_zip_iterator(thrust::make_tuple(
		_d_vec_lineIndexOrig.begin(), _d_vec_origPos.begin(), result.begin())), 
		thrust::make_zip_iterator(thrust::make_tuple(
		_d_vec_lineIndexOrig.end(),	_d_vec_origPos.end(), result.end())), 
		functor_GetLineIndexInRange(min, max));

	thrust::device_vector<int>::iterator newEnd = thrust::unique(result.begin(), result.end());
	newEnd = thrust::remove(result.begin(), newEnd, -1);
	thrust::host_vector<int> h_result(result.begin(), newEnd);
	return h_result;
}

struct functor_GenLineIndex
{
	int* index;

	template <typename T>
	__device__ void operator() (T t)
	{
		int offset = thrust::get<0>(t);
		int length = thrust::get<1>(t);
		int lineIdx = thrust::get<2>(t);
		
		for(int i = 0; i < length; i++)
		{
			index[offset + i] = lineIdx;
		}
	}

	functor_GenLineIndex(int* _index)
	{
		index = _index;
	}
};

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
__device__ inline bool onSegment(float2 p, float2 q, float2 r)
{
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
       return true;
 
    return false;
}

//*******from http://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
__device__  inline int orientation(float2 p, float2 q, float2 r)
{
    // See 10th slides from following link for derivation of the formula
    // http://www.dcs.gla.ac.uk/~pat/52233/slides/Geometry1x1.pdf
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);
 
    if (val == 0) return 0;  // colinear
 
    return (val > 0)? 1: 2; // clock or counterclock wise
}
 
// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
__device__  inline bool doIntersect(float2 p1, float2 q1, float2 p2, float2 q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);
 
    // General case
    if (o1 != o2 && o3 != o4)
        return true;
 
    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
 
    // p1, q1 and p2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
 
    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
 
     // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;
 
    return false; // Doesn't fall in any of the above cases
}

	//thrust::for_each(
	//	thrust::make_zip_iterator(thrust::make_tuple(_d_vec_cutPointsMark.begin(),counting_zero)),
	//	thrust::make_zip_iterator(thrust::make_tuple(_d_vec_cutPointsMark.end(), counting_zero+ d_vec_filledOffset.size())),
	//	functor_ComputeCutPointsWithLine(
	//		thrust::raw_pointer_cast(d_vec_ellipseSet.data()),
	//		_h_vec_ellipseSet->size(),
	//		thrust::raw_pointer_cast(d_vec_posScreen.data())
	//		));

//__device__ __host__ inline bool InsideBlade(float2 p, ellipse e)
//{
//
//}

struct functor_ComputeCutPointsWithLine
{
	ellipse* ellipseSet;
	int numEllipses;
	float2* posScreen;

	template <typename T>
	__device__ void operator() (T t)
	{
		bool vertexIsFocus = thrust::get<1>(t);
		int cutPointsMark = thrust::get<0>(t);
		if(vertexIsFocus)
		{
			if(2 == cutPointsMark)
				thrust::get<0>(t) = 0;
			return;
		}

		int index = thrust::get<2>(t);
		if(0 == index )//when index == 0, posScreen[index - 1] has no value
			return;

		//meaning of the values of cutPointMark:
		//0: not any cut point
		//1: the first vertex of a streamline
		//2: the cut point
		if(2 == cutPointsMark)		//see weather make it 0
		{
			//no use!!!
			//recover the cut points that have left the ellipse region
			//bool insideAnyBlades = false;
			//for(int j = 0; j < numEllipses; j++)
			//{
			//	if(inOneBlade(ellipseSet[j], ellipseSet[j].b * 0.8, posScreen[index]))
			//		insideAnyBlades = true;
			//}
			//if(!insideAnyBlades)
			//	thrust::get<0>(t) = 0;
		}	
		else if(0 == cutPointsMark) {
			for(int j = 0; j < numEllipses; j++)
			{
				ellipse e = ellipseSet[j];
				float2 line[2];
				line[0] = make_float2(e.x - e.a * cos(e.angle), e.y - e.a * sin(e.angle));
				line[1] = make_float2(e.x + e.a * cos(e.angle), e.y + e.a * sin(e.angle));
				if(doIntersect(
					posScreen[index - 1], posScreen[index], 
					line[0], line[1]))
					thrust::get<0>(t) = 2;
			}
		}
		else //(1 == cutPointsMark )	
		{
			//doing nothing
		}
	}

	functor_ComputeCutPointsWithLine(
		ellipse* _ellipseSet, int _numEllipses, float2* _posScreen)
	{
		ellipseSet = _ellipseSet;
		numEllipses = _numEllipses;
		posScreen = _posScreen;
	}
};
	//thrust::for_each(
	//	thrust::make_zip_iterator(thrust::make_tuple(_d_vec_cutPointsMark.begin(), d_vec_filledOffset.begin(), counting_zero)),
	//	thrust::make_zip_iterator(thrust::make_tuple(_d_vec_cutPointsMark.end(), d_vec_filledOffset.end(), counting_zero + _d_vec_cutPointsMark.size())),
	//	functor_AssignFilledOffsetsFramMask()
	//	);
struct functor_AssignFilledOffsetsFramMask
{
	template <typename T>
	__device__ void operator() (T t)
	{
		int cutPointsMask = thrust::get<0>(t);
		int idx = thrust::get<2>(t);
		if(1 == cutPointsMask || 2 == cutPointsMask)
			thrust::get<1>(t) = idx;
	}
};

struct functor_UpdateCutPointsMarkByConnectivity
{
	int *cutPointsMark;

	//template <typename T>
	__device__ void operator() (int offset)
	{
//		int offset = thrust::get<0>(t);
		cutPointsMark[offset] = 1;
	}

	functor_UpdateCutPointsMarkByConnectivity(int *_cutPointsMark)
	{
		cutPointsMark = _cutPointsMark;
	}
};


struct functor_PushFromLine
{
	ellipse *ellipseSet;
	int numEllipses;

	template <typename T>
	__device__ void operator() (T t)
	{
		if(thrust::get<1>(t) < 0)
			return;
		float2 p_screen = thrust::get<0>(t);
		for(int j = 0; j < numEllipses; j++)
		{
			ellipse e = ellipseSet[j];
			float2 line[2];
			line[0] = make_float2(e.x - e.a * cos(e.angle), e.y - e.a * sin(e.angle));
			line[1] = make_float2(e.x + e.a * cos(e.angle), e.y + e.a * sin(e.angle));
			if(Point2Line(p_screen, line[0], line[1]) < 2)
			{
				float2 p_proj = ProjectPoint2Line(p_screen, line[0], line[1]);
				float2 dir = normalize(p_screen - p_proj);
				p_screen = p_screen + dir * 4;
			}
		}
		thrust::get<0>(t) = p_screen;
	}

	functor_PushFromLine(ellipse* _ellipseSet, int _numEllipses)
	{
		ellipseSet = _ellipseSet;
		numEllipses = _numEllipses;
	}
};

struct functor_Screen2ObjectOnSamePlane
{
	template <typename T>
	__device__ void operator() (T t)
	{
		float4 pos_clip = thrust::get<1>(t);
		float2 v_screen = thrust::get<2>(t);

		float2 v = Screen2Clip(v_screen);
		pos_clip.x = v.x;//ELLIPSE_CENTER.x / WIN_WIDTH * 2.0 - 1.0;//v.x;
		pos_clip.y = v.y;//ELLIPSE_CENTER.y / WIN_HEIGHT * 2.0 - 1.0;//v.y;
		thrust::get<0>(t) = Clip2Object(pos_clip);
	}
};

struct functor_UpdateLineIndexWithPickedLine
{
	int* offsets;
	int* lengths;
	bool* vertexIsFocus;

//	template <typename T>
	__device__ void operator() (int picked)
	{
		int offset = offsets[picked];
		int length = lengths[picked];
		for(int i = 0; i < length; i++)	{
			int idx = offset + i;
			vertexIsFocus[idx] = true;
		}
	}

	functor_UpdateLineIndexWithPickedLine(int* _offsets, int* _lengths, bool* _vertexIsFocus)
	{
		offsets = _offsets;
		lengths = _lengths;
		vertexIsFocus = _vertexIsFocus;
	}
};

void UpdateLineIndex()
{
	thrust::for_each(
		thrust::make_zip_iterator(thrust::make_tuple(d_vec_streamlineOffsets.begin(), d_vec_streamlineLengths.begin(), counting_zero)),
		thrust::make_zip_iterator(thrust::make_tuple(d_vec_streamlineOffsets.end(), d_vec_streamlineLengths.end(), counting_zero + d_vec_streamlineOffsets.size())),
		functor_GenLineIndex(thrust::raw_pointer_cast(_d_vec_lineIndex.data())));
}

void ComputeCutPoints()
{
	//this offset use the index to mark the first element of a streamline, 
	//and use -1 to mark the others
	thrust::device_vector<int> d_vec_filledOffset;	
	d_vec_filledOffset.assign(_nv, -1);
	//thrust::for_each(
	//	//d_vec_streamlineOffsets.begin(), d_vec_streamlineOffsets.end(),
	//	thrust::make_zip_iterator(thrust::make_tuple(d_vec_streamlineOffsets.begin(), d_vec_streamlineLengths.begin())),
	//	thrust::make_zip_iterator(thrust::make_tuple(d_vec_streamlineOffsets.end(), d_vec_streamlineLengths.end())),
	//functor_AssignFilledOffsets(
	//	thrust::raw_pointer_cast(d_vec_filledOffset.data()),
	//	thrust::raw_pointer_cast(_d_vec_vertexIsFocus.data())
	//	));

	thrust::device_vector<float2> d_vec_posScreen(_nv);
	thrust::device_vector<float4> d_vec_posClip(_nv);
	thrust::transform(_d_vec_pos.begin(), _d_vec_pos.end(), d_vec_posClip.begin(), functor_Object2Clip());
	thrust::transform(d_vec_posClip.begin(), d_vec_posClip.end(), d_vec_posScreen.begin(), functor_Clip2Screen());

	thrust::device_vector<ellipse> d_vec_ellipseSet = *_h_vec_ellipseSet;

	//compute the new _d_vec_cutPointsMark
	thrust::for_each(
		thrust::make_zip_iterator(thrust::make_tuple(_d_vec_cutPointsMark.begin(), _d_vec_vertexIsFocus.begin(), counting_zero)),
		thrust::make_zip_iterator(thrust::make_tuple(_d_vec_cutPointsMark.end(), _d_vec_vertexIsFocus.end(), counting_zero+ d_vec_filledOffset.size())),
		functor_ComputeCutPointsWithLine(
			thrust::raw_pointer_cast(d_vec_ellipseSet.data()),
			_h_vec_ellipseSet->size(),
			thrust::raw_pointer_cast(d_vec_posScreen.data())
			));

	//use _d_vec_cutPointsMark to update d_vec_filledOffset
	thrust::for_each(
		thrust::make_zip_iterator(thrust::make_tuple(_d_vec_cutPointsMark.begin(), d_vec_filledOffset.begin(), counting_zero)),
		thrust::make_zip_iterator(thrust::make_tuple(_d_vec_cutPointsMark.end(), d_vec_filledOffset.end(), counting_zero + _d_vec_cutPointsMark.size())),
		functor_AssignFilledOffsetsFramMask()
		);

	//use d_vec_filledOffset to update connectivity
	thrust::device_vector<int>::iterator filledOffsetEnd = thrust::remove(d_vec_filledOffset.begin(), d_vec_filledOffset.end(), -1); 
	int newSize = filledOffsetEnd - d_vec_filledOffset.begin();
	d_vec_streamlineOffsets.resize(newSize);
	thrust::copy(d_vec_filledOffset.begin(), filledOffsetEnd, d_vec_streamlineOffsets.begin());
	thrust::device_vector<int> d_vec_streamlineLengthsTmp(newSize);
	thrust::adjacent_difference(d_vec_streamlineOffsets.begin(), d_vec_streamlineOffsets.end(), d_vec_streamlineLengthsTmp.begin());
	d_vec_streamlineLengths.resize(newSize);
	thrust::copy(d_vec_streamlineLengthsTmp.begin() + 1, d_vec_streamlineLengthsTmp.end(), d_vec_streamlineLengths.begin());
	d_vec_streamlineLengths.back() = _nv - d_vec_streamlineOffsets.back();
	
	UpdateLineIndex();
}

void UpdateLineIndexWithPickedLine()
{
	//thrust::for_each(
	//	thrust::make_zip_iterator(thrust::make_tuple(
	//		d_vec_streamlineOffsets.begin(), d_vec_streamlineLengths.begin(), counting_zero)),
	//	thrust::make_zip_iterator(thrust::make_tuple(
	//		d_vec_streamlineOffsets.end(), d_vec_streamlineLengths.end(), counting_zero + d_vec_streamlineOffsets.size())),
	//	functor_GenLineIndex(thrust::raw_pointer_cast(_d_vec_lineIndex.data()))
	//	);

	//cout<<"cnt2:"<<_d_vec_lineIndex.back()<<endl;

	//cout<<"*****"<<endl;
	//_d_vec_lineIndex = _d_vec_lineIndexOrig;
	thrust::device_vector<int> picked_lineSet;// = *_pickedLineSet;
	picked_lineSet.assign(_pickedLineSet->begin(), _pickedLineSet->end());

	thrust::for_each(picked_lineSet.begin(), picked_lineSet.end(),
		functor_UpdateLineIndexWithPickedLine(
			thrust::raw_pointer_cast(d_vec_streamlineOffsetsOrig.data()),
			thrust::raw_pointer_cast(d_vec_streamlineLengthsOrig.data()),
			thrust::raw_pointer_cast(_d_vec_vertexIsFocus.data()))
		);
	//for(int i = 0; i < _d_vec_lineIndex.size(); i+= 100)	{
	//	cout<<_d_vec_lineIndex[i] << ", ";
	//}
	//then reverse the sign of the vertices of the picked streamlines

}


//n is the number of vertices
void SetVertexCoords(float* data, int n)
{
	_nv = n;
	_d_vec_pos.assign((float4*)data, (float4*)data + _nv); 
	
	cudaMemcpyToSymbol(NV, &_nv, sizeof(int));
	d_vec_posScreen.resize(_nv);
	_d_vec_origPos.resize(_nv);
	_d_vec_lineIndexOrig.resize(_nv);
	_d_vec_lineIndex.resize(_nv);
	_d_vec_vertexIsFocus.resize(_nv);

	_d_vec_cutPointsMark.resize(_nv, 0);	//use 0 to mark the vertex that are not cut point	
	_d_vec_cutPointsMarkOrig.resize(_nv, 0);	//use 0 to mark the vertex that are not cut point	


	//_d_vec_IsCutPoint.resize(_nv);
	//_d_vec_IsCutPoint.assign(_nv, false);
	
	//make a copy
	thrust::copy(_d_vec_pos.begin(), _d_vec_pos.end(), _d_vec_origPos.begin());
}

void SetLens(VECTOR4* lensCenterObject)
{
	_lensCenterObject = (float4*)lensCenterObject;
}

void SetMode(DEFORM_MODE *deformMode, SOURCE_MODE *sourceMode)
{
	_deformMode = deformMode;
	_sourceMode = sourceMode;
}

void SetHull(std::vector<hull_type> *hullSet)
{
	if(hullSet->size() > MAX_NUM_GROUP)
	{
		cout<<"exceeded the maximum number of groups..."<<endl;
		exit(1);
	}
	//_hullSet = hullSet;
	d_vec_hullSet = *hullSet;
}

void SetEllipse(std::vector<ellipse> *ellipseSet)
{
	if(ellipseSet->size() > MAX_NUM_GROUP)
	{
		cout<<"exceeded the maximum number of groups..."<<endl;
		exit(1);
	}
	//_ellipseSet = ellipseSet;
	_h_vec_ellipseSet = ellipseSet;
}

void SetPickedLineSet(thrust::host_vector<int> *pickedLineSet)
{
	_pickedLineSet = pickedLineSet;
}

//void UpdateLineIndexFromOffsetLengths()
//{
//
//}

void SetConnectivity(thrust::host_vector<int> &length, thrust::host_vector<int> &offset)
{
	int numLines = length.size();
	d_vec_streamlineLengths.resize(numLines);
	d_vec_streamlineOffsets.resize(numLines);
	thrust::copy(length.begin(), length.end(), d_vec_streamlineLengths.begin());
	//d_vec_streamlineLengthsOrig = d_vec_streamlineLengths;
	//d_vec_streamlineOffsetsOrig = d_vec_streamlineOffsets;
	//cout<<"size of d_vec_streamlineLengths:"<<d_vec_streamlineLengths.size();
	thrust::exclusive_scan(d_vec_streamlineLengths.begin(), d_vec_streamlineLengths.end(), d_vec_streamlineOffsets.begin());
	//cout<<"size of offsets:"<<d_vec_streamlineOffsets.end() - d_vec_streamlineOffsets.begin()<<endl;
	offset = d_vec_streamlineOffsets;
	//for(int i = 0; i < h_vec_offset.size(); i++)
	//	offset.push_back(h_vec_offset[i]);
	d_vec_streamlineLengthsOrig = d_vec_streamlineLengths;
	d_vec_streamlineOffsetsOrig = d_vec_streamlineOffsets;
	
	//thrust::for_each(
	//	thrust::make_zip_iterator(thrust::make_tuple(d_vec_streamlineOffsets.begin(), d_vec_streamlineLengths.begin(), counting_zero)),
	//	thrust::make_zip_iterator(thrust::make_tuple(d_vec_streamlineOffsets.end(), d_vec_streamlineLengths.end(), counting_zero + d_vec_streamlineOffsets.size())),
	//	functor_GenLineIndex(thrust::raw_pointer_cast(_d_vec_lineIndexOrig.data())));
	UpdateLineIndex();
	_d_vec_lineIndexOrig = _d_vec_lineIndex;

	thrust::for_each(
		d_vec_streamlineOffsets.begin(),
		d_vec_streamlineOffsets.end(),
	functor_UpdateCutPointsMarkByConnectivity(
		thrust::raw_pointer_cast(_d_vec_cutPointsMarkOrig.data())
		));
	_d_vec_cutPointsMark = _d_vec_cutPointsMarkOrig;


	check_cuda_errors(__FILE__, __LINE__);
}

void GetConnectivity(thrust::host_vector<int> &offsets, thrust::host_vector<int> &lengths)
{
	lengths = d_vec_streamlineLengths;
	offsets = d_vec_streamlineOffsets;
}

void SetLineIndexCUDA(int *data)
{
	_d_vec_lineIndex.assign(data, data + _nv);
	//cout<<"_nv line index:" << _nv <<endl;
}

void SetDeformOnPara(bool *deformOn)
{
	 _deformOn = deformOn;
}

void AssignLineIndexFromDevice(int *data)
{
	//cout<<"AssignLineIndexFromDevice..."<<endl;
	thrust::copy(_d_vec_lineIndex.begin(), _d_vec_lineIndex.end(), data);
}

void SetParaCUDA(float para)
{
	_para = para;
}

void SetVBOData(float4* d_raw_clip, float3* d_raw_tangent, int* d_raw_translucent)
{
	_d_ptr_posClip = thrust::device_pointer_cast(d_raw_clip);
	_d_ptr_tangent = thrust::device_pointer_cast(d_raw_tangent);
	_d_ptr_translucent = thrust::device_pointer_cast(d_raw_translucent);
}

void SetDeformWinSize(int w, int h)
{
	cudaMemcpyToSymbol(WIN_WIDTH, &w, sizeof(int));
	cudaMemcpyToSymbol(WIN_HEIGHT, &h, sizeof(int));
	_h_winWidth = w;
	_h_winHeight = h;
}

void SetMatrix(	float* modelview, float* projection, float* invModelView, float* invProjection)
{
	cudaMemcpyToSymbol(MODELVIEW, modelview, 16 * sizeof(float));
	cudaMemcpyToSymbol(INVMODELVIEW, invModelView, 16 * sizeof(float));
	cudaMemcpyToSymbol(PROJECTION, projection, 16 * sizeof(float));
	cudaMemcpyToSymbol(INVPROJECTION, invProjection, 16 * sizeof(float));

	for(int i = 0; i < 16; i++)
	{
		_h_modelview[i] = modelview[i];
		_h_projection[i] = projection[i];
	}
	_invModelView = invModelView;
	_invProjection = invProjection;

}

thrust::host_vector<float2> GetPosScreenOrig()
{
	thrust::device_vector<float2> d_vec_origPosScreen(_nv);
	thrust::device_vector<float4> d_vec_origPosClip(_nv);
	thrust::transform(_d_vec_origPos.begin(), _d_vec_origPos.end(), d_vec_origPosClip.begin(), functor_Object2Clip());
	thrust::transform(d_vec_origPosClip.begin(), d_vec_origPosClip.end(), d_vec_origPosScreen.begin(), functor_Clip2Screen());

	thrust::host_vector<float2> h_vec_screen  = d_vec_origPosScreen;
	return h_vec_screen;
}


	//thrust::for_each(
	//	thrust::make_zip_iterator(thrust::make_tuple(d_vec_posScreen.begin(), _d_vec_vertexIsFocus.begin(), _d_ptr_translucent)),
	//	thrust::make_zip_iterator(thrust::make_tuple(d_vec_posScreen.end(), _d_vec_vertexIsFocus.end(), _d_ptr_translucent + _nv)),
	//	functor_UpdateTranslucentVertices(
	//		thrust::raw_pointer_cast(d_vec_ellipseSet.data()),
	//		_h_vec_ellipseSet->size())
	//		);

struct functor_UpdateTranslucentVertices
{
	ellipse* ellipseSet;
	int numEllipses;

	template <typename T>
	__device__ void operator() (T t)
	{
		float2 posScreen = thrust::get<0>(t);
		bool vertexIsFocus = thrust::get<1>(t);
		bool insideAnyBlades = false;
		for(int j = 0; j < numEllipses; j++)
		{
			//ellipse e = ellipseSet[j];
			//float2 line[2];
			//line[0] = make_float2(e.x - e.a * cos(e.angle), e.y - e.a * sin(e.angle));
			//line[1] = make_float2(e.x + e.a * cos(e.angle), e.y + e.a * sin(e.angle));
			//float2 center = ProjectPoint2Line(posScreen, line[0], line[1]);
			//float radius = radius_blade(ellipseSet[j], center);
		//	if(length(posScreen - tmp) < ellipseSet[j].a)//inOneBlade(ellipseSet[j], 0.5/*ellipseSet[j].b*/, posScreen))
		//	if(length(center - posScreen) < radius)
			if(inOneEllipse(ellipseSet[j], posScreen))
				insideAnyBlades = true;
		}
		if(insideAnyBlades && !vertexIsFocus)
			thrust::get<2>(t) = 1;
		else
			thrust::get<2>(t) = 0;
	}

	functor_UpdateTranslucentVertices(
		ellipse* _ellipseSet, int _numEllipses)
	{
		ellipseSet = _ellipseSet;
		numEllipses = _numEllipses;
	}
};


// Wrapper for the __global__ call that sets up the kernel call
void launch_kernel(clock_t t0)//, unsigned int mesh_width, unsigned int mesh_height, float time)
{
//	clock_t t0;
//#if (TEST_PERFORMANCE == 2)
////	t0 = clock();
//#endif
	_d_vec_prePos = _d_vec_pos;
	float4* d_raw_ptr_pos = thrust::raw_pointer_cast(_d_vec_pos.data());

    // execute the kernel
    dim3 block(256, 1, 1);
    dim3 grid(ceil((float)_nv / block.x), 1, 1);

	thrust::for_each(
		thrust::make_zip_iterator(thrust::make_tuple(counting_zero, _d_ptr_tangent)),
		thrust::make_zip_iterator(thrust::make_tuple(counting_zero + _nv, _d_ptr_tangent + _nv)),
		functor_computeNormal(thrust::raw_pointer_cast(_d_vec_lineIndex.data()), d_raw_ptr_pos));

	//clip coordiates of streamlines
	thrust::transform(_d_vec_pos.begin(), _d_vec_pos.end(), _d_ptr_posClip, functor_Object2Clip());
	thrust::transform(_d_ptr_posClip, _d_ptr_posClip + _nv, d_vec_posScreen.begin(), functor_Clip2Screen());

	if(0 == _h_vec_ellipseSet->size() )
		return;

	thrust::device_vector<ellipse> d_vec_ellipseSet = *_h_vec_ellipseSet;
	//cout<<"d_vec_ellipseSet.size():"<<d_vec_ellipseSet.size()<<endl;
	thrust::for_each(
		thrust::make_zip_iterator(thrust::make_tuple(d_vec_posScreen.begin(), _d_vec_vertexIsFocus.begin(), _d_ptr_translucent)),
		thrust::make_zip_iterator(thrust::make_tuple(d_vec_posScreen.end(), _d_vec_vertexIsFocus.end(), _d_ptr_translucent + _nv)),
		functor_UpdateTranslucentVertices(
			thrust::raw_pointer_cast(d_vec_ellipseSet.data()),
			d_vec_ellipseSet.size())
			);

#if (TEST_PERFORMANCE == 2)
	PrintElapsedTime(t0, "prepare data(before deformation kernel)");
#endif	
	if(*_deformOn)
	{
#if (TEST_PERFORMANCE == 3)
	    cudaEvent_t start, stop;
		float time;
		cudaEventCreate(&start);
		cudaEventCreate(&stop);

		cudaEventRecord(start, 0);
#endif
		//cout<<"_d_vec_lineIndex.back() before kernel:"<<_d_vec_lineIndex.back()<<endl;
   		kernel_convex<<< grid, block>>>(d_raw_ptr_pos, thrust::raw_pointer_cast(_d_ptr_posClip), 
					thrust::raw_pointer_cast(d_vec_posScreen.data()), thrust::raw_pointer_cast(_d_vec_prePos.data()), 
					thrust::raw_pointer_cast(_d_vec_origPos.data()), thrust::raw_pointer_cast(_d_vec_lineIndex.data()),
					thrust::raw_pointer_cast(_d_vec_vertexIsFocus.data()),
					_nv,
					thrust::raw_pointer_cast(d_vec_ellipseSet.data()), 
					thrust::raw_pointer_cast(d_vec_hullSet.data()), 
					_h_vec_ellipseSet->size(),
					*_deformMode,
					_para);
		 
#if (TEST_PERFORMANCE == 3)
		cudaEventRecord(stop, 0);
		cudaEventSynchronize(stop);

		cudaEventElapsedTime(&time, start, stop);
		printf("%f\tms to %s\n", time, "run deformation kernel");
#endif
		check_cuda_errors(__FILE__, __LINE__);
	}
}
