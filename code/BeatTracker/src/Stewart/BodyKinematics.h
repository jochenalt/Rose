/*
 * BodyKinematics.h
 *
 *  Created on: Feb 23, 2018
 *      Author: jochenalt
 */

#ifndef SRC_STEWART_BODYKINEMATICS_H_
#define SRC_STEWART_BODYKINEMATICS_H_

#include "basics/point.h"
#include "basics/spatial.h"
#include "StewartKinematics.h"

class BodyKinematics {
public:
	BodyKinematics();
	virtual ~BodyKinematics();
	void setup();
	static BodyKinematics& getInstance() {
		static BodyKinematics instance;
		return instance;
	}

	void computeServoAngles(const Pose& bodyPose, Point bodyServoArmCentre_world[6], double bodyServoAngle_rad[6], Point bodyBallJoint_world[6],  Point bodyServoBallJoint_world[6],
			                const Pose& headPose, Point headServoArmCentre_world[6], double headServoAngle_rad[6], Point headBallJoint_world[6],  Point headServoBallJoint_world[6]);
	void getServoArmCentre(Point servoArmCentre_world[6]);

	void resetSpeedMeasurement() { bodyKin.resetSpeedMeasurement(); headKin.resetSpeedMeasurement(); };
private:
	StewartKinematics bodyKin;
	StewartKinematics headKin;

};

#endif /* SRC_STEWART_BODYKINEMATICS_H_ */
