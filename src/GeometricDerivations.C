#include "GeometricDerivations.h"
#include "calculus.h"

GeometricDerivations::GeometricDerivations(){}
GeometricDerivations::~GeometricDerivations(){}

void GeometricDerivations::GetData(OSUFlow* OsuFlow)
{
	osuflow = OsuFlow;
	vectorField = osuflow->GetFlowField();
}

/// <summary>
/// Calculate the derivative of the tangent. Result is normalized.
/// </summary>
/// <param name="point">The point at which to calculate the tangent</param>
/// <returns>nit principal normal vector</returns>
VECTOR3 GeometricDerivations::UnitNormal(const VECTOR3& point)
{
	VECTOR3 normal;
	
	// The gradient will give us the rate of change in X, Y, and Z for U, V, and W
	MATRIX3 jacobian = TangentGradient(point);

	// uvw values
	VECTOR3 uvw = UnitTangent(point);

	// jacobian * tangent yields normal
	normal = mvmult(jacobian, uvw);

	// Make it a unit vector
	normal.Normalize();

	return normal;
}


/// <summary>
/// Calculate the unit principal binormal vector.
/// </summary>
/// <param name="point">Coordinate in the vector flow field.</param>
/// <returns>Unit binormal vector.</returns>
VECTOR3 GeometricDerivations::UnitBinormal(const VECTOR3& point)
{
	// Need unit tangent and unit normal for the binormal
	VECTOR3 tangent = UnitTangent(point);
	VECTOR3 normal = UnitNormal(point);

	// binormal is tangent cross normal according to wiki
	VECTOR3 binormal = crossProduct(tangent,normal);

	// The magnitude here should be 1 already, but for consistency sake:
	binormal.Normalize();

	return binormal;
}


/// <summary>
/// The unit vector tangent.
/// </summary>
/// <param name="point">Coordinate in the vector field.</param>
/// <returns>Unit vector tangent.</returns>
VECTOR3 GeometricDerivations::UnitTangent(const VECTOR3& point)
{
	VECTOR3 tangent;
	vectorField->at_phys(point,0,tangent);

	// Make it a unit vector
	tangent.Normalize();

	return tangent;
}


/// <summary>
/// 
/// </summary>
/// <param name="point"></param>
/// <returns></returns>
VECTOR3 GeometricDerivations::BinormalPrime(const VECTOR3& point)
{
	// The gradient will give us the rate of change in X, Y, and Z for U, V, and W
    MATRIX3 jacobian = BinormalGradient(point);

    // uvw values 
    VECTOR3 binormal = UnitBinormal(point);

    // jacobian * binormal yields the derivative of the binormal
    VECTOR3 binormalPrime = mvmult(jacobian, binormal);

   // return the derived quantity
    return binormalPrime;
}


VECTOR3 GeometricDerivations::TangentPrime(const VECTOR3& point)
{
	// The gradient will give us the rate of change in X, Y, and Z for U, V, and W
	MATRIX3 jacobian = TangentGradient(point);

	// uvw values
	VECTOR3 tangent = UnitTangent(point);

	// jacobian * tangent yields the derivative of the tangent
	VECTOR3 tangentPrime = mvmult(jacobian, tangent);

	// return the non-normalized derived quantity
	return tangentPrime;
}


