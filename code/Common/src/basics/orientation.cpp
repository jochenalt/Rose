
#include "basics/stringhelper.h"
#include "basics/orientation.h"
#include "basics/util.h"

#include "math.h"
#include <cmath>


Quaternion::Quaternion(const EulerAngles &e) {
	double t0 = std::cos(e.yaw * 0.5);
	double t1 = std::sin(e.yaw * 0.5);
	double t2 = std::cos(e.roll * 0.5);
	double t3 = std::sin(e.roll * 0.5);
	double t4 = std::cos(e.nick * 0.5);
	double t5 = std::sin(e.nick * 0.5);

	w = t0 * t2 * t4 + t1 * t3 * t5;
	x = t0 * t3 * t4 - t1 * t2 * t5;
	y = t0 * t2 * t5 + t1 * t3 * t4;
	z = t1 * t2 * t4 - t0 * t3 * t5;
}

EulerAngles::EulerAngles(const Quaternion &q) {
	double ysqr = q.y * q.y;

	// roll (x-axis rotation)
	double t0 = +2.0 * (q.w * q.x + q.y * q.z);
	double t1 = +1.0 - 2.0 * (q.x * q.x + ysqr);
	roll = std::atan2(t0, t1);

	// pitch (y-axis rotation)
	double t2 = +2.0 * (q.w * q.y - q.z * q.x);
	t2 = ((t2 > 1.0) ? 1.0 : t2);
	t2 = ((t2 < -1.0) ? -1.0 : t2);
	nick = std::asin(t2);

	// yaw (z-axis rotation)
	double t3 = +2.0 * (q.w * q.z + q.x * q.y);
	double t4 = +1.0 - 2.0 * (ysqr + q.z * q.z);
	yaw = std::atan2(t3, t4);
}

EulerAngles::EulerAngles(const Rotation &r) {
	roll = r.x;
	nick = r.y;
	yaw = r.z;
}

EulerAngles::EulerAngles(const EulerAngles  &eu) {
	roll = eu.roll;
	nick = eu.nick;
	yaw = eu.yaw;
}


Quaternion::Quaternion(const Rotation &r) {
	EulerAngles eu(r);
	Quaternion q(eu);
	*this = q;
}

ostream& operator<<(ostream& os, const Rotation& p)
{
	os << std::setprecision(3) << "R(" << p.x << "," << p.y << "," << p.z << ")";
    return os;
}


void Rotation::moveTo(const Rotation& b, double dT, double maxAngulaRspeed) {
	double d = distance(b);
	double maxDistance = maxAngulaRspeed*dT;
	if (d  > maxDistance) {
		(*this) += (b -(*this))*maxDistance/d ;
	}
	else
		(*this) = b;
}

std::ostream& Rotation::serialize(std::ostream &out) const {
	out << std::setprecision(4) << "{"
		<< "\"x\":" << floatToString(x,4) << ","
		<< "\"y\":" << floatToString(y,4) << ","
		<< "\"z\":" << floatToString(z,4) << "}";
	return out;
}


std::istream& Rotation::deserialize(std::istream &in, bool &ok) {
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

void Rotation::limit(const Rotation &min, const Rotation &max) {
	x = constrain(x, min.x, max.x);
	y = constrain(y, min.y, max.y);
	z = constrain(z, min.z, max.z);
}
