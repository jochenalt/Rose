#define _USE_MATH_DEFINES
#include <cmath>

#include "basics/serializer.h"
#include "basics/stringhelper.h"

#include "basics/spatial.h"
#include "basics/util.h"


ostream& operator<<(ostream& os, const Pose& p)
{
	os << std::setprecision(3) << "( pos=" << p.position << ",ori=" << p.orientation << ")";
	return os;
}


// add a vector to a pose by considering the applied pose as transformation(trans and rot)
Pose Pose::applyTransformation(const Pose& add) const {
		Pose result;
	 	result.position = position + add.position.getRotatedAroundZ(orientation.z);
	 	result.orientation.z = orientation.z  + add.orientation.z;
	 	return result;
}


Pose Pose::inverse() const {
	Pose result;
	result.position -= position;
	result.position = result.position.getRotatedAroundZ(-orientation.z);
	result.orientation -= orientation;
 	return result;
}


Pose Pose::applyInverseTransformation(const Pose& sub) const {
		Pose  result;
		result.position = position - sub.position.getRotatedAroundZ(orientation.z-sub.orientation.z);
		result.orientation.z = orientation.z-sub.orientation.z;
	 	return result;
}


std::ostream& Pose::serialize(std::ostream &out) const {
	out << "{\"pos\":";
	position.serialize(out);
	out << ",\"ori\":";
	orientation.serialize(out);
	out << "}";
	return out;
}

std::istream& Pose::deserialize(std::istream &in, bool &ok) {
    if (in) {
    	parseCharacter(in, '{', ok);
    	parseString(in, ok); // "position"
    	parseCharacter(in, ':', ok);
    	position.deserialize(in, ok);
    	parseCharacter(in, ',', ok);

    	parseString(in, ok); // "orientation"
    	parseCharacter(in, ':', ok);
    	orientation.deserialize(in, ok);
    	parseCharacter(in, '}', ok);
    }
    return in;
}


ostream& operator<<(ostream& os, const TotalBodyPose& p)
{
	os << std::setprecision(3) << "(head=" << p.head<< ")";
	return os;
}


std::ostream& TotalBodyPose::serialize(std::ostream &out) const {
	out << "{\"head\":";
	head.serialize(out);
	out << "}";
	return out;
}

std::istream& TotalBodyPose::deserialize(std::istream &in, bool &ok) {
    if (in) {
    	parseCharacter(in, '{', ok);
    	parseString(in, ok); // "head"
    	parseCharacter(in, ':', ok);
    	head.deserialize(in, ok);
    	parseCharacter(in, '}', ok);
    }
    return in;
}

// return length of hyopthenusis of orthogonal triangle
double triangleHypothenusisLength(double a, double b) {
    return sqrt(a*a+b*b);
}

// return height of general triangle (no assumptions about shape)
double triangleHeightToC(double a, double b, double c) {
	// herons law
	double semiperimeter = 0.5*(a + b + c);
	// take care that squareroot does not get negative, this can happen if semiperimeter gets close to c
	double height = 0;
	if ((semiperimeter - b > 0) && (semiperimeter - a > 0) && (semiperimeter - c > 0))
		height = 2.0 / c * sqrt(semiperimeter* (semiperimeter-a) * (semiperimeter-b) * (semiperimeter-c));

	return height;
}

// return a vector that is orthogonal to a, has length l and z=0
Vector orthogonalVector(const Vector& a, double l) {
	double x = l/(sqrt( sqr(a[X]/a[Y]) + 1.0));
	Vector S = { x, sqrt(l*l - x*x) , 0 };
	return S;
}

Vector crossProduct(const Vector& a, const Vector& b) {
	Vector c = { a[Y]*b[Z] - a[Z]*b[Y], a[Z]*b[X] - a[X]*b[Z], a[X]*b[Y] - a[Y]*b[X]};
	return c;
}

void setVectorLength(Vector &a, double l) {
	double factor = l/sqrt(a[X]*a[X] + a[Y]*a[Y] + a[Z]*a[Z]);
	a[X] *= factor;
	a[Y] *= factor;
	a[Z] *= factor;
}

