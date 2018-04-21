
#ifndef TYPES_H_
#define TYPES_H_

#include <math.h>
#include <limits>

typedef double angle_rad;
typedef double angle_deg;

const double qnan = std::numeric_limits<double>::quiet_NaN();
const double inf  = std::numeric_limits<double>::infinity();
const double ninf = -std::numeric_limits<double>::infinity();

typedef double seconds;					    // time
typedef uint32_t milliseconds;					// time
typedef uint32_t microseconds;					// time
typedef double mmPerMillisecond;				// speed
typedef double mmPerSecond;					// speed
typedef double radPerSecond;					// angualar speed

typedef int millimeter_int;				     	// distance
typedef double millimeter;						// distance
typedef double mmPerMillisecondPerMillisecond;	// acceleration
typedef int int_millimeter;						// distance

// allowed difference when checking floats for equality
const int floatPrecisionDigits=8;
const double floatPrecision=pow(10.0,-floatPrecisionDigits);
const double floatPrecisionSqrt=sqrt(floatPrecision);

// include matrix library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include "matrix/cmatrix"
#pragma GCC diagnostic pop

using techsoft::mslice;

typedef techsoft::matrix<double>  Matrix;
typedef techsoft::matrix<double>  HomogeneousMatrix;
typedef std::valarray<double> HomogeneousVector;
typedef std::valarray<double> Vector;
#endif
