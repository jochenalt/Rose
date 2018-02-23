/*
 * BodyKinematics.cpp
 *
 *  Created on: Feb 23, 2018
 *      Author: jochenalt
 */

#include <Stewart/BodyKinematics.h>


StewartConfiguration bodyStewartConfig = {46.5, 				// servoCentreRadius_mm
										  radians(12.4),		// servoCentreAngle_rad
										  50.4, 				// servoArmCentreRadius_mm
										  radians(11.5),		// servoArmCentreAngle_mm
										  40.0,					// plateJointRadius_mm
										  radians(7.7),			// plateJointAngle_rad
										  106.3,				// rodLength_mm
										  52.0,					// servoArmLength_mm
										  30.6,					// servoCentreHeight_mm
										  4.0					// plateBallJointHeight_mm
									  	  };


StewartConfiguration headStewartConfig = {18.368, 				// servoCentreRadius_mm
										  radians(13.63),		// servoCentreAngle_rad
										  25.529, 				// servoArmCentreRadius_mm
										  radians(14.04),		// servoArmCentreAngle_mm
										  17.48,				// plateJointRadius_mm
										  radians(16.62),		// plateJointAngle_rad
										  56.3,				    // rodLength_mm
										  20.0,					// servoArmLength_mm
										  24.36,	 		    // servoCentreHeight_mm
										   4.0					// plateBallJointHeight_mm
									  	  };


BodyKinematics::BodyKinematics() {
}

BodyKinematics::~BodyKinematics() {
}


void BodyKinematics::setup() {
	bodyKin.setup(bodyStewartConfig);
}

void BodyKinematics::computeServoAngles(const Pose& bodyPose, double servoAngle_rad[6], Point ballJoint_world[6],  Point servoBallJoint_world[6],
						                const Pose& headPose) {
	bodyKin.computeServoAngles(bodyPose, servoAngle_rad, ballJoint_world,  servoBallJoint_world);
}

void BodyKinematics::getServoArmCentre(Point servoArmCentre_world[6]) {
	bodyKin.getServoArmCentre(servoArmCentre_world);
}