// solve equation a*sin(alpha) + b*cos(alpha) = c
void solveTrgLinearCombinationWithEqualPhase(double a, double b, double c,
		double &alpha1, double &alpha2, bool& infiniteSolutions) {
	infiniteSolutions = false;

	// take care that either a or b is negative, makes cases easier.
	if ((a<0) && (b<0)) {
		a = -a;
		b = -b;
		c = -c;
	}

	if (abs(a) < floatPrecision) {
		if (abs(b) < floatPrecision) {
			alpha1 = qnan;
			alpha2 = qnan;
			infiniteSolutions = true;
		} else {
			alpha1 = acos(c/b);
			alpha2 = qnan;
		}
	}
	else {
		if (abs(b) < floatPrecision) {
			alpha1 = asin(c/a);
			alpha2 = qnan;
		} else {
			double amplitude = sqrt (a*a + b*b);

			double t1 = asin(c/amplitude);
			double phase1 = atan(b/a);

			double t2 = acos(c/amplitude);
			double phase2 = atan(a/b);
			// in first and forth quadrant, phase is ok, in
			// second and third quadrant we need to add PI (check ./theory/linear combination of sin and cos.pdf)
			if (a<0) {// = cos(phase)
				phase1 += M_PI;
			}
			if (b<0) {// = cos(phase)
				phase2 += M_PI;
			}

			alpha1 = t1 - phase1;
			alpha2 = t2 + phase2;
// #define DOUBLECHECK
#ifdef DOUBLECHECK
			double probe1 = a*sin(alpha1) + b*cos(alpha1);
			double probe2 = a*sin(alpha2) + b*cos(alpha2);

			if ((abs(probe1-c) > floatPrecision) || (abs(probe2-c) > floatPrecision)) {
				throw "error in solveTrgLinearCombinationWithEqualPhase";
			}
#endif
		}
	}

	// take care that alpha1 is always set, alpha2 might be nan
	if ((alpha1 == qnan) && (alpha2 != qnan)) {
		alpha1 = alpha2;
		alpha2 = qnan;
	}

}

// return the distance of two 3D points
double distance(const Vector& a, const Vector& b) {
    return sqrt((b[0]-a[0])*(b[0]-a[0]) + (b[1]-a[1])*(b[1]-a[1]) + (b[2]-a[2])*(b[2]-a[2]));
}



void computeInverseTransformationMatrix(HomogeneousMatrix m, HomogeneousMatrix& inverse) {
	inverse = HomogeneousMatrix(4,4,
			{ 	m[0][0], m[1][0], m[2][0],- m[0][0]*m[0][3] - m[1][0]*m[1][3] - m[2][0]*m[2][3],
				m[0][1], m[1][1], m[2][1],- m[0][1]*m[0][3] - m[1][1]*m[1][3] - m[2][1]*m[2][3],
				m[0][2], m[1][2], m[2][2],- m[0][2]*m[0][3] - m[1][2]*m[1][3] - m[2][2]*m[2][3],
				0,			0		,0		,	1});

}

HomogeneousMatrix computeInverseTransformationMatrix(HomogeneousMatrix m) {
	HomogeneousMatrix inverse = HomogeneousMatrix(4,4,
			{ 	m[0][0], m[1][0], m[2][0],- m[0][0]*m[0][3] - m[1][0]*m[1][3] - m[2][0]*m[2][3],
				m[0][1], m[1][1], m[2][1],- m[0][1]*m[0][3] - m[1][1]*m[1][3] - m[2][1]*m[2][3],
				m[0][2], m[1][2], m[2][2],- m[0][2]*m[0][3] - m[1][2]*m[1][3] - m[2][2]*m[2][3],
				0,			0		,0		,	1});
	return inverse;
}

void createRotationMatrix(const Rotation &r, HomogeneousMatrix& m) {
	double sinX = sin(r.x);
	double cosX = cos(r.x);
	double sinY = sin(r.y);
	double cosY = cos(r.y);
	double sinZ = sin(r.z);
	double cosZ = cos(r.z);

	m = HomogeneousMatrix(4,4,
			{ 	cosZ*cosY, 	-sinZ*cosX+cosZ*sinY*sinX,  	sinZ*sinX+cosZ*sinY*cosX, 	0,
				sinZ*cosY, 	 cosZ*cosX + sinZ*sinY*sinX, 	-cosZ*sinX+sinZ*sinY*cosX, 	0,
				-sinY,	 	cosY*sinX,						cosY*cosX,					0,
				0,			0,								0,							1});
}

