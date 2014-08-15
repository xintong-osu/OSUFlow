#include "StreamDeform.h"

#include <fstream>
#include "BSPLINE.h"
#include "ColorPalette.h"
#include <math.h>

#include <list>
//#include <DataMgr.h>
#include "Ellipse.h"
#include "TransformMgr.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



#define NUM_BUNDLE 36


#define SAMPLE_NUM 200
#define SAMPLE_RATE 5

template <typename T>
inline bool invertMatrix(T m[16], T invOut[16])
{
    double inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

inline void RefineHull(vector<VECTOR3> &convexHull2D, float generalSize)
{
	vector<VECTOR3> newHull;
	int size = convexHull2D.size();
	//cout<<"generalSize:"<<generalSize<<endl;
	float threshold_max = generalSize * 0.1;
	float threshold_min = threshold_max * 0.5;
	newHull.push_back(convexHull2D[0]);
	for(int i = 1; i <= size; i++)
	{
		VECTOR3 diff = convexHull2D[i % size] - newHull.back();
		while(diff.GetMag() > threshold_max)
		{
		//	cout<<diff.GetMag() <<endl;
			VECTOR3 dir = diff * (1 / diff.GetMag());
			newHull.push_back(newHull.back() + dir * threshold_max);
			diff = convexHull2D[i % size] - newHull.back();
		}
		if(diff.GetMag() > threshold_min && i != size)
			newHull.push_back(convexHull2D[i]);
		//diff = convexHull2D[(i + 1) % size] - newHull.back();
		//if(i < (size - 1) && diff.GetMag() > (0.5 * generalSize))
		//	newHull.push_back(convexHull2D[(i + 1) % size]);
	}
	convexHull2D = newHull;
}

inline void SmoothByBspline(vector<VECTOR3> &principalCurve)
{
	int count = principalCurve.size();
	VECTOR3 *out_pts = new VECTOR3[count];
	bspline(principalCurve.size() - 1, 4, (point*)principalCurve.data(), (point*)out_pts, count);

	principalCurve.clear();
	for(int i =0; i < count; i++)
	{
		principalCurve.push_back(out_pts[i]);
	}
	delete [] out_pts;
}

inline void SmoothByBspline(vector<VECTOR4> &principalCurve4)
{
	int count = principalCurve4.size();

	vector<VECTOR3> principalCurve;
	for(int i = 0; i < principalCurve4.size(); i++)
		principalCurve.push_back(VECTOR3(principalCurve4[i][0], principalCurve4[i][1], principalCurve4[i][2]));

	VECTOR3 *out_pts = new VECTOR3[count];
	bspline(principalCurve.size() - 1, 4, (point*)principalCurve.data(), (point*)out_pts, count);

	principalCurve.clear();
	for(int i =0; i < count; i++)
	{
		principalCurve4.push_back(VECTOR4(out_pts[i][0], out_pts[i][1], out_pts[i][2], 1));
	}
	delete [] out_pts;
}

inline vector<VECTOR2> GenSmoothHull(vector<Point_2> convexHull2D, float generalSize)
{
	vector<VECTOR3> smoothHull;
	for(int i = 0; i < convexHull2D.size(); i++)
		smoothHull.push_back(VECTOR3(convexHull2D[i][0], convexHull2D[i][1], 0));
	RefineHull(smoothHull,generalSize);
	//InterpolateHull(smoothHull,generalSize);
	
	/*
	smoothHull.push_back(VECTOR3(convexHull2D[0][0], convexHull2D[0][1], 0));
	SmoothByBspline(smoothHull);
	smoothHull.pop_back();
	*/
	//_hullSmooth.clear();
	
	vector<VECTOR2> hullSmooth2D;
	for(int i = 0; i < smoothHull.size(); i++)
		hullSmooth2D.push_back(VECTOR2(smoothHull[i][0], smoothHull[i][1]));
	//if( hullSmooth2D.size() > MAX_HULL_POINT_NUM)
	//{
	//	cout<<"The number of vertices in the hull exceeded the limit..."<<endl;
	//	exit(1);
	//}
	//cout<<"hullSmooth2D.size():"<<hullSmooth2D.size()<<endl;
	return hullSmooth2D;
}

inline ellipse GenEllipse(vector<Point_2> pointSet)
{
//	cout<<"pointSet:"<<pointSet.size()<<endl;
	Min_ellipse_2  me(pointSet.begin(), pointSet.end(), true);
	Ellipse_2<K> e(me);
	float ellipse_angle = std::atan2( e.va().y(), e.va().x() );// (atan2(B, A - C) + M_PI) * 0.5;
	float ellipse_center_x =	e.center().x();//me.ellipse().;//	- DD / AA * 0.5;
	float ellipse_center_y =	e.center().y();//	- EE / CC * 0.5;
	//float tmp = - 4 * FF * AA * CC + CC * DD * DD + AA * EE * EE;
	float ellipse_size_a = sqrt(e.va() * e.va());//	2 * sqrt(tmp / (4 * AA * CC * CC));
	float ellipse_size_b =	sqrt(e.vb() * e.vb());//2 * sqrt(tmp / (4 * AA * AA * CC));
	return ellipse(ellipse_center_x, ellipse_center_y, ellipse_size_a, ellipse_size_b, ellipse_angle);
}

void StreamDeform::setMatrix(GLdouble *ModelViewMatrix, GLdouble *ProjectionMatrix, int *Viewport)
{
	_ModelViewMatrix = ModelViewMatrix;
	_ProjectionMatrix = ProjectionMatrix;
	_Viewport = Viewport;
}

void StreamDeform::SetDistFileName(char* arg)
{
	_filename_dist = arg;
}

void StreamDeform::GenBundleColor()
{
	for(int i = 0; i < _streamBundles.size(); i++)
		_bundleColor.push_back(*(new VECTOR3(getR(i), getG(i), getB(i))));
}

VECTOR3 StreamDeform::GetBundleColor(int i)
{
	return _bundleColor[i];
}

StreamDeform::StreamDeform(void)
{
	_deformMode = DEFORM_MODE::MODE_ELLIPSE;
	_sourceMode = SOURCE_MODE::MODE_BUNDLE;
	_VertexCopy = NULL;
	_VertexCopyPrev = NULL;

	_pickBlockStart = VECTOR3(0, 0, 0);
	_pickBlockSize = VECTOR3(4, 4, 4);
	_pickBlockMoveStep = 4;
	_autoDeformMode = true;
	_lensChangeStep = 0.0002;

	SetMode(&_deformMode, &_sourceMode);
	SetPickedLineSet(&_pickedLineSet);
	//SetStreamDeform(this);
	_deformOn = true;
	_interactMode = INTERACT_MODE::TRANSFORMATION;
	SetDeformOnPara(&_deformOn);
}

vector<ellipse> StreamDeform::GetEllipse()
{
	return _focusEllipseSet;
}

StreamDeform::~StreamDeform(void)
{
}

void StreamDeform::SetDeformMode(DEFORM_MODE mode)
{
	DEFORM_MODE preMode = _deformMode;
	_deformMode = mode;
	//RestoreStreamConnectivity();
	//if(_deformMode == DEFORM_MODE::MODE_LINE)
	//	resetOrigPos();
	//redo deformation only when the new mode is different
	if(preMode != _deformMode)		
		RedoDeformation();
}

void StreamDeform::SetInteractMode(INTERACT_MODE mode)
{
	_interactMode = mode;
}

INTERACT_MODE StreamDeform::GetInteractMode()
{
	return _interactMode;
}

void StreamDeform::SetSourceMode(SOURCE_MODE mode)
{
	_sourceMode = mode;
	if(_sourceMode == SOURCE_MODE::MODE_LENS)
	{
	//	resetPos();
		_pickedLineSetOrig.clear();
		//_focusEllipseSet.clear();
		if(_focusEllipseSet.size() == 0)
			_focusEllipseSet.push_back(_ellLens);
	

	//	RestoreStreamConnectivity();
	}
	else if(_sourceMode == SOURCE_MODE::MODE_LOCATION)	{
		_deformMode = DEFORM_MODE::MODE_ELLIPSE;
	}
	
	RedoDeformation();
}

void StreamDeform::RedoDeformation()
{
	//RestoreStreamConnectivity();
	//resetOrigPos();
	RestoreAllStream();
	ProcessAllStream();
}

void StreamDeform::FinishDrag()
{
	//if(_deformMode == DEFORM_MODE::MODE_LINE)
		RedoDeformation();
}

SOURCE_MODE StreamDeform::GetSourceMode()
{
	return _sourceMode;
}

DEFORM_MODE StreamDeform::GetDeformMode()
{
	return _deformMode;
}

vector<vector<int>>* StreamDeform::getBundle()
{
	return &_streamBundles;
}

void StreamDeform::bundling(int n)
{
	//_bundle.HierarchicalClustering();
	//_bundle._SetData(_primitiveBases, _primitiveLengths);
	//_bundle.HierarchicalClustering();
	//_bundle._SaveBundleFile(FILENAME_BUNDLE);
	clock_t t0 = clock();
	_bundle = new bundle();
	_bundle->getDist(_filename_dist);
	//_bundle->getDist(filename_emd_dist);
	_bundle->clustering();
	_bundle->getClusters(n);
#if (TEST_PERFORMANCE == 6)
	PrintElapsedTime(t0, "Clustering");
#endif
	loadBundle(_bundle->getClusterId(), n);

	GenBundleColor();
}


//called by the rendering program
void StreamDeform::Init()
{
	SetVertexCoords((float*)(GetPrimitiveBases()->at(0)), _vertexCount );
	SetPrimitive(_primitiveLengths, _primitiveOffsets);
	_primitiveOffsetsOrig = _primitiveOffsets;
	
	if(_vertexLineIndex != NULL)
		delete [] _vertexLineIndex;
	_vertexLineIndex = new int[_vertexCount];

	//////set color/////////
	for(int i = 0; i < _primitiveOffsetsOrig.size(); i++)
	{
		int offset = _primitiveOffsetsOrig[i];
		int length = _primitiveLengthsOrig[i];
		for(int j = 0; j < length; j++)
		{
			VECTOR4 c = GetBundleColor(_streamBundleIndex[i]);// GetColor((float)j / (length - 1));
			_primitiveColors.push_back(c);
		}
	}
	SetEllipse(&_focusEllipseSet);
}

void StreamDeform::SetPara(float p)
{
	_para = p;
	SetParaCUDA(p);
}

int StreamDeform::GetVertexLineIndex(int i)
{
	return _vertexLineIndex[i];
}

bool StreamDeform::GetLinePicked(int il)
{
	bool picked = false;
	for(int j = 0; j < _pickedLineSet.size(); j++)
	{
		if(_pickedLineSet[j] == il)
			picked = true;
	}
	return picked;
}

bool StreamDeform::GetLinePickedOrig(int il)
{
	bool picked = false;
	for(int j = 0; j < _pickedLineSetOrig.size(); j++)
	{
		if(_pickedLineSetOrig[j] == il)
			picked = true;
	}
	return picked;
}

void StreamDeform::SetCudaResourceClip(cudaGraphicsResource *_cuda_vbo_clip_resource)
{
	cuda_vbo_clip_resource = _cuda_vbo_clip_resource;
}

void StreamDeform::SetCudaResourceTangent(cudaGraphicsResource *_cuda_vbo_tangent_resource)
{
	cuda_vbo_tangent_resource = _cuda_vbo_tangent_resource;
}

void StreamDeform::SetWinSize(int _winWidth, int _winHeight)
{
	winWidth = _winWidth;
	winHeight = _winHeight;

	_ellLens.a = (winWidth + winHeight )* 0.04;
	_ellLens.b = _ellLens.a;
	_ellLens.angle = 0;
	_ellLens.x = winWidth * 0.5;
	_ellLens.y = winHeight * 0.5;
}

void StreamDeform::SetAutoDeformMode(bool b)
{
	_autoDeformMode = b;
}

VECTOR2 StreamDeform::Object2Screen(VECTOR4 point)
{
	GLdouble winX, winY, winZ;
	gluProject((double)point[0], (double)point[1], (double)point[2],
		_ModelViewMatrix, _ProjectionMatrix, _Viewport, 
		&winX, &winY, &winZ);
	return VECTOR2(winX, winY);
}


template<typename T>
vector<Point_2> StreamDeform::Object2Screen(vector<T> pointSet)
{
	vector<Point_2> pointSetProj;
	for(int i = 0; i < pointSet.size(); i++)
	{
		GLdouble winX, winY, winZ;
		gluProject((double)pointSet[i][0], (double)pointSet[i][1], (double)pointSet[i][2],
			_ModelViewMatrix, _ProjectionMatrix, _Viewport, 
			&winX, &winY, &winZ);
		pointSetProj.push_back(*(new Point_2(winX, winY)));
	}
	return pointSetProj;
}

/*******lens***********/
bool StreamDeform::InsideFirstEllipse(float x, float y)
{
	VECTOR2 lens_center_screen = VECTOR2(_focusEllipseSet.front().x, _focusEllipseSet.front().y);
	VECTOR2 dir2Center = VECTOR2(x, y) - lens_center_screen;	//current distance to the center
	float dist2Center = (dir2Center).GetMag();	//current distance to the center 
	float t = atan2(dir2Center[1], dir2Center[0]);
	if(t < 0)
		t += 2 * M_PI; 

	//rotate to canonical position
	t = t - _focusEllipseSet.front().angle;	
	float radius = _focusEllipseSet.front().a * _focusEllipseSet.front().b 
		/ sqrt(pow(_focusEllipseSet.front().b * cos(t),2) + pow(_focusEllipseSet.front().a * sin(t),2));
	return dist2Center < 0.8 * radius;
}

vector<VECTOR2> StreamDeform::GetEllipseEndPoints()
{
	float angle = _focusEllipseSet[0].angle;//atan2(y - _focusEllipseSet.front().y, x - _focusEllipseSet.front().x) + M_PI;
	vector<VECTOR2> endPoints;
	for(float t = 0; t < M_PI * 2; t += (M_PI * 0.5))
	{
		float x = _focusEllipseSet[0].x + _focusEllipseSet[0].a * cos(t) * cos(angle) - _focusEllipseSet[0].b * sin(t) * sin(angle);
		float y = _focusEllipseSet[0].y + _focusEllipseSet[0].a * cos(t) * sin(angle) + _focusEllipseSet[0].b * sin(t) * cos(angle);
		endPoints.push_back(VECTOR2(x,y));
	}
	return endPoints;
}

inline bool PointsClose(VECTOR2 v1, VECTOR2 v2)
{
	return (v1 - v2).GetMag() < 32.0;
}

bool StreamDeform::OnEllipseTwoEndPoints(float x1, float y1,  float x2, float y2)
{
	bool onEllipseEndPt;
	vector<VECTOR2> endPoints = GetEllipseEndPoints();
	if(		(PointsClose(VECTOR2(x1, y1), endPoints[0]) && PointsClose(VECTOR2(x2, y2), endPoints[2]))
		||	(PointsClose(VECTOR2(x2, y2), endPoints[0]) && PointsClose(VECTOR2(x1, y1), endPoints[2])))
	{
		onEllipseEndPt = true;
		_onLongAxisEndPt = true;
	}	else if
		(	(PointsClose(VECTOR2(x1, y1), endPoints[1]) && PointsClose(VECTOR2(x2, y2), endPoints[3]))
		||	(PointsClose(VECTOR2(x2, y2), endPoints[1]) && PointsClose(VECTOR2(x1, y1), endPoints[3])))
	{
		onEllipseEndPt = true;
		_onLongAxisEndPt = false;
	}
	return onEllipseEndPt;
}

bool StreamDeform::OnEllipseEndPoint(float x, float y)
{
	bool onEllipseEndPt;
	vector<VECTOR2> endPoints = GetEllipseEndPoints();
	onEllipseEndPt = false;
	for(int i = 0; i < endPoints.size(); i++)
	{
		if(PointsClose(VECTOR2(x, y), endPoints[i]))
		{
			onEllipseEndPt = true;
			if(i == 0 || i == 2)
				_onLongAxisEndPt = true;
			else
				_onLongAxisEndPt = false;
			break;
		}
	}
	return onEllipseEndPt;
}

void StreamDeform::ChangeLensDepth(int m)
{
	//float step = 0.0002;

	//should be Object2Camera, but not Object2Clip
	//cout<<"_lensChangeStep:"<<_lensChangeStep<<endl;
	VECTOR4 lens_center_clip = Object2Clip(_lensCenterObject, _ModelViewMatrix, _ProjectionMatrix);
	if(m > 0)
		lens_center_clip[2] += _lensChangeStep;
	else
		lens_center_clip[2] -= _lensChangeStep;
	_lensCenterObject = Clip2Object(lens_center_clip, _invModelViewMatrixf, _invProjectionMatrixf);
	//SetEllipse(&_focusEllipseSet);
	ProcessAllStream();
}

void StreamDeform::MoveLensCenterOnScreen(float dx, float dy)
{
	//VECTOR2 center_clip_2 = Screen2Clip(VECTOR2(_lens_center_screen[0] + dx, _lens_center_screen[1] + dy), winWidth, winHeight);
	//VECTOR4 lens_center_clip = VECTOR4(center_clip_2[0], center_clip_2[1], _lensDepth_clip, 1);
	//_lens_center = Clip2Object(lens_center_clip, _invModelViewMatrixf, _invProjectionMatrixf);


	VECTOR4 lens_center_clip = Object2Clip(_lensCenterObject, _ModelViewMatrix, _ProjectionMatrix);

	VECTOR2 lensCenterScreen = Clip2Screen(lens_center_clip, winWidth, winHeight);

	lensCenterScreen[0] += dx;
	lensCenterScreen[1] += dy;

	_focusEllipseSet.front().x = lensCenterScreen[0];
	_focusEllipseSet.front().y = lensCenterScreen[1];

	VECTOR2 lensCenterClipXY = Screen2Clip(lensCenterScreen, winWidth, winHeight);

	VECTOR4 lensCenterClipNew(lensCenterClipXY[0], lensCenterClipXY[1], lens_center_clip[2], 1);

	_lensCenterObject = Clip2Object(lensCenterClipNew, _invModelViewMatrixf, _invProjectionMatrixf);

	//ProcessAllStream();
}

void StreamDeform::UpdateLensScreen()
{
	if(_sourceMode != SOURCE_MODE::MODE_LENS)
		return;

	VECTOR2 lensCenterScreen = Object2Screen(_lensCenterObject);

	_focusEllipseSet.front().x = lensCenterScreen[0];
	_focusEllipseSet.front().y = lensCenterScreen[1];
}

void StreamDeform::MoveLensEndPtOnScreen(float x, float y)
{
	VECTOR2 lens_center_screen = VECTOR2(_focusEllipseSet.front().x, _focusEllipseSet.front().y);
	VECTOR2 dirFromCenter = VECTOR2(x, y) - lens_center_screen;
	float angle = atan2(dirFromCenter[1], dirFromCenter[0]);
	float length = dirFromCenter.GetMag();// Screen2ObjectLength(dirFromCenter.GetMag(), _lensDepth_clip);

	if(_onLongAxisEndPt)
	{
		//_lensEllipseRatio = dirFromCenter.GetMag() / _focusEllipseSet[0].b;
		_focusEllipseSet.front().a = length;
	}
	else
	{
		_focusEllipseSet.front().b = length;
		angle += (M_PI * 0.5);
	}
	if(angle < 0)
		angle += M_PI;
	_focusEllipseSet.front().angle = angle ;
	//ProcessAllStream();
}

void StreamDeform::MoveLensTwoEndPtOnScreen(float x1, float y1, float x2, float y2)
{
	VECTOR2 lens_center_screen = ( VECTOR2(x1, y1) + VECTOR2(x2, y2) ) * 0.5;
	VECTOR2 dirFromCenter = VECTOR2(x1, y1) - lens_center_screen;
	float angle = atan2(dirFromCenter[1], dirFromCenter[0]);
	float length = dirFromCenter.GetMag();// Screen2ObjectLength(dirFromCenter.GetMag(), _lensDepth_clip);

	if(_onLongAxisEndPt)
	{
		//_lensEllipseRatio = dirFromCenter.GetMag() / _focusEllipseSet[0].b;
		_focusEllipseSet.front().a = length;
	}
	else
	{
		_focusEllipseSet.front().b = length;
		angle += (M_PI * 0.5);
	}
	if(angle < 0)
		angle += M_PI;
	_focusEllipseSet.front().angle = angle ;
	//ProcessAllStream();
}

//void StreamDeform::ChangeLensOnScreen(float x, float y)
//{
//	VECTOR4 lens_center_clip = Object2Clip(_lens_center, _ModelViewMatrix, _ProjectionMatrix);
//	lens_center_clip[0] += x;
//	lens_center_clip[1] += y;
//
//	_lens_center = Clip2Object(lens_center_clip, _invModelViewMatrixf, _invProjectionMatrixf);
//	LensTouchLine();
//}

void StreamDeform::ChangeLensAngle(int m)
{
	_focusEllipseSet.front().angle += (m * 0.2);
	LensTouchLine();
}

void StreamDeform::ChangeLensChangeStep(float m)
{
	_lensChangeStep *= m;
}
//
//void StreamDeform::ChangeLensRatio(int m)
//{
//	_lensEllipseRatio += (m * 0.1);
//	LensTouchLine();
//}

float* StreamDeform::GetLensCenter()
{
	return &_lensCenterObject[0];
}

void StreamDeform::GenHullEllipse()
{
	_focusEllipseSet.clear();
	_focusHullSet.clear();
	if(0 == _picked3DHulls.size() )
		return;

	vector<vector<Point_2>> convexHull2DSet;
	
	//for(int i = 0; i < _focusPointSet.size(); i++)
	for(int i = 0; i < _picked3DHulls.size(); i++)
	{
		vector<Point_2> bundleHullProj;
		bundleHullProj = Object2Screen<VECTOR4>(_picked3DHulls[i]);

		vector<Point_2> convexHull2D;
		CGAL::convex_hull_2(bundleHullProj.begin(), bundleHullProj.end(), std::back_inserter(convexHull2D));

		convexHull2DSet.push_back(convexHull2D);
		_focusEllipseSet.push_back(GenEllipse(convexHull2D));

		//vector<VECTOR2> smoothHull = GenSmoothHull(convexHull2DSet[i]);
		//hull_type hullCUDA((float2*)&smoothHull[0], smoothHull.size());
		//_focusHullSet.push_back(hullCUDA);
	}

	if(_deformMode == DEFORM_MODE::MODE_LINE)	{
	}
	else if(_deformMode == DEFORM_MODE::MODE_HULL)	//refine convexhull
	{
		//_smoothConvexHull2DSet.clear();
		for(int i = 0; i < convexHull2DSet.size(); i++)
		{
			vector<VECTOR2> smoothHull = GenSmoothHull(convexHull2DSet[i], (_focusEllipseSet[i].a + _focusEllipseSet[i].b) * 0.5);
			hull_type hullCUDA((float2*)&smoothHull[0], smoothHull.size());
			_focusHullSet.push_back(hullCUDA);
			//_smoothConvexHull2DSet.push_back();
		}
	}
}


float StreamDeform::Object2ScreenLength(float length_object, VECTOR4 center_object)
{
	//VECTOR4 center_object = VECTOR4(0,0,depth_object,0); 
	VECTOR4 center_camera = Object2Camera(center_object, _ModelViewMatrix);
	VECTOR4 center_clip = Camera2Clip(center_camera, _ProjectionMatrix);
	VECTOR2 center_screen = Clip2Screen(GetXY(center_clip), winWidth, winHeight);
	VECTOR4 s_object = center_object + VECTOR4(length_object, 0, 0, 0);//an arbitrary point on sphere
	VECTOR4 s_camera = Object2Camera(s_object, _ModelViewMatrix);
	float dist_camera = (s_camera - center_camera).GetMag(); 
	VECTOR4 s1_camera = center_camera +  VECTOR4(dist_camera, 0, 0, 0);
	VECTOR2 s1_screen = Clip2Screen(GetXY(Camera2Clip(s1_camera, _ProjectionMatrix)), winWidth, winHeight);
	float length_screen = (s1_screen - center_screen).GetMag();
	return length_screen;
}
//
//float StreamDeform::Screen2ObjectLength(float length_screen, float depth_screen)
//{
//	VECTOR4 lens_center_camera = Object2Camera(_lens_center, _ModelViewMatrix);
//	VECTOR4 lens_center_clip = Camera2Clip(lens_center_camera, _ProjectionMatrix);
//
//	//VECTOR4 center_object = VECTOR4(0,0,depth_object,0); 
//	VECTOR2 center_clip_2 = Screen2Clip(VECTOR2(0,0), winWidth, winHeight);
//	VECTOR4 center_clip = VECTOR4(center_clip_2[0], center_clip_2[1], lens_center_clip[2], 1);
//	//VECTOR4 center_camera = Clip2Camera(center_clip, _invProjectionMatrixf);
//	VECTOR4 center_object = Clip2Object(center_clip, _invModelViewMatrixf, _invProjectionMatrixf);
//	
//	VECTOR2 s_clip_2 = Screen2Clip(VECTOR2(length_screen,0), winWidth, winHeight);
//	VECTOR4 s_clip = VECTOR4(s_clip_2[0], s_clip_2[1], lens_center_clip[2], 1);
//	//VECTOR4 s_camera = Clip2Camera(s_clip, _invProjectionMatrixf);
//	VECTOR4 s_object = Clip2Object(s_clip, _invModelViewMatrixf, _invProjectionMatrixf);
//
//	//float length_camera = (center_camera - s_camera).GetMag();
//	float length_camera = (center_object - s_object).GetMag();
//	return length_camera;
//}

void StreamDeform::RunCuda()
{
	clock_t t0 = 0;
#if (TEST_PERFORMANCE == 2)
	t0 = clock();
#endif
	if(_sourceMode == SOURCE_MODE::MODE_BUNDLE || _sourceMode == SOURCE_MODE::MODE_LOCATION)
		GenHullEllipse();
	
	//for single bundle, automatically decide the deformation mode
	//if((_sourceMode == SOURCE_MODE::MODE_BUNDLE || _sourceMode == SOURCE_MODE::MODE_LOCATION) && _autoDeformMode)// || _sourceMode == SOURCE_MODE::MODE_DYNAMIC_TRACE)
	if(_sourceMode == SOURCE_MODE::MODE_BUNDLE && _autoDeformMode)// || _sourceMode == SOURCE_MODE::MODE_DYNAMIC_TRACE)
	{
		//need to change!!!!!
		if(_sourceMode == SOURCE_MODE::MODE_LOCATION)
			return;
		if(_focusEllipseSet.size() == 1)
		{
			ellipse ell = _focusEllipseSet.front();
			if(ell.a / ell.b > 3)
			{
				if(_deformMode != DEFORM_MODE::MODE_LINE)
				{
					_deformMode = DEFORM_MODE::MODE_LINE;
					RedoDeformation();
				}
			}
			else
			{
				if(_deformMode != DEFORM_MODE::MODE_ELLIPSE)
				{
					_deformMode = DEFORM_MODE::MODE_ELLIPSE;
					RedoDeformation();
				}
			}
		}
	}

	// map OpenGL buffer object for writing from CUDA
	float3 *d_raw_tangent;
    size_t num_bytes; 

    checkCudaErrors(cudaGraphicsMapResources(1, &cuda_vbo_clip_resource, 0));
    checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&_d_raw_clip, &num_bytes,  
						       cuda_vbo_clip_resource));

	checkCudaErrors(cudaGraphicsMapResources(1, &cuda_vbo_tangent_resource, 0));
    checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_raw_tangent, &num_bytes,  
						       cuda_vbo_tangent_resource));

	check_cuda_errors(__FILE__, __LINE__);

