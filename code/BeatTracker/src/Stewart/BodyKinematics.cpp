/*
 * BodyKinematics.cpp
 *
 *  Created on: Feb 23, 2018
 *      Author: jochenalt
 */

#include <Stewart/BodyKinematics.h>


StewartConfiguration bodyStewartConfig = {"body",
										  28.972, 				// servoCentreRadius_mm
										  radians(12.28),		// servoCentreAngle_rad
										  34.763 , 				// servoArmCentreRadius_mm (for rendering only)
										  radians(10.19),		// servoArmCentreAngle_mm (for rendering only)
										  39.071,				// plateJointRadius_mm
										  radians(7.35),		// plateJointAngle_rad
										  74.0,   				// rodLength_mm
										  38.0,					// servoArmLength_mm
										  25.0,					// servoCentreHeight_mm
										  3.595,				// plateBallJointHeight_mm
										  radians(60.0),		// topServoLimit_rad
									      radians(90.0)			// bottomServoLimit_rad
};


StewartConfiguration headStewartConfig = {"head",
		                                  16.955, 				// servoCentreRadius_mm
										  radians(21.277),		// servoCentreAngle_rad
										  22.41, 				// servoArmCentreRadius_mm (for rendering only)
										  radians(15.93),		// servoArmCentreAngle_mm (for rendering only)
										  16.134,				// plateJointRadius_mm
										  radians(12.53),		// plateJointAngle_rad
										  44.0,				    // rodLength_mm
										  17.0,					// servoArmLength_mm
										  25.0,   	 		    // servoCentreHeight_mm
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
}

void BodyKinematics::getServoArmCentre(Point servoArmCentre_world[6]) {
	bodyKin.getServoArmCentre(servoArmCentre_world);
}

Pose BodyKinematics::computeHeadPose(const Pose& bodyPose, const Pose &relPoseAboveBellyButton) {
	// regular headPose is relative to body Pose. This function computes this relative
	// head pose out of an absolute pose that is right above the belly button
	// bodyPose * headPose = poseAboveBellyButton
	// -> headPose = bodyPose^-1 * poseAboveBellyButton

	Pose poseAboveBellyButton(relPoseAboveBellyButton);
	poseAboveBellyButton.position.x += bodyPose.position.x;
	poseAboveBellyButton.position.y += bodyPose.position.y;
	poseAboveBellyButton.position.z += bodyPose.position.z;

	// compute head pose in world coordinates
	HomogeneousMatrix bodyBaseTransformation = createTransformationMatrix(bodyPose);
	HomogeneousMatrix inverseBodyTransformation = computeInverseTransformationMatrix(bodyBaseTransformation);
	HomogeneousMatrix poseAboveBellyTransformation = createTransformationMatrix(poseAboveBellyButton);
	HomogeneousMatrix headTransformation = inverseBodyTransformation * poseAboveBellyTransformation;
	Pose headPose = getPoseByTransformationMatrix(headTransformation);
	return headPose;
}

