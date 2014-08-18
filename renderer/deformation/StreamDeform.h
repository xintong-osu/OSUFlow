#pragma once
#include "vector_types.h"
#include "DeformGPU.h"
#include <math.h>
#include <VectorMatrix.h>
#include "opengl.h"
#include <GL\GLU.h>
//#include <GL\GL.h>
//#include <map>
#include "Bundle.h"
//#include "GenerateStreamline.h"

#define COLOR2INDEX 256
#include "modes.h"


#include <CGAL/Cartesian.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Min_ellipse_2.h>
#include <CGAL/Min_ellipse_2_traits_2.h>
#include <CGAL/Gmpq.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Alpha_shape_2.h>

#include <CGAL/point_generators_3.h>
#include <CGAL/algorithm.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL\HalfedgeDS_decorator.h>
#include <CGAL/Delaunay_triangulation_3.h>
#include <CGAL/Alpha_shape_3.h>

#include <thrust\host_vector.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K>                     Polyhedron_3;
typedef CGAL::Alpha_shape_vertex_base_3<K>          Vb;
typedef CGAL::Alpha_shape_cell_base_3<K>            Fb;
typedef CGAL::Triangulation_data_structure_3<Vb,Fb>  Tds;
typedef CGAL::Delaunay_triangulation_3<K,Tds>       Dalaunay_Triangulation_3;
typedef CGAL::Alpha_shape_3<Dalaunay_Triangulation_3>         Alpha_shape_3;
typedef CGAL::Triangulation_3<K> Triangulation_3;
typedef CGAL::Tetrahedron_3<K> Tetrahedron_3;
typedef Alpha_shape_3::Alpha_shape_vertices_iterator Alpha_shape_vertices_iterator_3;
typedef Alpha_shape_3::Alpha_iterator               Alpha_iterator;
typedef K::Point_3                                  Point_3;
typedef K::Segment_3                              Segment_3;
typedef CGAL::Creator_uniform_3<double, Point_3>  PointCreator;

typedef  CGAL::Point_2<K>                 Point_2;
typedef  CGAL::Min_ellipse_2_traits_2<K>  Traits;
typedef  CGAL::Min_ellipse_2<Traits>      Min_ellipse_2;
typedef CGAL::Alpha_shape_vertex_base_2<K> Av;

typedef CGAL::Triangulation_face_base_2<K> Tf;
typedef CGAL::Alpha_shape_face_base_2<K,Tf> Af;

typedef CGAL::Triangulation_default_data_structure_2<K,Av,Af> Tds_2;
typedef CGAL::Delaunay_triangulation_2<K,Tds_2> Dt;
typedef CGAL::Alpha_shape_2<Dt> Alpha_shape_2;
typedef CGAL::Triangle_2<K> Triangle_2;
typedef CGAL::Triangulation_2<K> Triangulation_2;

using namespace std;


struct polyline{
//	int index;
	int vertMin;
	int vertMax;
	float depthMin;
	float depthMax;

	polyline(int vertex_min, int vertex_max, float depth_min, float depth_max)
		: vertMin(vertex_min), vertMax(vertex_max), depthMin(depth_min), depthMax(depth_max)
	{
	}

	polyline()
	{
	}
};

template <typename T>
struct point2
{
	T x;
	T y;

	point2(T a, T b): x(a), y(b)
	{
	}

	point2()
	{
	}

	point2<T> operator-(const point2& pt2) const
	{
		return point2<T>(x - pt2.x, y - pt2.y);
	}

	point2 operator+(const point2& pt2) const
	{
		return point2<T>(x + pt2.x, y + pt2.y);
	}

	point2 operator *(const float m)
	{
		return point2<T>(x * m, y * m);
	}

	point2 operator /(const float m)
	{
		return point2<T>(x / m, y / m);
	}

	float norm()
	{
		return sqrt((float)(x * x + y * y));
	}

	void normalize()
	{
		float m = norm();
		x /= m;
		y /= m;
	}

	T norm2()	//square of L2 norm
	{
		return (x * x + y * y);
	}


};

