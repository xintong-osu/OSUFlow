#ifndef DEFORM_H
#define DEFORM_H

#include <stdlib.h>
#include <modes.h>
#include <vector>
#include <cuda_runtime_api.h>
#include <iostream>
#include <VectorMatrix.h>

#define HULL_SIZE 128
#define TEST_PERFORMANCE -1
//#define MAX_HULL_POINT_NUM 128

struct ellipse{
	float x;
	float y;
	float a;
	float b;
	float angle;
	ellipse(float _x, float _y, float _a, float _b, float _angle)
	{
		x = _x;
		y = _y;
		a = _a;
		b = _b;
		angle = _angle;
	}

	ellipse()
	{
	}
};

struct hull_type{
	float2 v[HULL_SIZE];   //vertex
	int nv;
	hull_type(float2* data, int cnt)
	{
		if(cnt > HULL_SIZE)
		{
			printf("hull size is too large!\n");
			exit(1);
			return;
		}
		nv = cnt;
		for(int i = 0; i < nv; i++)
		{
			v[i] = data[i];
		}
	}
};

void SetDeformWinSize(int w, int h);

void SetMatrix(	float* modelview, float* projection, float* invModelView, float* invProjection);

//void SetEllipse(float centerX, int centerY, float a, float b, float angle);
void SetEllipse(std::vector<ellipse> *ellipseSet);

//void SetIsCutPoint(std::vector<bool>* isCutPoint);

void SetPickedLineSet(std::vector<int> *pickedLineSet);

void SetVertexCoords(float* data, int n);

std::vector<bool> ComputeCutPoints();

//void SetHull(float* data, int nv, float center_x, float center_y);
void SetHull(std::vector<hull_type> *hullSet);

//void SetPrimitiveLength(std::vector<int> data);
void SetPrimitive(std::vector<int> data, std::vector<int> &offset);

void SetParaCUDA(float eps);

void SetVBOData(float4* d_raw_clip, float3* d_raw_tangent);

void SetLineIndexCUDA(int *data);

void SetLens(VECTOR4* lensCenterObject);

void SetDeformOnPara(bool *deformOn);

void launch_kernel(clock_t t0);

void LensTouchLine();

void SetMode(DEFORM_MODE *deformMode, SOURCE_MODE *sourceMode);

std::vector<VECTOR2> GetPosScreenOrig();

void AssignLineIndexFromDevice(int *data);

//void SetStreamDeform(void* sd);

//reset the vertex position to its original position
void resetPos();

std::vector<int> PickStreamByBlockCUDA(float min[3], float max[3]);

// This will output the proper CUDA error strings in the event that a CUDA host call returns an error
#define checkCudaErrors(err)           __checkCudaErrors (err, __FILE__, __LINE__)

inline void check_cuda_errors(const char *filename, const int line_number)
{
  cudaThreadSynchronize();
  cudaError_t error = cudaGetLastError();
  if(error != cudaSuccess)
  {
    printf("CUDA error at %s:%i: %s\n", filename, line_number, cudaGetErrorString(error));
    exit(-1);
  }
}

inline void __checkCudaErrors( cudaError err, const char *file, const int line )
{
    if( cudaSuccess != err) {
		fprintf(stderr, "%s(%i) : CUDA Runtime API error %d: %s.\n",
                file, line, (int)err, cudaGetErrorString( err ) );
        exit(-1);
    }
}

//
//// General GPU Device CUDA Initialization
//int gpuDeviceInit(int devID)
//{
//    int deviceCount;
//    checkCudaErrors(cudaGetDeviceCount(&deviceCount));
//
//    if (deviceCount == 0)
//    {
//        fprintf(stderr, "gpuDeviceInit() CUDA error: no devices supporting CUDA.\n");
//        exit(-1);
//    }
//
//    if (devID < 0)
//        devID = 0;
//            
//    if (devID > deviceCount-1)
//    {
//        fprintf(stderr, "\n");
//        fprintf(stderr, ">> %d CUDA capable GPU device(s) detected. <<\n", deviceCount);
//        fprintf(stderr, ">> gpuDeviceInit (-device=%d) is not a valid GPU device. <<\n", devID);
//        fprintf(stderr, "\n");
//        return -devID;
//    }
//
//    cudaDeviceProp deviceProp;
//    checkCudaErrors( cudaGetDeviceProperties(&deviceProp, devID) );
//
//    if (deviceProp.major < 1)
//    {
//        fprintf(stderr, "gpuDeviceInit(): GPU device does not support CUDA.\n");
//        exit(-1);                                                  
//    }
//        
//    checkCudaErrors( cudaSetDevice(devID) );
//    printf("gpuDeviceInit() CUDA Device [%d]: \"%s\n", devID, deviceProp.name);
//
//    return devID;
//}


template <typename T>
inline void PrintMatrix(T m[16])
{
	cout<<m[0]<<","<<m[4]<<","<<m[8]<<","<<m[12]<<endl
		<<m[1]<<","<<m[5]<<","<<m[9]<<","<<m[13]<<endl
		<<m[2]<<","<<m[6]<<","<<m[10]<<","<<m[14]<<endl
		<<m[3]<<","<<m[7]<<","<<m[11]<<","<<m[15]<<endl;
}

inline void PrintElapsedTime(clock_t t0, char* msg)
{
    clock_t t = clock();
    clock_t compute_time = (t - t0) * 1000 / CLOCKS_PER_SEC;
    printf("%f\tms to %s\n", (double)compute_time , msg);
}

#endif