#ifndef _GEOMETRICDERIVATIONS_H_
#define _GEOMETRICDERIVATIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "OSUFlow.h"

class GeometricDerivations
{
	public:
	// constructor and destructor
		GeometricDerivations();
		~GeometricDerivations();

		void GetData(OSUFlow* OsuFlow);

		VECTOR3 UnitNormal(const VECTOR3& point);
		VECTOR3 UnitBinormal(const VECTOR3& point);
		VECTOR3 UnitTangent(const VECTOR3& point);

		VECTOR3 BinormalPrime(const VECTOR3& point);
		VECTOR3 TangentPrime(const VECTOR3& point);
		
		MATRIX3 BinormalGradient(const VECTOR3& point);
		MATRIX3 TangentGradient(const VECTOR3& point);

		// Utility method to convert a float array to a Vector3
		VECTOR3 Convert(float * invalue);
		// Utility method to convert a Vector3 to float array
		float* Convert(VECTOR3  invalue);

		OSUFlow* osuflow;
		CVectorField* vectorField;
};

#endif



