#include "SphericalCursor.h"
#include <Windows.h>
#include <GL/gl.h>
#include <GL\GLU.h>
#include <sstream>

using namespace Leap;

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};

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
	//Leap::Vector f1 = fingers.leftmost().tipPosition();
	//Leap::Vector f2 = fingers.rightmost().tipPosition();
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
	/*Leap::Vector f1 = fingers.leftmost().tipPosition();
	Leap::Vector f2 = fingers.rightmost().tipPosition();*/
	//_fingersInSphere.clear();
	//for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
	//	const Finger finger = *fl;
	//	_fingersInSphere.push_back((finger.tipPosition() - _translate) * _scale + _centerFixed);
	//}
	_handCenter = CoordsGlobal2Local(fingers[2].bone(Leap::Bone::Type::TYPE_PROXIMAL).prevJoint());
}

void SphericalCursor::Translate(const Leap::Frame frame)
{
	//for(int i = 0; i < _fingersInSphere.size(); i++)	{
	//	if(_fingersInSphere[i].distanceTo(_center) > _radiusOut)	{
	//		float moveAmount = _fingersInSphere[i].distanceTo(_center) - _radiusOut;
	//		Leap::Vector moveDir = (_fingersInSphere[i] - _center).normalized();
	//		_center += (moveDir * moveAmount * 0.1);
	//		break;
	//	}
	//}
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

	//for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
	//	// Get the first hand
	//	const Hand hand = *hl;
	//	//std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
	//	//std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
	//	//          << ", palm position: " << hand.palmPosition() << std::endl;
	//	//// Get the hand's normal vector and direction
	//	//const Vector normal = hand.palmNormal();
	//	//const Vector direction = hand.direction();

	//	//// Calculate the hand's pitch, roll, and yaw angles
	//	//std::cout << std::string(2, ' ') <<  "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
	//	//          << "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
	//	//          << "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;

	//	// Get fingers
	//	const FingerList fingers = hand.fingers();
	//	std::vector<Vector> fingertips;
	//	for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
	//		const Finger finger = *fl;
	//		std::cout << std::string(4, ' ') <<  fingerNames[finger.type()]
	//		<< " finger, id: " << finger.id()
	//			<< ", length: " << finger.length()
	//			<< "mm, width: " << finger.width() << std::endl;

	//		// Get finger bones
	//		for (int b = 0; b < 4; ++b) {
	//			Bone::Type boneType = static_cast<Bone::Type>(b);
	//			Bone bone = finger.bone(boneType);
	//			std::cout << std::string(6, ' ') <<  boneNames[boneType]
	//			<< " bone, start: " << bone.prevJoint()
	//				<< ", end: " << bone.nextJoint()
	//				<< ", direction: " << bone.direction() << std::endl;
	//		}
	//		fingertips.push_back(finger.bone(Leap::Bone::Type::TYPE_DISTAL).nextJoint());
	//	}

	//	_statePrev = _state;
	//	if(hand.grabStrength() > 0.9)
	//		_state = 1;
	//	else
	//		_state = 0;

	//	if(_statePrev == 0 && _state == 1)	{
	//		ScaleFingers(frame);
	//	}
	//	if(_state == 1)	{
	//		ComputeFingerInSphere(frame);
	//		ChangeSpheres();

	//	}
	//}

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

