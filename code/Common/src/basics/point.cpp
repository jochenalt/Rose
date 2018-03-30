
#include "math.h"
#include <cmath>
#include "basics/util.h"

#include "basics/point.h"
#include "basics/stringhelper.h"

ostream& operator<<(ostream& os, const Point& p)
{
	os << std::fixed << std::setprecision(1) << "P(" << p.x << "," << p.y << "," << p.z << ")";
    return os;
}


Point::Point() {
	null();
}

Point::Point(const Point& p) {
	x = p.x;
	y = p.y;
	z = p.z;
}


void Point::null() {
	x = 0.0;
	y = 0.0;
	z = 0.0;
};

Point::Point(double xP,double yP, double zP) {
	x = xP;
	y = yP;
	z = zP;
}

void Point::set(double pX, double pY,double pZ) {
	x = pX;
	y = pY;
	z = pZ;
}


bool Point::isNull() const {
	return 	((abs(x) < floatPrecision) &&
			 (abs(y) < floatPrecision) &&
			 (abs(z) < floatPrecision));
};

void Point::translate(const Point& pPoint) {
	x += pPoint.x;
	y += pPoint.y;
	z += pPoint.z;
}

Point  Point::getTranslated(const Point& pPoint) {
	Point result(*this);
	result += pPoint;
	return result;
}

void Point::mirrorAt(const Point& pMirror,double scale) {
	x = pMirror.x + (pMirror.x-x)*scale;
	y = pMirror.y + (pMirror.y-y)*scale;
	z = pMirror.z + (pMirror.z-z)*scale;
}

void Point::mirrorAt(const Point& pMirror) {
	x = pMirror.x + (pMirror.x-x);
	y = pMirror.y + (pMirror.y-y);
	z = pMirror.z + (pMirror.z-z);
}

double Point::distance(const Point& pPoint) const {
	return sqrt(distanceSqr(pPoint));
}

double Point::distanceSqr(const Point& pPoint) const {
	return (pPoint.x-x)*(pPoint.x-x) + (pPoint.y-y)*(pPoint.y-y) +  (pPoint.z-z)*(pPoint.z-z);
}


double Point::length() const {
	return sqrt(x*x + y*y+ z*z);
}

double Point::lengthSqr() const {
	return x*x + y*y+ z*z;
}

double Point::scalarProduct(const Point& pPoint) const {
	return x*pPoint.x + y*pPoint.y + z*pPoint.z;
}

Point Point::orthogonalProjection(const Point& pLine) const {
	Point result = pLine;
	result *= scalarProduct(pLine)/ pLine.scalarProduct(pLine);
	return result;
}

Point Point::orthogonalProjection(const Point& pLineA, const Point& pLineB) const {
	Point translate(pLineA);
	Point selfTranslated=(*this);
	selfTranslated-= translate;
	Point line = pLineB;
	line -= translate;
	Point result = selfTranslated.orthogonalProjection(line);
	result += translate;
	return result;
}

Point Point::getPointOfLine(double ratio, const Point& target) {
	if (ratio > 1.0)
		ratio = 1.0;
	if (ratio < 0.0)
		ratio = 0.0;
	Point result(*this);
	result.x += ratio*(float)(target.x - x);
	result.y += ratio*(float)(target.y - y);
	result.z += ratio*(float)(target.z - z);
	return result;
}

Point::~Point() {

};

Point::Point(const HomogeneousVector& p) {
	x = p[X];
	y = p[Y];
	z = p[Z];
}

Point::Point(int args, const valarray<float>& vec) {
	for (int i = 0;i<args;i++)
		(*this)[i] = vec[i];
}


Point Point::getRotatedAroundZ(double alpha) const {
	double sa = sin(alpha);
	double ca = cos(alpha);

	Point rotated((ca*x - sa*y),
			      (sa*x + ca*y),
			      z );
	return rotated;
}

Point Point::getRotatedAroundX(double alpha) const {
	double sa = sin(alpha);
	double ca = cos(alpha);

	Point rotated(x,
			      ( ca*y - sa*z),
			      ( sa*y + ca*z)
			      );
	return rotated;
}

