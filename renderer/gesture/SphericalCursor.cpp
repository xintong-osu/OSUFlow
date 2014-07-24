#include "SphericalCursor.h"
#include <Windows.h>
#include <GL/gl.h>
#include <GL\GLU.h>
#include <sstream>

using namespace Leap;

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};


Leap::Vector RelativePalm3DLoc(Leap::Frame frame)
{
	Leap::Hand leftHand = frame.hands().leftmost();
	Leap::Hand rightHand = frame.hands().rightmost();
	Leap::Vector palmCenter = leftHand.stabilizedPalmPosition();
	float spaceSide = leftHand.palmWidth() * 2;
	Leap::Vector dir = leftHand.direction().normalized();
	Leap::Finger middleFinger = leftHand.fingers().fingerType(Leap::Finger::Type::TYPE_MIDDLE).frontmost();
	Leap::Vector middleFingerTip = middleFinger.stabilizedTipPosition();
	Leap::Vector palmNormal = leftHand.palmNormal();
	Leap::Vector yDir = palmNormal.cross(dir).normalized();
	Leap::Vector origin = palmCenter - dir * spaceSide * 0.3 - yDir * spaceSide * 0.5;

	Leap::Vector pointerTip = rightHand.fingers().fingerType(Leap::Finger::Type::TYPE_INDEX).frontmost().tipPosition();
	Leap::Vector vecPalmCenter2Tip = pointerTip - palmCenter;
	float dist2Palm = vecPalmCenter2Tip.dot(palmNormal.normalized());
	//http://stackoverflow.com/questions/9605556/how-to-project-a-3d-point-to-a-3d-plane
	Leap::Vector projTip = vecPalmCenter2Tip - dist2Palm * palmNormal + palmCenter;

	Leap::Vector vecOrigin2ProjTip = projTip - origin;
	Leap::Vector ret;
	ret.x = vecOrigin2ProjTip.dot(dir) / spaceSide;
	ret.y = vecOrigin2ProjTip.dot(yDir) / spaceSide;
	ret.z = (dist2Palm - 50) / spaceSide;


	return ret;
}

void RelativePlanePosition(Leap::Frame frame, Leap::Vector &planePt, Leap::Vector &planeNormal)
{
	Leap::Hand leftHand = frame.hands().leftmost();
	Leap::Hand rightHand = frame.hands().rightmost();
	Leap::Vector dirLeft = leftHand.direction().normalized();
	Leap::Vector palmNormalLeft = leftHand.palmNormal().normalized();
	Leap::Vector yDir = palmNormalLeft.cross(dirLeft).normalized();
	
	Leap::Vector rightNormal = rightHand.palmNormal().normalized();
	planeNormal.x = rightNormal.dot(dirLeft);
	planeNormal.y = rightNormal.dot(yDir);
	planeNormal.z = rightNormal.dot(palmNormalLeft);
	planeNormal = planeNormal.normalized();
	planePt = Clamp(RelativePalm3DLoc(frame));
}


void RelativeFingerOrientation()
{
}

static int GetNumOfExtendedFingers(const FingerList fingers)
{
	int extendedFingers = 0;
	for (int f = 0; f < fingers.count(); f++)	{
		Finger finger = fingers[f];
		if(finger.isExtended()) extendedFingers++;
	}
	return extendedFingers;
}

static Finger GetExtendedFinger(const FingerList fingers)
{
	for (int f = 0; f < fingers.count(); f++)	{
		Finger finger = fingers[f];
		if(finger.isExtended()) 
			return finger;
	}
}

void SphericalCursor::SetCenter(float x, float y, float z)
{
	_center.x = x;
	_center.y = y;
	_center.z = z;
}

void SphericalCursor::SetCenter(float3 v)
{
	_center = Leap::Vector(v.x, v.y, v.z);
}

void SphericalCursor::SetRadiusIn(float v)
{
	_radiusIn = v;
}

void SphericalCursor::SetRadiusOut(float v)
{
	_radiusOut = v;
}

float SphericalCursor::GetRadiusIn()
{
	return _radiusIn;
}

float SphericalCursor::GetRadiusOut()
{
	return _radiusOut;
}

Leap::Vector SphericalCursor::GetCenter()
{
	return _center;
}


void SphericalCursor::FingerInput(float3 v0, float3 v1, float3 v2, float3 v3, float3 v4)
{

}


void SphericalCursor::ScaleFingers(const Leap::Frame frame)//std::vector<Leap::Vector> v)
{
	const FingerList fingers = frame.hands().leftmost().fingers();
	_translate = GetExtendedFinger(fingers).tipPosition();//fingers[2].bone(Leap::Bone::Type::TYPE_PROXIMAL).prevJoint();//(f1 + f2) * 0.5;
	_scale = (_radiusIn + _radiusOut) / frame.hands().leftmost().palmWidth();
	_centerFixed = _center;
}

Leap::Vector SphericalCursor::CoordsGlobal2Local(Leap::Vector v)
{
	return (v- _translate) * _scale + _centerFixed;
}




void SphericalCursor::ComputeFingerInSphere(const Leap::Frame frame)
{
	const FingerList fingers = frame.hands().leftmost().fingers();
	_handCenter = CoordsGlobal2Local(fingers[2].bone(Leap::Bone::Type::TYPE_PROXIMAL).prevJoint());
}