inline void Bouble2FloatMatrix(double matrix_double[16], float matrix_single[16])
{
	for(int i = 0; i < 16; i++)
		matrix_single[i] = matrix_double[i];
}

template <typename T>
struct point3
{
	T x;
	T y;
	T z;

	point3(T a, T b, T c): x(a), y(b), z(c)
	{
	}

	point3()
	{
	}

	point3 operator-(const point3& pt2) const
	{
		return point3<T>(x - pt2.x, y - pt2.y, z - pt2.z);
	}

	point3 operator+(const point3& pt2) const
	{
		return point3<T>(x + pt2.x, y + pt2.y, z + pt2.z);
	}

	point3 operator *(const float m)
	{
		return point3<T>(x * m, y * m, z * m);
	}

	point3 operator /(const float m)
	{
		return point3<T>(x / m, y / m, z / m);
	}

	float norm()
	{
		return sqrt((float)(x * x + y * y + z * z));
	}

	void normalize()
	{
		float m = norm();
		x /= m;
		y /= m;
		z /= m;
	}

	T norm2()	//square of L2 norm
	{
		return (x * x + y * y + z * z);
	}
};

class StreamDeform
{
	vector<VECTOR4*> _primitiveBases;
	thrust::host_vector<int> _primitiveLengths;
	thrust::host_vector<int> _primitiveOffsets;
	thrust::host_vector<int> _primitiveLengthsOrig;
	thrust::host_vector<int> _primitiveOffsetsOrig;
	vector<VECTOR4> _primitiveColors;

    float4 *_d_raw_clip;

	//void changeStreamline(VECTOR4* streamline, int size, int startingPoint, point2<int> mousePressPt, point2<int> mouseMovePt, int r2);
	GLdouble *_ModelViewMatrix;
	GLdouble *_ProjectionMatrix;
	float _invModelViewMatrixf[16];
	float _invProjectionMatrixf[16];
	int *_Viewport;
//	set<int> _movelineIndex;
	float* _VertexCopy;
	float* _VertexCopyPrev;
	int _vertexCount;
	float _thresh_depth;
	int *_vertexLineIndex;	//record the streamline index of each vertex


	//vector<int> h_vec_streamlineLengthsCut;
	//vector<int> h_vec_streamlineOffsetsCut;

	//float _ellipse_angle;
	//float _ellipse_center_x;
	//float _ellipse_center_y;
	//float _ellipse_size_width;
	//float _ellipse_size_height;
	
	bool _convex_2d;

	//3d convex hull of each bundle
	vector<vector<VECTOR4>> _bundleHull;

	DEFORM_MODE _deformMode;
	SOURCE_MODE _sourceMode;
	INTERACT_MODE _interactMode;

	bool bDragging;
	int _prevToX, _prevToY;

	struct cudaGraphicsResource *cuda_vbo_clip_resource;
	struct cudaGraphicsResource *cuda_vbo_tangent_resource;

	//streamline index and its vertex one step in the past
/*	vector<int> _prevLinesIndex;
	vector<vector<VECTOR4>> _prevLinesVertex;	*/	

	//void changeVertex(VECTOR4 vFrom, VECTOR4 &vTo, float x, float y);

	//void Object2Clips(
	//GLdouble  	objX,
 //	GLdouble  	objY,
 //	GLdouble  	objZ,
 //	GLdouble*  	winX,
 //	GLdouble*  	winY,
 //	GLdouble*  	winZ);

	//void getWorldCoords(
	//GLdouble  	winX,
 //	GLdouble  	winY,
 //	GLdouble  	winZ,
 //	GLdouble*  	objX,
 //	GLdouble*  	objY,
 //	GLdouble*  	objZ);

	//vector<VECTOR2> _GetExcludeRegion(vector<VECTOR2> points);

	void bundling(int n);

	vector<VECTOR3> _bundleColor;
	
	bundle *_bundle;

	//streamline index of each bundle
	vector<vector<int>> _streamBundles;

	void _LoadBundleFile(char *filename);



	//bundle index for each streamline
	int* _streamBundleIndex;
//	int* _streamBundleIndexArray;
	//vector<int> _streamBundleIndex;

	void getMainBundle();