//#if TEST_PERFORMANCE
//	clock_t t1 = clock();
//	unsigned long compute_time = (t1 - t0) * 1000 / CLOCKS_PER_SEC;
//	cout<<"Map cuda resource time:"<< (float)compute_time * 0.001 << "sec" << endl;
//#endif

	//set matrix and window size
	SetDeformWinSize(winWidth, winHeight);//can be moved to somewhere else
	float tModelViewMatrixf[16];
	float tProjectionMatrixf[16];
	Bouble2FloatMatrix(_ModelViewMatrix, tModelViewMatrixf);
	Bouble2FloatMatrix(_ProjectionMatrix, tProjectionMatrixf);

	invertMatrix(tModelViewMatrixf, _invModelViewMatrixf);
	invertMatrix(tProjectionMatrixf, _invProjectionMatrixf);


	SetMatrix(tModelViewMatrixf, tProjectionMatrixf, _invModelViewMatrixf, _invProjectionMatrixf);

	//SetEllipse(&_focusEllipseSet);
	SetHull(&_focusHullSet);
	SetVBOData(_d_raw_clip, d_raw_tangent);

	if(_vertexCount > 0)
		launch_kernel(t0);

//#if (TEST_PERFORMANCE == 4)
//	t0 = clock();
//#endif
    // unmap buffer object
    checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_vbo_clip_resource, 0));
    checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_vbo_tangent_resource, 0));