HomogeneousMatrix createRotationMatrix(const Rotation &r) {
	HomogeneousMatrix result(4,4);
	createRotationMatrix(r, result);
	return result;
}

void createTransformationMatrix(const Pose& p, HomogeneousMatrix& m) {
	m = HomogeneousMatrix(4,4,
						{ 1, 	0,  	0,  	p.position.x,
						  0, 	1, 		0,	 	p.position.y,
						  0,	0,		1,		p.position.z,
						  0,	0,		0,		1});
	HomogeneousMatrix rotate(4,4);
	createRotationMatrix(p.orientation, rotate);
	m *= rotate;
}

void createTransformationMatrix(const Point& p, HomogeneousMatrix& m) {
	m = HomogeneousMatrix(4,4,
						{ 1, 	0,  	0,  	p.x,
						  0, 	1, 		0,	 	p.y,
						  0,	0,		1,		p.z,
						  0,	0,		0,		1});
}


HomogeneousMatrix createTransformationMatrix(const Pose& p) {
	HomogeneousMatrix result(4,4);
	createTransformationMatrix(p, result);
	return result;
}

HomogeneousMatrix createTransformationMatrix(const Point& p) {
	HomogeneousMatrix result(4,4);
	createTransformationMatrix(p, result);
	return result;
}

Point getPointByTransformationMatrix(HomogeneousMatrix& m) {
	return  Point(m[0][3], m[1][3], m[2][3]);
}

Rotation getRotationByTransformationMatrix(HomogeneousMatrix& m) {
	double beta = atan2(-m[2][0], sqrt(m[0][0]*m[0][0] + m[1][0]*m[1][0]));
	double gamma = 0;
	double alpha = 0;
	if (almostEqual(beta, (M_PI/2.0), floatPrecision)) {
		alpha = 0;
		gamma = atan2(m[0][1], m[1][1]);
	} else {
		if (almostEqual(beta, -(M_PI/2.0),floatPrecision)) {
			alpha = 0;
			gamma = -atan2(m[0][1], m[1][1]);
		} else {
			alpha = atan2(m[1][0],m[0][0]);
			gamma = atan2(m[2][1], m[2][2]);
		}
	}

	return Rotation (gamma, beta, alpha);
}

Pose getPoseByTransformationMatrix(HomogeneousMatrix& m) {
	return Pose(getPointByTransformationMatrix(m),getRotationByTransformationMatrix(m));
}

HomogeneousVector getHomogeneousVector(const Point& p) {
	HomogeneousVector result = {
		p.x,
		p.y,
		p.z,
		1.0 };
	return result;
}

bool almostEqual(const Point& a, const Point& b, double precision) {
	return ((abs(a.x-b.x) < precision) &&
			(abs(a.y-b.y) < precision) &&
			(abs(a.z-b.z) < precision));
}



void SpatialPID::reset() {
	errorIntegral.null();
	lastError.null();
	pidSampler.reset();
}

Rotation SpatialPID::getPID(Rotation error, double propFactor, double IntegFactor, double derivativeFactor, const Rotation &outMax) {
		Rotation outMin = outMax * -1.0;
		seconds dT = pidSampler.dT();
		Rotation imuCompensation ;
		if (dT > floatPrecision) { // first round is for the road
			Rotation prop = error;
			Rotation integ = errorIntegral;
			Rotation deriv = (error-lastError);

			// cout << std::setprecision(4) << "pid=(" << prop << "," << integ << "," << deriv << ")->(" << prop*p << "," << integ*i << "," << deriv*d << ") dT=" << dT;
			imuCompensation = prop*propFactor + integ * IntegFactor + deriv*derivativeFactor / dT;
			imuCompensation.limit(outMin, outMax);

			errorIntegral += error*dT;
			errorIntegral.limit(outMin, outMax);
			lastError = error;
		}

		return imuCompensation;
	}



