#ifndef SPHERICAL_CURSOR_H
#define SPHERICAL_CURSOR_H
#include "vector_types.h"
#include "vector_functions.h"
#include "Leap.h"
#include "vector"
#include <string>
#include <ctime>

enum TRANSFORM_MODE
{
	TRANSLATE,
	ROTATE,
	SCALE,
	IDLE,
};

Leap::Vector RelativePalm3DLoc(Leap::Frame frame);

class SphericalCursor
{
public:
	void SetCenter(float x, float y, float z);
	void SetCenter(float3 v);
	void SetRadiusIn(float v);
	void SetRadiusOut(float v);
	float GetRadiusIn();
	float GetRadiusOut();
	Leap::Vector GetCenter();
	void Draw();
	void DrawFingersInSphere();
	void FingerInput(float3 v0, float3 v1, float3 v2, float3 v3, float3 v4);
	void FingerInput(Leap::Frame frame);
	void ComputeFingerInSphere(const Leap::Frame frame);
	void Translate(const Leap::Frame frame);
	void Rotate(const Leap::Frame frame);
	void ScaleFingers(const Leap::Frame frame);
	std::vector<Leap::Vector> GetFingerTipsInSphere();
	Leap::Vector GetHandCenter();
	Leap::Vector CoordsGlobal2Local(Leap::Vector v);
	std::string GetMsg();
	//bool IsOneFingerExtended(const Leap::Frame frame);
	SphericalCursor();
	~SphericalCursor();

private:
	Leap::Vector _center;
	Leap::Vector _centerFixed;
	float _radiusIn;
	float _radiusOut;
	//int _state;
	//int _statePrev;
	Leap::Vector _translate;
	float _scale;
	std::vector<Leap::Vector> _fingersInSphere;
	Leap::Vector _handCenter;
	TRANSFORM_MODE _transformMode;
	TRANSFORM_MODE _transformModePrev;
	int _numExtendedFingers;
	std::clock_t _start;
	Leap::Vector _handDir;
	Leap::Vector _handDirPrev;
	Leap::Matrix _rotationMatrix;
	Leap::Frame _framePrev;
};
#endif