void SphericalCursor::Translate(const Leap::Frame frame)
{
	FingerList fingers =  frame.hands().leftmost().fingers();
	if(GetNumOfExtendedFingers(fingers )> 0)
		_handCenter = CoordsGlobal2Local(GetExtendedFinger(fingers).tipPosition());
	else
		_handCenter = CoordsGlobal2Local(frame.hands().leftmost().fingers().fingerType(Finger::TYPE_INDEX)[0].tipPosition());
	//	_handCenter = CoordsGlobal2Local(fingers[2].bone(Leap::Bone::Type::TYPE_PROXIMAL).prevJoint());
	float dist = _handCenter.distanceTo(_center);
	if(dist > _radiusIn)	{
		float moveAmount = dist - _radiusIn;
		Leap::Vector moveDir = (_handCenter - _center).normalized();
		_center += (moveDir * moveAmount * 0.1);
	}
}

void SphericalCursor::Rotate(const Leap::Frame frame)
{
	_handDirPrev = _handDir;
	_handDir = frame.hands().leftmost().direction();
	;
	//Leap::Vector moveDir = _handDir - _handDirPrev;
	Leap::Vector rotAxis = _handDir.cross(_handDirPrev);
	float rotAng = _handDir.angleTo(_handDirPrev );

	Leap::Matrix rotMat(rotAxis, rotAng);
	_rotationMatrix = frame.hands().leftmost().rotationMatrix(_framePrev) * _rotationMatrix ;
}

std::vector<Leap::Vector> SphericalCursor::GetFingerTipsInSphere()
{
	return _fingersInSphere;
}

Leap::Vector SphericalCursor::GetHandCenter()
{
	return _handCenter;
}

void SphericalCursor::FingerInput(Leap::Frame frame)
{
	HandList hands = frame.hands();
	const Hand hand = hands.leftmost();
	_numExtendedFingers = GetNumOfExtendedFingers(hand.fingers());
	_transformModePrev = _transformMode;
	if(0 == _numExtendedFingers )
	{
		_transformMode = TRANSFORM_MODE::IDLE;

		_start = std::clock();
	}
    double duration;
    duration = ( std::clock() - _start ) / (double) CLOCKS_PER_SEC;

	//give half a second to allow the device to detect all the fingers
	if(_transformModePrev == TRANSFORM_MODE::IDLE && duration > 0.5)	{
		if(1 == _numExtendedFingers )
		{
			_transformMode = TRANSFORM_MODE::TRANSLATE;
			ScaleFingers(frame);
		}
		else if(5 == _numExtendedFingers )
			_transformMode = TRANSFORM_MODE::ROTATE;
		else if(2 == _numExtendedFingers )
			_transformMode = TRANSFORM_MODE::SCALE;

	}
	if(_transformMode == TRANSFORM_MODE::TRANSLATE)	{
		//if(_transformModePrev != TRANSFORM_MODE::TRANSLATE )
		//ComputeFingerInSphere(frame);
		Translate(frame);
	}
	else if(_transformMode == TRANSFORM_MODE::ROTATE)	{
		Rotate(frame);
	}
	_framePrev = frame;
}

void SphericalCursor::DrawFingersInSphere()
{
	GLUquadricObj *quadric;
	quadric = gluNewQuadric();
	gluQuadricDrawStyle(quadric, GLU_FILL);

	for(int i = 0; i < _fingersInSphere.size(); i++)	{
		glPushMatrix();
		glTranslatef(_fingersInSphere[i].x, _fingersInSphere[i].y, _fingersInSphere[i].z);
		gluSphere( quadric , _radiusIn * 0.2, 16 , 16 );
		glPopMatrix();
	}

	glPushMatrix();
	glTranslatef(_handCenter.x, _handCenter.y, _handCenter.z);
	gluSphere( quadric , _radiusIn * 0.4, 16 , 16 );
	glPopMatrix();

	gluDeleteQuadric(quadric); 
}

std::string SphericalCursor::GetMsg()
{
	//std::string ret;
	std::stringstream ss;
	ss<<"number of stretched fingers:"<< _numExtendedFingers<<"\n";
	ss<<"Mode:"<<_transformMode<<"\n";
	return ss.str();
}


void SphericalCursor::Draw()
{
	glPushMatrix();
	GLUquadricObj *quadric;
	quadric = gluNewQuadric();

	gluQuadricDrawStyle(quadric, GLU_FILL );
	glTranslatef(_center.x, _center.y, _center.z);

	gluQuadricDrawStyle(quadric, GLU_SILHOUETTE);
	glPushAttrib( GL_LIGHTING_BIT );
	glDisable(GL_LIGHTING);

	glMultMatrixf(_rotationMatrix.toArray4x4().m_array);

	gluSphere( quadric , _radiusIn, 16 , 16 );
	//gluSphere( quadric , _radiusOut, 16 , 16 );

	glPopAttrib();
	gluDeleteQuadric(quadric); 
	glPopMatrix();


	DrawFingersInSphere();
}

SphericalCursor::SphericalCursor()
{
	_transformMode = TRANSFORM_MODE::IDLE;
	_transformModePrev = TRANSFORM_MODE::IDLE;
}

SphericalCursor::~SphericalCursor()
{
}