MATRIX3 GeometricDerivations::BinormalGradient(const VECTOR3& point)
{
	//
    // Gradient of the x, y, and z components
    //
	MATRIX3 jacobian;
	
        VECTOR3 deltax(0.5f, 0.0f, 0.0f);
    VECTOR3 deltay(0.0f, 0.5f, 0.0f);
    VECTOR3 deltaz(0.0f, 0.0f, 0.5f);

	VECTOR3 forwardVector;      // f(x + h) 
    VECTOR3 backwardVector;     // f(x - h)
    VECTOR3 componentGradient;  // f'(x)

	// ***
    // FIRST ROW
    // ***
    forwardVector = UnitBinormal(point + deltax);
    backwardVector = UnitBinormal(point - deltax);

    // f'(x) = (  f(x + h) - f(x - h)  ) / 2
    componentGradient = (forwardVector - backwardVector);

    // dF/dx
    jacobian[0][0] = componentGradient[0] / 1.0f;
    jacobian[1][0] = componentGradient[1] / 1.0f;
    jacobian[2][0] = componentGradient[2] / 1.0f;

	// ***
    // SECOND ROW
    // ***
    forwardVector = UnitBinormal(point + deltay);
    backwardVector = UnitBinormal(point - deltay);

    // f'(x) = (  f(x + h) - f(x - h)  ) / 2
    componentGradient = (forwardVector - backwardVector);

    // dF/dx
    jacobian[0][1] = componentGradient[0] / 1.0f;
    jacobian[1][1] = componentGradient[1] / 1.0f;
    jacobian[2][1] = componentGradient[2] / 1.0f;

	// ***
    // THIRD ROW
    // ***
    forwardVector = UnitBinormal(point + deltaz);
    backwardVector = UnitBinormal(point - deltaz);

    // f'(x) = (  f(x + h) - f(x - h)  ) / 2
    componentGradient = (forwardVector - backwardVector);

    // dF/dx
    jacobian[0][2] = componentGradient[0] / 1.0f;
    jacobian[1][2] = componentGradient[1] / 1.0f;
    jacobian[2][2] = componentGradient[2] / 1.0f;

    return jacobian;
}


/// <summary>
/// Jacobian Matrix:
/// Row 1: du/dx  du/dy  du/dz 
/// Row 2: dv/dx  dv/dy  dv/dz 
/// Row 3: dw/dx  dw/dy  dw/dz 
/// 
/// Calculated taking central difference.
/// 
/// Method depends on fact that Tangent method normalizes vector field entries.
/// </summary>
/// <param name="point">point in the vector field.</param>
/// <returns>Returns a matrix containing the jacobian of the vector field.</returns>
MATRIX3 GeometricDerivations::TangentGradient(const VECTOR3& point)
{
	//
    // Gradient of the x, y, and z components
    //
    MATRIX3 jacobian;
	
    VECTOR3 deltax(0.5f, 0.0f, 0.0f);
    VECTOR3 deltay(0.0f, 0.5f, 0.0f);
    VECTOR3 deltaz(0.0f, 0.0f, 0.5f);

    VECTOR3 forwardVector;      // f(x + h)
    VECTOR3 backwardVector;     // f(x - h)
    VECTOR3 componentGradient;  // f'(x)

    // ***
    // FIRST ROW
    // ***
    //VECTOR3 test(50.0,50.0,50.0);
    //forwardVector = UnitTangent(point + test);

    forwardVector = UnitTangent(point + deltax);
    backwardVector = UnitTangent(point - deltax);

    // f'(x) = (  f(x + h) - f(x - h)  ) / 2
    componentGradient = (forwardVector - backwardVector);

    // dF/dx
    jacobian[0][0] = componentGradient[0] / 1.0f;
    jacobian[1][0] = componentGradient[1] / 1.0f;
    jacobian[2][0] = componentGradient[2] / 1.0f;

    // ***
    // SECOND ROW
    // ***
    forwardVector = UnitTangent(point + deltay);
    backwardVector = UnitTangent(point - deltay);

    // f'(x) = (  f(x + h) - f(x - h)  ) / 2
    componentGradient = (forwardVector - backwardVector);

    // dF/dx
    jacobian[0][1] = componentGradient[0] / 1.0f;
    jacobian[1][1] = componentGradient[1] / 1.0f;
    jacobian[2][1] = componentGradient[2] / 1.0f;

	// ***
    // THIRD ROW
    // ***
    forwardVector = UnitTangent(point + deltaz);
    backwardVector = UnitTangent(point - deltaz);

    // f'(x) = (  f(x + h) - f(x - h)  ) / 2
    componentGradient = (forwardVector - backwardVector);

    // dF/dx
    jacobian[0][2] = componentGradient[0] / 1.0f;
    jacobian[1][2] = componentGradient[1] / 1.0f;
    jacobian[2][2] = componentGradient[2] / 1.0f;

    return jacobian;
}