//#if (TEST_PERFORMANCE == 4)
//	PrintElapsedTime(t0, "Unmap cuda resource time");
//#endif
}

void StreamDeform::LineLensProcess()
{
	_isCutPoint = ComputeCutPoints();
	ComputeNewPrimitives();
	AssignLineIndexFromDevice(_vertexLineIndex);
	UpdateVertexLineIndexForCut();
}

void StreamDeform::GetModelViewMatrix(float matrix[16])
{
	float tModelViewMatrixf[16];
	Bouble2FloatMatrix(_ModelViewMatrix, tModelViewMatrixf);
	for(int i = 0; i < 16; i++)
		matrix[i] = tModelViewMatrixf[i];
}

void StreamDeform::GetProjectionMatrix(float matrix[16])
{
	float tProjectionMatrixf[16];
	Bouble2FloatMatrix(_ProjectionMatrix, tProjectionMatrixf);
	for(int i = 0; i < 16; i++)
		matrix[i] = tProjectionMatrixf[i];
}

void StreamDeform::ProcessCut()
{
	if(_picked3DHulls.size() > 0)
	{
		GenHullEllipse();
		_isCutPoint = ComputeCutPoints();
		ComputeNewPrimitives();
		UpdateVertexLineIndex();
	}
}

void StreamDeform::resetOrigPos()
{
	resetPos();

	//GenSkeletonByLines(streamGroups.front());


}

