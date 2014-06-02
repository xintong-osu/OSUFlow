#ifndef SPHERICAL_CURSOR_H
#define SPHERICAL_CURSOR_H
#include "vector_types.h"
#include "vector_functions.h"
#include "Leap.h"
#include "vector"

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
	void ChangeSpheres();
	void ScaleFingers(const Leap::Frame frame);
	std::vector<Leap::Vector> GetFingerTipsInSphere();
	Leap::Vector GetHandCenter();
	Leap::Vector CoordsGlobal2Local(Leap::Vector v);
	SphericalCursor();
	~SphericalCursor();

private:
	Leap::Vector _center;
	Leap::Vector _centerFixed;
	float _radiusIn;
	float _radiusOut;
	int _state;
	int _statePrev;
	Leap::Vector _translate;
	float _scale;
	std::vector<Leap::Vector> _fingersInSphere;
	Leap::Vector _handCenter;
};
#endif