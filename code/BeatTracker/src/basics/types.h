
#ifndef TYPES_H_
#define TYPES_H_

#include <math.h>
#include <limits>

typedef double realnum;
typedef double angle_rad;
typedef double angle_deg;

const realnum qnan = std::numeric_limits<double>::quiet_NaN();
const realnum inf  = std::numeric_limits<double>::infinity();
const realnum ninf = -std::numeric_limits<double>::infinity();

typedef realnum seconds;					    // time
typedef uint32_t milliseconds;					// time
typedef realnum mmPerMillisecond;				// speed
typedef realnum mmPerSecond;					// speed
typedef realnum radPerSecond;					// angualar speed

typedef int millimeter_int;				     	// distance
typedef realnum millimeter;						// distance
typedef realnum mmPerMillisecondPerMillisecond;	// acceleration
typedef int int_millimeter;						// distance

// allowed difference when checking floats for equality
const int floatPrecisionDigits=8;
const realnum floatPrecision=pow(10.0,-floatPrecisionDigits);
const realnum floatPrecisionSqrt=sqrt(floatPrecision);

// include matrix library
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#include "matrix/cmatrix"
#pragma GCC diagnostic pop

using techsoft::mslice;

typedef techsoft::matrix<realnum>  Matrix;
typedef techsoft::matrix<realnum>  HomogeneousMatrix;
typedef std::valarray<realnum> HomogeneousVector;
typedef std::valarray<realnum> Vector;
#endif