void StreamDeform::ProcessAllStream()
{
#if (TEST_PERFORMANCE == 1)
	clock_t t0 = clock();
#endif
	if(SOURCE_MODE::MODE_LENS == _sourceMode)
		LensTouchLine();
	else if(SOURCE_MODE::MODE_BUNDLE == _sourceMode)
		BuildLineGroups();
	else if(SOURCE_MODE::MODE_LOCATION == _sourceMode)
		PickStreamByBlock();

#if (TEST_PERFORMANCE == 1)
	PrintElapsedTime(t0, "Process streamlines");
#endif

#if (TEST_PERFORMANCE == 4)
	clock_t t0 = clock();
#endif
	if(_deformMode == DEFORM_MODE::MODE_LINE)
	{	
		if(SOURCE_MODE::MODE_LENS == _sourceMode)
			LineLensProcess();
		else if(SOURCE_MODE::MODE_BUNDLE == _sourceMode)
			ProcessCut();
		else if(SOURCE_MODE::MODE_LOCATION == _sourceMode)
			LineLensProcess();
	}
#if (TEST_PERFORMANCE == 4)
	PrintElapsedTime(t0, "cut");
#endif
}

std::vector<hull_type>* StreamDeform::GetHull()
{
	return &_focusHullSet;
}

//the index of target streamlines is negative
//the index of other streamlines is positive
//call it whenever the target changes
void StreamDeform::UpdateVertexLineIndex()
{
	int iv = 0;
	vector<int> lengths = _primitiveLengths;// GetPrimitiveLengthsRender();
	for(int i = 0; i < lengths.size(); i++ )
	{
		iv = _primitiveOffsets[i];
		bool picked = false;
		for(int j = 0; j < _pickedLineSet.size(); j++)
		{
			if(_pickedLineSet[j] == i)
				picked = true;
		}
			
		if(picked)
		{
			for(int j = 0; j < lengths[i]; j++)
				_vertexLineIndex[iv++] = -i;
		}
		else
		{
			for(int j = 0; j < lengths[i]; j++)
				_vertexLineIndex[iv++] = i;
		}
	}
	//cout<<"**set vertex line index..."<<endl;
	//for(int i = 0; i < _vertexCount; i++)
	//	cout<<_vertexLineIndex[i]<<"\t";
	SetLineIndexCUDA(_vertexLineIndex);
//	SetPickedLineCUDA(&_pickedLineSet[0], _pickedLineSet.size());
}

