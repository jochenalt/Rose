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

	void computeServoAngles(const Pose& bodyPose, double servoAngle_rad[6], Point ballJoint_world[6],  Point servoBallJoint_world[6],
			                const Pose& headPose);
	void getServoArmCentre(Point servoArmCentre_world[6]);

private:
	StewartKinematics bodyKin;
	StewartKinematics headKin;

};

#endif /* SRC_STEWART_BODYKINEMATICS_H_ */