Point Point::getRotatedAroundY(double alpha) const {
	double sa = sin(alpha);
	double ca = cos(alpha);

	Point rotated(( ca*x + sa*z),
			      y,
			      (-sa*x + ca*z)
			      );
	return rotated;
}


Point& Point::operator= (const Point& p) {
		x = p.x;
		y = p.y;
		z = p.z;
		return *this;
}

void Point::operator= (const HomogeneousVector& p) {
		x = p[X];
		y = p[Y];
		z = p[Z];
}

void Point::limit(const Point &min, const Point &max) {
	x = constrain(x, min.x, max.x);
	y = constrain(y, min.y, max.y);
	z = constrain(z, min.z, max.z);
}


void Point::operator+= (const Point& p) {
	x += p.x;
	y += p.y;
	z += p.z;
}

void Point::operator-= (const Point& p) {
	x -= p.x;
	y -= p.y;
	z -= p.z;
}

void Point::operator*= (const double f) {
	x *= f;
	y *= f;
	z *= f;
}

void Point::operator/= (const double f) {
	float xrf= 1.0/f;
	x *= xrf;
	y *= xrf;
	z *= xrf;
}

Point Point::operator- (const Point& p) const{
	Point result= (*this);
	result -= p;
	return result;
}

Point Point::operator+ (const Point& p) const{
	Point result= (*this);
	result += p;
	return result;
}

Point Point::operator/ (const double f) const{
	Point result= (*this);
	result*= (1./f);
	return result;
}

Point Point::operator* (const double f) const{
	Point result= (*this);
	result*=f;
	return result;
}


bool Point::operator==(const Point& pos) {
	return ((abs(x-pos.x) < floatPrecision) && (abs(y - pos.y) <floatPrecision) && (abs(z -pos.z) < floatPrecision));
};

bool Point::operator!=(const Point& pos) {
	return ((*this) != (Point)pos);
};

bool Point::operator==(const Point& pos) const {
	return ((abs(x-pos.x) < floatPrecision) && (abs(y - pos.y) <floatPrecision) && (abs(z -pos.z) < floatPrecision));
};

bool Point::operator!=(const Point& pos) const {
	return !((*this) == pos);
};


double& Point::operator[] (int idx)  {
	switch (idx) {
		case X:	return x;break;
		case Y:	return y;break;
		case Z:	return z;break;
			default:
		break;
	}
	return x;
};

double Point::operator[] (int idx)  const {
	switch (idx) {
		case X:	return x;break;
		case Y:	return y;break;
		case Z:	return z;break;
		default:
		break;
	}
	return x;
};

void Point::moveTo(const Point& b, seconds dT, double maxSpeed) {
	double d = distance(b);
	double maxDistance = maxSpeed*dT;
	if (d  > maxDistance) {
		(*this) += (b -(*this))*maxDistance/d ;
	}
	else
		(*this) = b;
}

HomogeneousVector Point::getHomVector() const {
	HomogeneousVector result = { x,y,z,1.0 };
	return result;
}

// returns the vector, i.e. a 3-dimensional vector
Vector Point::getVector() const {
		Vector result = { x,y,z };
		return result;
}

std::ostream& Point::serialize(std::ostream &out) const {
	out << std::setprecision(4) << "{"
		<< "\"x\":" << floatToString(x,2) << ","
		<< "\"y\":" << floatToString(y,2) << ","
		<< "\"z\":" << floatToString(z,2) << "}";
	return out;
}

std::istream& Point::deserialize(std::istream &in, bool &ok) {
    if (in) {
    	parseCharacter(in, '{', ok);
    	parseString(in, ok);
    	parseCharacter(in, ':', ok);
    	deserializePrim(in, x, ok);
    	parseCharacter(in, ',', ok);
    	parseString(in, ok);
    	parseCharacter(in, ':', ok);
    	deserializePrim(in, y, ok);
    	parseCharacter(in, ',', ok);
    	parseString(in, ok);
    	parseCharacter(in, ':', ok);
    	deserializePrim(in, z, ok);
    	parseCharacter(in, '}', ok);
    }
    return in;
}