void StreamDeform::UpdateVertexLineIndexForCut()
{
	int iv = 0;
	vector<int> lengths = _primitiveLengths;// GetPrimitiveLengthsRender();
	for(int i = 0; i < lengths.size(); i++ )
	{
		iv = _primitiveOffsets[i];
		if(_vertexLineIndex[iv] < 0)
			continue;
		
		for(int j = 0; j < lengths[i]; j++)
			_vertexLineIndex[iv++] = i;
	}
	SetLineIndexCUDA(_vertexLineIndex);
//	SetPickedLineCUDA(&_pickedLineSet[0], _pickedLineSet.size());
}

void StreamDeform::PickBundle(int ib)
{
	if(_sourceMode != SOURCE_MODE::MODE_BUNDLE)
		return;
	if(ib < 0)
		return;

	_autoDeformMode = true;

	_pickedBundleSet.clear();
	_pickedBundleSet.insert(ib);

	//we still need to restore before cutting line
	//because we need the focus streamlines to be the original shape to analize its convex hull
	RestoreAllStream();
	ProcessAfterBundleChanged();
}

void StreamDeform::AddRemoveBundle(int ib)
{
	if(_sourceMode != SOURCE_MODE::MODE_BUNDLE)
		return;
	if(ib < 0)
		return;
	//have to reset original position, because the newly added bundle is deformed
	//however, the original position is needed for deciding groups
	//RestoreAllStream();
	std::set<int>::iterator it;
	it = _pickedBundleSet.find(ib);
	if(it == _pickedBundleSet.end())
		_pickedBundleSet.insert(ib);
	else
		_pickedBundleSet.erase(it);

	ProcessAfterBundleChanged();
}