	//the index of the majority bundle around the mouse point
	int _majBundle;

	//pick the streamlines in the majority bundle
	//void _PickBundle();

	void loadBundle(int *clusterid , int nClusters);


	void GenBundleColor();

	template<typename T>
	void GenSkeleton(vector<T> bundle);

	vector<VECTOR4>* GetPickedBundlePoints();

	float _para;

	vector<VECTOR3> _principalCurve;

	vector<DEFORM_MODE> _deformMode_bundle;

	int winWidth, winHeight;
	//int pickedBundleIndex;
	set<int> _pickedBundleSet;
	vector<int> _pickedLineSet;
	vector<int> _pickedLineSetOrig;
	vector<ellipse> _focusEllipseSet;
	vector<hull_type> _focusHullSet;

	//vector<vector<VECTOR4>> _focusPointSet;
	//vector<vector<VECTOR2>> _smoothConvexHull2DSet;
	vector<bool> _groupLikeCurve; 

	char* filename_vec;

	vector<vector<VECTOR4>>* _dynamicTraces;
	vector<VECTOR4> _tracePoints;
	/*void DecideConvex2D();*/

	template<typename T>
	vector<Point_2> Object2Screen(vector<T> bundleHull);
	VECTOR2 Object2Screen(VECTOR4 point);
	//void DecideConvex2D(vector<Point_2> convexHull2D, vector<Point_2> bundleProj);
	//vector<VECTOR2> GenSmoothHull(vector<Point_2> convexHull2D);
	//void GenFocusPointSet();

	VECTOR3 _pickBlockStart;
	VECTOR3 _pickBlockSize;
	float _pickBlockMoveStep;
	vector<vector<VECTOR4>> _picked3DHulls;

	int _numSampleOnBlock;
	int _streamLength;
	vector<VECTOR3>* _seeds;

	//lens
	float _biggestSize;
	//VECTOR4 _lens_center;
	//float _lens_radius;
	//float _lensEllipseRatio;
	//float _lensEllipseAngle;
	ellipse _ellLens;
	//float _lensDepth_clip;
	//VECTOR2 _lens_center_screen;
	//bool _onEllipseEndPt;
	bool _onLongAxisEndPt;
	float _lensChangeStep;


	float _pfMin[4];
	float _pfMax[4];
	VECTOR4 _lensCenterObject;

	bool _deformOn;	//turn on/off deformation
	bool _autoDeformMode;
	char *_filename_dist;

	void ClusterStreamByBoundingBox(vector<int> streamIndices, vector<vector<int>> &streamGroups);
	vector<vector<VECTOR4>> Groups2Hull(vector<vector<int>> streamGroups);
	//GenGroupsSamplePoint
	void ClusterStreamByBoundingBoxOnScreen(vector<int> streamIndices, vector<vector<int>> &streamGroups);

	float Object2ScreenLength(float length_object, VECTOR4 center_object);
	//float Screen2ObjectLength(float length_screen, float depth_screen);

	void GenSkeletonByLines(vector<int> lineIndex);
	void ComputeNewPrimitives();
	void UpdateVertexLineIndexForCut();

	//void CutStreamByCurve();

public:
	StreamDeform(void);
	~StreamDeform(void);

	void setDraggingStartPoint(int x, int y);
	void setData(float *pfCoords, int size, vector<int> pviGlPrimitiveBases, vector<int> pviGlPrimitiveLengths);
	void setMatrix(GLdouble *ModelViewMatrix, GLdouble *ProjectionMatrix, int *Viewport);
	void SetDistFileName(char* arg);

	//bool dragTo(int fromX, int fromY, int toX, int toY, float *depth, unsigned char *color);
	//void getDragSet(float *depth, unsigned char *color, int size);
	//void draw();
	//void drawPickWin(int x, int y, int width, int height);
	//void restoreVertex();
	void SetDeformMode(DEFORM_MODE mode);
	void SetInteractMode(INTERACT_MODE mode);
	INTERACT_MODE GetInteractMode();
	//void ReSetDeformMode(int i);

