/*
 * BodyKinematics.cpp
 *
 *  Created on: Feb 23, 2018
 *      Author: jochenalt
 */

#include <Stewart/BodyKinematics.h>


StewartConfiguration bodyStewartConfig = {"body",
		   	   	   	   	   	   	   	   	  46.5, 				// servoCentreRadius_mm
										  radians(12.4),		// servoCentreAngle_rad
										  50.4, 				// servoArmCentreRadius_mm
										  radians(11.5),		// servoArmCentreAngle_mm
										  40.0,					// plateJointRadius_mm
										  radians(7.7),			// plateJointAngle_rad
										  106.3,				// rodLength_mm
										  52.0,					// servoArmLength_mm
										  30.6,					// servoCentreHeight_mm
										  4.0,					// plateBallJointHeight_mm
										  radians(60.0),		// topServoLimit_rad
									      radians(90.0)			// bottomServoLimit_rad
};


StewartConfiguration headStewartConfig = {"head",
		                                  18.368, 				// servoCentreRadius_mm
										  radians(13.63),		// servoCentreAngle_rad
										  25.529, 				// servoArmCentreRadius_mm
										  radians(14.04),		// servoArmCentreAngle_mm
										  17.48,				// plateJointRadius_mm
										  radians(16.62),		// plateJointAngle_rad
										  56.3,				    // rodLength_mm
										  20.0,					// servoArmLength_mm
										  24.36,	 		    // servoCentreHeight_mm
										   4.0,					// plateBallJointHeight_mm
										  radians(60.0),		// topServoLimit_rad
										  radians(90.0)			// bottomServoLimit_rad

};


BodyKinematics::BodyKinematics() {
}

BodyKinematics::~BodyKinematics() {
}


void BodyKinematics::setup() {
	bodyKin.setup(bodyStewartConfig);
	headKin.setup(headStewartConfig);

}

void BodyKinematics::computeServoAngles(const Pose& bodyPose, Point bodyServoArmCentre_world[6], double bodyServoAngle_rad[6], Point bodyBallJoint_world[6],  Point bodyServoBallJoint_world[6],
						                const Pose& headPose, Point headServoArmCentre_world[6], double headServoAngle_rad[6], Point headBallJoint_world[6],  Point headServoBallJoint_world[6]) {
	bodyKin.getServoArmCentre(bodyServoArmCentre_world);
	bodyKin.computeServoAngles(bodyPose, bodyServoAngle_rad, bodyBallJoint_world,  bodyServoBallJoint_world);

	headKin.getServoArmCentre(headServoArmCentre_world);
	headKin.computeServoAngles(headPose, headServoAngle_rad, headBallJoint_world,  headServoBallJoint_world);

	/*
	// headPose is relative to body Pose
	// compute head pose in world coordinates
	HomogeneousMatrix bodyBaseTransformation = createTransformationMatrix(bodyPose);
	HomogeneousMatrix body2HeadTransformation = createTransformationMatrix(headPose);
	HomogeneousMatrix headTransformation = bodyBaseTransformation * body2HeadTransformation;
	headPose_world = getPoseByTransformationMatrix(headTransformation);
*/
}

void BodyKinematics::getServoArmCentre(Point servoArmCentre_world[6]) {
	bodyKin.getServoArmCentre(servoArmCentre_world);
}