void StreamDeform::ProcessAfterBundleChanged()
{
	_pickedLineSet.clear();
	for(set<int>::iterator it = _pickedBundleSet.begin(); it != _pickedBundleSet.end(); ++it)
		_pickedLineSet.insert(_pickedLineSet.end(), _streamBundles[*it].begin(), _streamBundles[*it].end());

	_pickedLineSetOrig = _pickedLineSet;
	UpdateVertexLineIndex();
	ProcessAllStream();
}



void StreamDeform::SetLensAxis(VECTOR2 startPoint, VECTOR2 endPoint)
{
	VECTOR2 centerPoint = (startPoint + endPoint) * 0.5;
	ellipse ellLens;
	VECTOR2 dir = startPoint - endPoint;
	ellLens.x = centerPoint[0];
	ellLens.y = centerPoint[1];
	ellLens.a = dir.GetMag() * 0.5;
	ellLens.b = ellLens.a * 0.3;
	ellLens.angle = atan2( dir[1], dir[0]);
	_focusEllipseSet.clear();
	_focusEllipseSet.push_back(ellLens);
//	SetEllipse(&_focusEllipseSet);

	RestoreAllStream();
	ProcessAllStream();
}

struct box3
{
	vector<int> lineIdx;
	float min[3];
	float max[3];

	void print()
	{
		cout<<min[0]<<","<<min[1]<<","<<min[2]<<";"<<max[0]<<","<<max[1]<<","<<max[2]<<endl;
	}
};

struct box2
{
	vector<int> lineIdx;
	float min[2];
	float max[2];

	void print()
	{
		cout<<min[0]<<","<<min[1]<<";"<<max[0]<<","<<max[1]<<endl;
	}
};

inline box2 GenBox2(int idx, vector<VECTOR2> line)
{
	box2 b;
	for(int i = 0; i < 2; i++)
	{
		b.min[i] = FLT_MAX;
		b.max[i] = -FLT_MAX;
	}
	for(int i = 0; i < line.size(); i++)
	{
		for(int j = 0; j < 2; j++)
		{
			if(line[i][j] < b.min[j])
				b.min[j] = line[i][j];
			if(line[i][j] > b.max[j])
				b.max[j] = line[i][j];
		}
	}
	b.lineIdx.push_back(idx);
	return b;
}

inline bool BoxOverlap(box3 b1, box3 b2, float threshold)
{
	//float threshold = _biggestSize * 0.05;
	for(int i = 0; i < 3; i++)
		if((b2.min[i] - b1.max[i]) > threshold || (b1.min[i] - b2.max[i]) > threshold)
			return false;
	return true;
}

inline bool BoxOverlap(box2 b1, box2 b2, float threshold[2])
{
	//float threshold = _biggestSize * 0.05;
	for(int i = 0; i < 2; i++)
		if((b2.min[i] - b1.max[i]) > threshold[i] || (b1.min[i] - b2.max[i]) > threshold[i])
			return false;
	return true;
}

inline bool BoxOverlap(box2 b1, box2 b2)
{
	//float threshold = _biggestSize * 0.05;
	for(int i = 0; i < 2; i++)
		if((b2.min[i] - b1.max[i]) > 0 || (b1.min[i] - b2.max[i]) > 0)
			return false;
	return true;
}

inline void MergeBox2ToBox1(box2 &b1, box2 &b2)
{
	b1.lineIdx.insert(b1.lineIdx.end(), b2.lineIdx.begin(), b2.lineIdx.end());
	for(int i = 0; i < 2; i++)
	{
		b1.min[i] = min(b1.min[i], b2.min[i]);
		b1.max[i] = max(b1.max[i], b2.max[i]);
	}
}

void StreamDeform::ClusterStreamByBoundingBoxOnScreen(vector<int> streamIndices, vector<vector<int>> &streamGroups)
{
    size_t num_bytes; 
    //checkCudaErrors(cudaGraphicsMapResources(1, &cuda_vbo_clip_resource, 0));
    //checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&_d_raw_clip, &num_bytes,  
				//		       cuda_vbo_clip_resource));

	//cout<<"**0"<<endl;
//	thrust::copy(d_ptr_clip, d_ptr_clip + _vertexCount, vec_clip.begin());

	//cout<<"**1"<<endl;
	list<box2> groupBoxes;

	std::vector<VECTOR2> pos = GetPosScreenOrig();

	for(int i = 0; i < streamIndices.size(); i++)
	{
		int idx = streamIndices[i];
		//vector<VECTOR4> line(_primitiveBases[idx], _primitiveBases[idx] + _primitiveLengths[idx]);
	//	vector<VECTOR4> line(vec_clip.begin() + _primitiveOffsets[i], vec_clip.begin() + _primitiveOffsets[i] + _primitiveLengths[i]);
		vector<VECTOR2> line;
		for(int j = 0; j < _primitiveLengths[idx]; j++)
			line.push_back(pos[_primitiveOffsets[idx] + j]);
		//box3 tmp = ;
		//tmp.print();
		groupBoxes.push_back(GenBox2(idx, line));
	}
	
	//for(list<box2>::iterator it = groupBoxes.begin(); it !=groupBoxes.end(); ++it)
	//{
	//	cout<<(*it).min[0]<<","<<(*it).max[0]<<","<<(*it).min[1]<<","<<(*it).max[1]<<endl;
	//}
	//cout<<"**3"<<endl;
	//cout<<"groupBoxes.size():"<<groupBoxes.size()<<endl;
	int preSize = groupBoxes.size() + 1;
	float threshold[2];// = 0.0;
	while(preSize > groupBoxes.size())
	{
		//for(int i = 0; i < groupBoxes.size(); i++)
		//	for(int j = i + 1; j < groupBoxes.size(); j++)
		preSize = groupBoxes.size();
		for(list<box2>::iterator it = groupBoxes.begin(); it !=groupBoxes.end(); ++it)
		{
			bool flagBreak = false;
			list<box2>::iterator jt = it;
			for(++jt; jt !=groupBoxes.end(); ++jt)
			{
				threshold[0] = ((it->max[0] - it->min[0]) + (jt->max[0] - jt->min[0])) * 0.3;
				threshold[1] = ((it->max[1] - it->min[1]) + (jt->max[1] - jt->min[1])) * 0.3;
				if(BoxOverlap(*it, *jt, threshold))
				{
					//it->print();
					//jt->print();
					//cout<<"next"<<endl;;
					MergeBox2ToBox1(*it, *jt);
					groupBoxes.erase(jt);
					flagBreak = true;
					break;
				}
			}
			if(flagBreak)
				break;
		}
	}
	//cout<<"groupBoxes.size()2:"<<groupBoxes.size()<<endl;
/*
	cout<<"after grouping:"<<endl;
	for(list<box2>::iterator it = groupBoxes.begin(); it !=groupBoxes.end(); ++it)
	{
		cout<<(*it).min[0]<<","<<(*it).max[0]<<","<<(*it).min[1]<<","<<(*it).max[1]<<endl;
	}*/
	for(list<box2>::iterator it = groupBoxes.begin(); it !=groupBoxes.end(); ++it)
		streamGroups.push_back(it->lineIdx);
}