	//void restorePrevDeformedLines();
	//void dragEnd();
	//bool getMoveline(int i);
	//void drawPickedSeg(vector<int> pviGlPrimitiveBases);
	//PICK_MODE getPickMode();
	DEFORM_MODE GetDeformMode();
	//map<int, polyline> _pickedLineSeg;
	//vector<int> _pickedLine;
	//void setGlColor(int i);

	//void setStreamSource(list<list<VECTOR4*>*> stream);

	vector<VECTOR4*>* GetPrimitiveBases();
	vector<VECTOR4>* GetPrimitiveColors();
	vector<vector<int>>* getBundle();
	VECTOR3 GetBundleColor(int i);

	//int GetBundleIndexOfLine(int i);
	//vector<VECTOR4> GetBundleHull();
	vector<vector<VECTOR4>> _bundleSamplePoints;// hold all the points in each bundle
	//vector<vector<VECTOR4>> _bundlePointsVECTOR4;
	//vector<vector<VECTOR3>> _alphaShapes;

	//vector<VECTOR3>* GetAlphaShape(int ib);
	void SetPara(float p);
	void PickBundle(int ib);
	void AddRemoveBundle(int ib);
	void ProcessAfterBundleChanged();
	//vector<Point_3> GetBundlePoints();
	void RunCuda();
	void resetOrigPos();
	void Init();
	void BuildLineGroups();

	void SetCudaResourceClip(cudaGraphicsResource *_cuda_vbo_clip_resource);
	void SetCudaResourceTangent(cudaGraphicsResource *_cuda_vbo_tangent_resource);
	void SetWinSize(int _winWidth, int _winHeight);
	void GenHullEllipse();
	void SetDomain(float pfMin[4], float pfMax[4]);
	void SetDeformOn(bool b);
	void SetAutoDeformMode(bool b);

	void UpdateVertexLineIndex();
	//void UpdateBundle();
	vector<ellipse> GetEllipse();

	//void SetPickedBundleIndex(int i);

	//void GenStreamByBlock();
	void PickStreamByBlock();
	//inline ellipse GenEllipse(vector<Point_2> pointSet);
	void SetSourceMode(SOURCE_MODE mode);

	void MovePickBlock(DIRECTION dir);

	void ChangeLensDepth(int m);
	//void ChangeLensOnScreen(float x, float y);
	void MoveLensCenterOnScreen(float dx, float dy);
	void MoveLensEndPtOnScreen(float x, float y);

	void MoveLensTwoEndPtOnScreen(float x1, float y1, float x2, float y2);

	float* GetLensCenter();
	SOURCE_MODE GetSourceMode();
	void ChangeLensRadius(int m);
	void ChangeLensAngle(int m);
	void ChangeLensChangeStep(float m);
	

	void SetLensAxis(VECTOR2 startPoint, VECTOR2 endPoint);
	//void ChangeLensRatio(int m);
	bool InsideFirstEllipse(float x, float y);
	bool OnEllipseEndPoint(float x, float y);
	bool OnEllipseTwoEndPoints(float x1, float y1,  float x2, float y2);
	vector<VECTOR2> GetEllipseEndPoints();

	void GetPickCube(VECTOR3 &min, VECTOR3 &max);
	std::vector<hull_type>* GetHull();
	thrust::host_vector<int> GetPrimitiveLengths();
	thrust::host_vector<int> GetPrimitiveOffsets();
	//vector<int> GetPrimitiveLengthsRender();
	//vector<int> GetPrimitiveOffsetsRender();
	int* GetStreamBundleIndex();
	int GetVertexLineIndex(int i);
	bool GetLinePicked(int il);
	bool GetLinePickedOrig(int il);
	thrust::host_vector<bool> _isCutPoint;		//has to be public
	void RestoreStreamConnectivity();

	void LineLensProcess();
	void RedoDeformation();
	void ProcessCut();
	void RestoreAllStream();
	void ProcessAllStream();
	void GetModelViewMatrix(float matrix[16]);
	void GetProjectionMatrix(float matrix[16]);
	//void SetCutLine(VECTOR2 startPoint, VECTOR2 endPoint);
	void FinishDrag();
	void UpdateLensScreen();
};

