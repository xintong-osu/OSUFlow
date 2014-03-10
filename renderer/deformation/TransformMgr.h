#ifndef TRANSFORM_MGR_H
#define TRANSFORM_MGR_H
#include "VectorMatrix.h"
//#include "cutil_math.h"


__device__ __host__ inline VECTOR2 GetXY(VECTOR4 pos)
{
	return VECTOR2(pos[0], pos[1]);
}

template <typename T>
__device__ __host__ 
inline float4 mat4mulvec4(T *a, float4 b)
{
	float4 c;
	c.x = a[0] * b.x + a[4] * b.y + a[8] * b.z + a[12] * b.w;
	c.y = a[1] * b.x + a[5] * b.y + a[9] * b.z + a[13] * b.w;
	c.z = a[2] * b.x + a[6] * b.y + a[10] * b.z + a[14] * b.w;
	c.w = a[3] * b.x + a[7] * b.y + a[11] * b.z + a[15] * b.w;
	return c;
}

template <typename T>
__device__ __host__ 
inline VECTOR4 mat4mulvec4(T *a, VECTOR4 b)
{
	VECTOR4 c;
	c[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
	c[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
	c[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
	c[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
	return c;
}

//Object space-->Camera space-->Clip space-->Screen space
//multiply projection and modelview matrix

//only multiply projection matrix
template <typename T>
__device__ __host__ 
inline float4 Camera2Clip(float4 pos, T* projection)
{
	float4 pos2;
	float4 v_screen = mat4mulvec4(projection, pos);//projection * modelview * v;
	pos2.x = v_screen.x / v_screen.w;
	pos2.y = v_screen.y / v_screen.w;
	pos2.z = v_screen.z / v_screen.w;
	pos2.w = 1.0;
	return pos2;
}

template <typename T>
__device__ __host__ 
inline VECTOR4 Camera2Clip(VECTOR4 pos, T* projection)
{
	VECTOR4 pos2;
	VECTOR4 v_screen = mat4mulvec4(projection, pos);//projection * modelview * v;
	pos2[0] = v_screen[0] / v_screen[3];
	pos2[1] = v_screen[1] / v_screen[3];
	pos2[2] = v_screen[2] / v_screen[3];
	pos2[3] = 1.0;
	return pos2;
}


//multiply modelview matrix
template <typename T>
__device__ __host__ inline float4 Object2Camera(float4 pos, T* modelView)//, float modelview[16], float projection[16])
{
	float4 pos2;
	float4 v_screen = mat4mulvec4(modelView, pos);//projection * modelview * v;
	pos2.x = v_screen.x / v_screen.w;
	pos2.y = v_screen.y / v_screen.w;
	pos2.z = v_screen.z / v_screen.w;
	pos2.w = 1.0;
	return pos2;
}

//multiply modelview matrix
template <typename T>
__device__ __host__ inline VECTOR4 Object2Camera(VECTOR4 pos, T* modelView)//, float modelview[16], float projection[16])
{
	VECTOR4 pos2;
	VECTOR4 v_screen = mat4mulvec4(modelView, pos);//projection * modelview * v;
	pos2[0] = v_screen[0] / v_screen[3];
	pos2[1] = v_screen[1] / v_screen[3];
	pos2[2] = v_screen[2] / v_screen[3];
	pos2[3] = 1.0;
	//return v_screen;
	return pos2;
}

//multiply modelview matrix
template <typename T>
__device__ __host__ inline VECTOR4 Camera2Object(VECTOR4 pos, T* invModelView)//, float modelview[16], float projection[16])
{
	VECTOR4 pos2;
	VECTOR4 v_screen = mat4mulvec4(invModelView, pos);//projection * modelview * v;
	pos2[0] = v_screen[0] / v_screen[3];
	pos2[1] = v_screen[1] / v_screen[3];
	pos2[2] = v_screen[2] / v_screen[3];
	pos2[3] = 1.0;
	//return v_screen;
	return pos2;
}

template <typename T>
__device__ inline float4 Clip2Object(float4 p, T* invModelView, T* invProjection)//, float modelview[16], float projection[16])
{
	p = mat4mulvec4(invModelView, mat4mulvec4(invProjection, p));
	p.x /= p.w;
	p.y /= p.w;
	p.z /= p.w;
	p.w = 1.0;
	return p;
}

template <typename T>
__device__ inline VECTOR4 Clip2Object(VECTOR4 p, T* invModelView, T* invProjection)//, float modelview[16], float projection[16])
{
	p = mat4mulvec4(invModelView, mat4mulvec4(invProjection, p));
	p[0] /= p[3];
	p[1] /= p[3];
	p[2] /= p[3];
	p[3] = 1.0;
	return p;
}

template <typename T>
__device__ inline VECTOR4 Clip2Camera(VECTOR4 p, T* invProjection)//, float modelview[16], float projection[16])
{
	p = mat4mulvec4(invProjection, p);
	p[0] /= p[3];
	p[1] /= p[3];
	p[2] /= p[3];
	p[3] = 1.0;
	return p;
}

template <typename T>
__device__ __host__ inline float4 Object2Clip(float4 pos, T* modelView, T* projection)//, float modelview[16], float projection[16])
{
	float4 pos_clip;
	float4 v_screen = mat4mulvec4(projection, mat4mulvec4(modelView, pos));//projection * modelview * v;
	pos_clip.x = v_screen.x / v_screen.w;
	pos_clip.y = v_screen.y / v_screen.w;
	pos_clip.z = v_screen.z / v_screen.w;
	pos_clip.w = 1.0;
	return pos_clip;
}

template <typename T>
__device__ __host__ inline VECTOR4 Object2Clip(VECTOR4 pos, T* modelView, T* projection)//, float modelview[16], float projection[16])
{
	VECTOR4 pos_clip;
	VECTOR4 v_screen = mat4mulvec4(projection, mat4mulvec4(modelView, pos));//projection * modelview * v;
	pos_clip[0] = v_screen[0] / v_screen[3];
	pos_clip[1] = v_screen[1] / v_screen[3];
	pos_clip[2] = v_screen[2] / v_screen[3];
	pos_clip[3] = 1.0;
	return pos_clip;
}

__device__ __host__ inline float2 Clip2Screen(float2 p, float winWidth, float winHeight)
{
	float2 p2;
	p2.x = (p.x + 1) * winWidth / 2.0;
	p2.y = (p.y + 1) * winHeight / 2.0;
	return p2;
}

__device__ __host__ inline VECTOR2 Clip2Screen(VECTOR2 p, float winWidth, float winHeight)
{
	VECTOR2 p2;
	p2[0] = (p[0] + 1) * winWidth / 2.0;
	p2[1] = (p[1] + 1) * winHeight / 2.0;
	return p2;
}

__device__ __host__ inline float2 Screen2Clip(float2 p, float winWidth, float winHeight)
{
	float2 p2;
	p2.x = p.x / winWidth * 2.0 - 1.0;
	p2.y = p.y / winHeight * 2.0 - 1.0;
	return p2;
}

__device__ __host__ inline VECTOR2 Screen2Clip(VECTOR2 p, float winWidth, float winHeight)
{
	VECTOR2 p2;
	p2[0] = p[0] / winWidth * 2.0 - 1.0;
	p2[1] = p[1] / winHeight * 2.0 - 1.0;
	return p2;
}

#endif //TRANSFORM_MGR_H