vector<int> StreamDeform::GetPrimitiveLengths()
{
	return _primitiveLengths;
}

thrust::host_vector<int> StreamDeform::GetPrimitiveOffsets()
{
	return _primitiveOffsets;
}

inline float Distance(Point_3 p1, VECTOR4 p2)
{
	return sqrt(
			pow((double)(p1.x() - p2[0]), 2) 
		+	pow((double)(p1.y() - p2[1]), 2)
		+	pow((double)(p1.z() - p2[2]), 2));
}

inline float Distance(Point_3 p, vector<VECTOR4> curve)
{
	float minDistance = FLT_MAX;
	for(int i = 0; i < curve.size(); i++)
	{
		float dist = Distance(p, curve[i]);
		if(dist < minDistance)
			minDistance = dist;
	}
	return minDistance;
}

vector<vector<VECTOR4>> StreamDeform::Groups2Hull(vector<vector<int>> streamGroups)
{
	vector<vector<VECTOR4>> hulls;
	vector<vector<Point_3>> groupSamplePoints;
	vector<vector<VECTOR4>> focusPointSet;
	_groupLikeCurve.clear();
	//cout<<"streamGroups.size():"<<streamGroups.size()<<endl;
	for(int ig = 0; ig < streamGroups.size(); ig++)
	{
		vector<VECTOR4> focusPoints;
		vector<VECTOR4> convexhull3D;
		vector<Point_3> bundleSamplePoints;
		for(int il = 0; il < streamGroups[ig].size(); il++)
		{
			int idx = streamGroups[ig][il];
			for(int ip = 0; ip < _primitiveLengths[idx]; ip+=SAMPLE_RATE)
			{
				Point_3 p(
				_primitiveBases[idx][ip][0], _primitiveBases[idx][ip][1], _primitiveBases[idx][ip][2]);
				bundleSamplePoints.push_back(p);
				focusPoints.push_back(_primitiveBases[idx][ip]);
			}
			//.insert(focusPoints.end(), _primitiveBases[idx], _primitiveBases[idx] +
		}
		groupSamplePoints.push_back(bundleSamplePoints);
		focusPointSet.push_back(focusPoints);
		/************/
		/************/

		Polyhedron_3 poly;
		CGAL::convex_hull_3(bundleSamplePoints.begin(), bundleSamplePoints.end(), poly);
		//_groupLikeCurve.push_back(IsLikeCurve(poly, bundleSamplePoints));
		for (Polyhedron_3::Vertex_const_iterator  it = poly.vertices_begin(); it != poly.vertices_end(); it++)
			convexhull3D.push_back(VECTOR4(it->point().x(), it->point().y(), it->point().z(),1.0));
		hulls.push_back(convexhull3D);
	}
	return hulls;
}

void StreamDeform::GetPickCube(VECTOR3 &min, VECTOR3 &max)
{
	min = _pickBlockStart;
	max = _pickBlockStart + _pickBlockSize;
}
void StreamDeform::ComputeNewPrimitives()
{
	//cout<<"_primitiveLengths.size()1:"<<_primitiveLengths.size()<<endl;
	_primitiveLengths.clear();
	_primitiveOffsets.clear();
	_pickedLineSet.clear();
	for(int i = 0; i < _primitiveLengthsOrig.size(); i++)
	{
		int length = _primitiveLengthsOrig[i];
		int offset = _primitiveOffsetsOrig[i];
		int offsetNew = offset;
		bool isFocus = GetLinePickedOrig(i);

		int lengthNew = 0;
		//const int lengthThreshold = 4;
		for(int j = 0; j < (length - 1); j++)
		{
			int idx = offset + j;
			if(		_isCutPoint[idx] )
			{
				//cout<<"_isCutPoint!  j="<<j<<endl;
				lengthNew = idx - offsetNew + 1;
				//if(lengthNew > lengthThreshold)
				{
					_primitiveOffsets.push_back(offsetNew);
					_primitiveLengths.push_back(lengthNew);
					//cout<<"offsetNew:"<<offsetNew<<endl;
					//cout<<"lengthNew:"<<lengthNew<<endl;
					if(isFocus)
						_pickedLineSet.push_back(_primitiveOffsets.size() - 1);
				}
				offsetNew = idx + 1;
			}
		}
		lengthNew = length - (offsetNew - offset);
		_primitiveOffsets.push_back(offsetNew);
		_primitiveLengths.push_back(lengthNew);

		if(isFocus)
			_pickedLineSet.push_back(_primitiveOffsets.size() - 1);
	}
	//cout<<"_primitiveLengths.size()2:"<<_primitiveLengths.size()<<endl;
	//_primitiveLengths = _primitiveLengthsOrig;
	//_primitiveOffsets = _primitiveOffsetsOrig;
	//_pickedLineSet = _pickedLineSetOrig;
}

void StreamDeform::BuildLineGroups()
{
	vector<vector<int>> streamGroups;
	ClusterStreamByBoundingBoxOnScreen(_pickedLineSet, streamGroups);
	_picked3DHulls = Groups2Hull(streamGroups);
	
}

void StreamDeform::RestoreAllStream()
{
	resetOrigPos();
	RestoreStreamConnectivity();
}

void StreamDeform::RestoreStreamConnectivity()
{
	_primitiveLengths = _primitiveLengthsOrig;
	_primitiveOffsets = _primitiveOffsetsOrig;
	_pickedLineSet = _pickedLineSetOrig;
	UpdateVertexLineIndex();
}

void StreamDeform::loadBundle(int *clusterid , int nClusters)
{
	//	_streamBundleIndex = new int[_primitiveLengths.size()];
	//	for(int i = 0; i < _primitiveLengths.size(); i++)
	//		_streamBundleIndex[i] = clusterid[i];
	_streamBundleIndex = clusterid;
	_streamBundles = vector<vector<int>>(nClusters,*(new vector<int>));
	for(int i = 0; i < _primitiveLengths.size(); i++)
	{
		_streamBundles[_streamBundleIndex[i]].push_back(i);
	}
	//	free(_bundle);
}

