/*
 * spatial.h
 *
 * Spatial data structures
 * - LimbAngles all angles of one leg
 * - Pose           position and orientation. Also used as transformation containing a translation and a rotation
 * - StampedPose    pose with timestamp
 * - SpatialPID     PID controller for Rotation
 * - LegPose		A pose of a leg (considerung the toe points position) and all joint angles
 * - PentaType      A template making a class a 5-tuple
 *
 * Author: JochenAlt
 */

#ifndef SPATIAL_H_
#define SPATIAL_H_



#include "basics/serializer.h"
#include "basics/point.h"
#include "basics/orientation.h"
#include "basics/util.h"

// An aggregation of a point and an orientation
class Pose : public Serializable  {
	public:
		friend ostream& operator<<(ostream&, const Pose&);

		Pose() {
			null();
		};
		virtual ~Pose() {};
		Pose(const Pose& pose) {
			position = pose.position;
			orientation = pose.orientation;
		};
		Pose(const Point& pPosition) {
			null();
			position = pPosition;
		}
		Pose(const Point& pPosition, const Rotation& pOrientation) {
			position = pPosition;
			orientation = pOrientation;
		};

		Pose(const Point& pPosition, const Quaternion& pQuaternion) {
			position = pPosition;
			orientation = Rotation(pQuaternion);
		};

		Pose(const Point& pPosition, const EulerAngles& pEuler) {
			position = pPosition;
			orientation = EulerAngles(pEuler);
		};

		void operator= (const Pose& pose) {
			position = pose.position;
			orientation = pose.orientation;
		}

		void null() {
			orientation.null();
			position.null();
		}

		bool isNull() const {
			return position.isNull();
		}

		void moveTo(const Pose& b, double dT, double maxSpeed, double maxRotateSpeed) {
			position.moveTo(b.position, dT, maxSpeed);
			orientation.moveTo(b.orientation, dT, maxRotateSpeed);
		};

		float distance(const Pose& pPose) const {
			return sqrt((pPose.position[0]-position[0])*(pPose.position[0]-position[0]) +
						(pPose.position[1]-position[1])*(pPose.position[1]-position[1]) +
						(pPose.position[2]-position[2])*(pPose.position[2]-position[2]));
		}

		float length() const{
			return sqrt(sqr(position[0]) + sqr(position[1]) + sqr(position[2]));
		}

		bool operator==(const Pose& pPose){
			return 	(position == pPose.position &&
					orientation == pPose.orientation);
		};

		bool operator!=(const Pose& pos)  {
			return !((*this) == pos);
		};

		bool operator==(const Pose& pPose) const {
			return 	(position == pPose.position &&
					orientation == pPose.orientation);
		};

		bool operator!=(const Pose& pos) const  {
			return !((*this) == pos);
		};

		void operator+=(const Pose& pos) {
			position += pos.position;
			orientation += pos.orientation;
		};
		void operator-=(const Pose& pos) {
			position -= pos.position;
			orientation -= pos.orientation;
		};

		void operator*=(const float x) {
			position *= x;
			orientation *= x;
		};
		void operator/=(const float x) {
			position /= x;
			orientation /= x;
		};

		Pose operator*(float x) const {
			Pose result(*this);
			result *= x;
			return result;
		};

		Pose operator/(float x) const {
			Pose result(*this);
			result /= x;
			return result;
		};
		Pose  operator+(const Pose& pos) const {
			Pose result(*this);
			result += pos;
			return result;
		};

		Pose operator-(const Pose& pos) const {
			Pose result(*this);
			result -= pos;
			return result;
		};

		// apply a transformation denoted by a pose (transation (first) and rotation)
		Pose applyTransformation(const Pose& add) const;
		Pose inverse() const;

		// apply the inverse transformation denoted by a pose
		Pose applyInverseTransformation(const Pose& sub) const;

		virtual std::ostream& serialize(std::ostream &out) const;
		virtual std::istream& deserialize(std::istream &in, bool &ok);

		Point position;
		Rotation orientation;
};



// a pose with a timestamp
class TotalBodyPose : public Serializable  {
	public:
		friend ostream& operator<<(ostream&, const TotalBodyPose&);

		TotalBodyPose() {
			null();
		};

		virtual ~TotalBodyPose() {};
		TotalBodyPose(const TotalBodyPose& p) {
			head = p.head;
		};

		TotalBodyPose(const Pose& pHead) {
			head = pHead;
		};


		void operator= (const TotalBodyPose& p) {
			head = p.head;
		}

		void null() {
			head.null();
		}

		bool isNull() {
			return head.isNull();
		}

		bool operator==(const TotalBodyPose& p) {
			return (head == p.head);
		};

		bool operator!=(const TotalBodyPose& p) {
			return !((*this) == p);
		};

		virtual std::ostream& serialize(std::ostream &out) const;
		virtual std::istream& deserialize(std::istream &in, bool &ok);

		Pose head;
};


// A PID controller for orientation in x,y
class SpatialPID {
public:
	SpatialPID() {};
	~SpatialPID() {};

	// reset all historical values such that it will start with 0
	void reset();
	Rotation getPID(Rotation error, double p, double i, double d, const Rotation &outMax);

	// for debugging purposes, returns the integrated I value
	Rotation getErrorIntegral() { return errorIntegral; };
private:
	Rotation lastError;
	Rotation errorIntegral;
	TimeSampler pidSampler;
};


// some basic vector operations
bool 	almostEqual(const Point& a, const Point& b, double precision);
double triangleHypothenusisLength(double a, double b); 		// pythagoras
double triangleHeightToC(double a, double b, double c);		// herons law
Vector 	orthogonalVector(const Vector& a, double l);			// return orthogonal vector with length and z = 0
Vector 	crossProduct(const Vector& a, const Vector& b);
void 	setVectorLength(Vector &a, double length);					// multiply vector with a factor that results in the given length

// solves equation
//      a*sin(alpha) + b*cos(alpha) = c
// by alpha
// returns two solutions, if only one solution is available, alpha2 is qnan
void solveTrgLinearCombinationWithEqualPhase(double a, double b, double c, double &alpha1, double &alpha2, bool& infiniteSolutions);

// create a rotation matrix from a given rotation around all axes
void createRotationMatrix(const Rotation &r, HomogeneousMatrix& m);
// create a rotation matrix from a given rotation around all axes
HomogeneousMatrix createRotationMatrix(const Rotation &r);
HomogeneousMatrix createTransformationMatrix(const Point& p);


void computeInverseTransformationMatrix(HomogeneousMatrix m, HomogeneousMatrix& inverse);
HomogeneousMatrix computeInverseTransformationMatrix(HomogeneousMatrix m);

void createTransformationMatrix(const Pose& p, HomogeneousMatrix& m);
HomogeneousMatrix createTransformationMatrix(const Pose& p);
Rotation getRotationByTransformationMatrix(HomogeneousMatrix& m);
Point getPointByTransformationMatrix(HomogeneousMatrix& m);
Pose getPoseByTransformationMatrix(HomogeneousMatrix& m);

HomogeneousVector getHomogeneousVector(const Point& p);


#endif /* SPATIAL_H_ */