void StreamDeform::setData(float *pfCoords, int size, vector<int> pviGlPrimitiveBases, vector<int> pviGlPrimitiveLengths)
{
	for(int i = 0; i < pviGlPrimitiveBases.size(); i++)
	{
		_primitiveBases.push_back((VECTOR4*)(&pfCoords[pviGlPrimitiveBases.at(i) * 4]));
	}
	_primitiveLengths = pviGlPrimitiveLengths;
	_primitiveLengthsOrig = _primitiveLengths;

	_vertexCount = size;

	cout<<"Total number of vertices:"<<_vertexCount <<endl;
	cout<<"Total number of streamlines:"<<pviGlPrimitiveBases.size()<<endl;

	//keep a copy of the coordinates of the vertices in order to be able to restore them
	if(_VertexCopy != NULL)
		delete [] _VertexCopy;
	_VertexCopy = new float[_vertexCount * 4];

	if(_VertexCopyPrev != NULL)
		delete [] _VertexCopyPrev;
	_VertexCopyPrev = new float[_vertexCount * 4];

	memcpy(_VertexCopy, pfCoords, _vertexCount * 4 * sizeof(float));
	bundling(NUM_BUNDLE);



	//	_LoadBundleFile(FILENAME_BUNDLE);
}

void StreamDeform::SetDomain(float pfMin[4], float pfMax[4])
{
	for(int i = 0; i < 4; i++)	{
		_pfMin[i] = pfMin[i];
		_pfMax[i] = pfMax[i];
	}
	float volumeSize[4];
	_pickBlockStart = VECTOR3((pfMin[0] + pfMax[0]) * 0.5, (pfMin[1] + pfMax[1]) * 0.5, (pfMin[2] + pfMax[2]) * 0.5);
	float size = ((pfMax[0] - pfMin[0]) + (pfMax[1] - pfMin[1]) + (pfMax[2] - pfMin[2])) * 0.02;
	_pickBlockSize = VECTOR3(size, size, size);
	_pickBlockMoveStep = size * 0.5;

	//VECTOR4 dataCenterObject, dataCenterClip;
	//_lens_radius = _biggestSize * 0.1;
	//_lensEllipseRatio = 1.0;
	//_lensEllipseAngle = 0;
	for(int i = 0; i < 4; i++)
		_lensCenterObject[i] = (pfMin[i] + pfMax[i]) * 0.5;
	_lensCenterObject[3] = 1;
	//_lensDepth_clip =  -1;
	SetLens(&_lensCenterObject);
}

void StreamDeform::SetDeformOn(bool b)
{
	_deformOn = b;
}

void StreamDeform::_LoadBundleFile(char *filename)
{
	ifstream ifs;
	ifs.open(filename);
	if(!ifs.is_open()) {
		cout << "cannot open file: " << filename << endl;
	}
	vector<int> oneBundle;
	int i;
	while(!ifs.eof())
	{
		ifs >> i;
		oneBundle.push_back(i);
		ifs.get();
		char c = ifs.get();
		if(c == '\n')
		{
			_streamBundles.push_back(oneBundle);
			oneBundle.clear();
		}
		ifs.putback(c);
	}
	_streamBundleIndex = new int[_primitiveLengths.size()];
	for(int i = 0; i < _streamBundles.size(); i++)
	{
		for(int j = 0; j < _streamBundles[i].size(); j++)
		{
			_streamBundleIndex[_streamBundles[i][j]] = i;
		}
	}
}

vector<VECTOR4*>* StreamDeform::GetPrimitiveBases()
{
	return &_primitiveBases;
}

vector<VECTOR4>* StreamDeform::GetPrimitiveColors()
{
	return &_primitiveColors;
}


//void StreamDeform::ChangeLensRadius(int m)
//{
//	if(m > 0)
//		_lens_radius *= 1.25;
//	else
//		_lens_radius *= 0.8;
//}

void StreamDeform::PickStreamByBlock()
{
	_pickedLineSet = PickStreamByBlockCUDA(&_pickBlockStart[0], &(_pickBlockStart + _pickBlockSize)[0]);
	vector<vector<int>> streamGroups;
	streamGroups.push_back(_pickedLineSet);
	//ClusterStreamByBoundingBoxOnScreen(_pickedLineSet, streamGroups);
	//cout<<"streamGroups.size():"<<streamGroups.size()<<endl;
	_picked3DHulls = Groups2Hull(streamGroups);
	UpdateVertexLineIndex();
}

void StreamDeform::MovePickBlock(DIRECTION dir)
{
	VECTOR3 minLen;
	VECTOR3 maxLen;
	//_genStream.GetSpaceDomain(minLen, maxLen);
	float tmp;
	switch(dir)
	{
	case DIRECTION::DIR_LEFT:
		_pickBlockStart[0] -= _pickBlockMoveStep;
		if(_pickBlockStart[0] < _pfMin[0])
		{
			_pickBlockStart[0] += _pickBlockMoveStep;
			return;
		}
		break;
	case DIRECTION::DIR_RIGHT:
		_pickBlockStart[0] += _pickBlockMoveStep;
		if(_pickBlockStart[0] > (_pfMax[0] - _pickBlockSize[0]))
		{
			_pickBlockStart[0] -= _pickBlockMoveStep;
			return;
		}
		break;
	case DIRECTION::DIR_DOWN:
		_pickBlockStart[1] -= _pickBlockMoveStep;
		if(_pickBlockStart[1] < _pfMin[1])
		{
			_pickBlockStart[1] += _pickBlockMoveStep;
			return;
		}
		break;
	case DIRECTION::DIR_UP:
		_pickBlockStart[1] += _pickBlockMoveStep;
		if(_pickBlockStart[1] > (_pfMax[1] - _pickBlockSize[1]))
		{
			_pickBlockStart[1] -= _pickBlockMoveStep;
			return;
		}
		break;
	case DIRECTION::DIR_IN:
		_pickBlockStart[2] -= _pickBlockMoveStep;
		if(_pickBlockStart[2] < _pfMin[2])
		{
			_pickBlockStart[2] += _pickBlockMoveStep;
			return;
		}
		break;
	case DIRECTION::DIR_OUT:
		_pickBlockStart[2] += _pickBlockMoveStep;
		if(_pickBlockStart[2] > (_pfMax[2] - _pickBlockSize[2]))
		{
			_pickBlockStart[2] -= _pickBlockMoveStep;
			return;
		}
		break;
	}

		//cout<<"dir:"<<dir<<endl;
	//GenStreamByBlock();
	//RestoreAllStream();
	ProcessAllStream();
}

int* StreamDeform::GetStreamBundleIndex()
{
	return _streamBundleIndex;
}
//
//void StreamDeform::ClearEllipse()
//{
//
//}
//void StreamDeform::SetCutLine(VECTOR2 startPoint, VECTOR2 endPoint)
//{
//	//_cutLine[0] = startPoint;
//	//_cutLine[1] = endPoint;
//}
