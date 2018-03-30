/*
 * BodyKinematics.cpp
 *
 *  Created on: Feb 23, 2018
 *      Author: jochenalt
 */

#include <stewart/BodyKinematics.h>


StewartConfiguration bodyStewartConfig = {"body",
										  27.354, 				// servoCentreRadius_mm
										  radians(12.83),		// servoCentreAngle_rad
										  35.211, 				// servoArmCentreRadius_mm (for rendering only)
										  radians(9.93),		// servoArmCentreAngle_mm (for rendering only)
										  28.036,				// plateJointRadius_mm
										  radians(8.2),  		// plateJointAngle_rad
										  74.0,   				// rodLength_mm
										  29.478,				// servoArmLength_mm
										  31.2,					// servoCentreHeight_mm
										  4.0,					// plateBallJointHeight_mm
										  65.0, 				// bottomPlatformRadius_mm
										  65.0, 				// topPlatformRadius_mm
										  -radians(56.0),		// topServoLimit_rad
									      radians(86.0)			// bottomServoLimit_rad
};


StewartConfiguration headStewartConfig = {"head",
										  27.354, 				// servoCentreRadius_mm
										  radians(12.83),		// servoCentreAngle_rad
										  35.211, 				// servoArmCentreRadius_mm (for rendering only)
										  radians(9.93),		// servoArmCentreAngle_mm (for rendering only)
										  28.036,				// plateJointRadius_mm
										  radians(8.2),		    // plateJointAngle_rad
										  84.0,				    // rodLength_mm
										  29.478,				// servoArmLength_mm
										  24.450,  	 		    // servoCentreHeight_mm
										  4.0,					// plateBallJointHeight_mm
										  65.0, 				// bottomPlatformRadius_mm
										  45.0, 				// topPlatformRadius_mm
										  -radians(56.0),		// topServoLimit_rad
										  radians(86.0)			// bottomServoLimit_rad

};

StewartConfiguration& BodyKinematics::getBodyConfig() {
	return bodyStewartConfig;
}
StewartConfiguration& BodyKinematics::getHeadConfig() {
	return headStewartConfig;
}


BodyKinematics::BodyKinematics() {
}

BodyKinematics::~BodyKinematics() {
}


void BodyKinematics::getPlatformMetrics(double& basePlatformRadius, double &intermediatePlatformRadius, double& topPlatformRadius, double& bottomRodLength, double & topRodLength) {
	basePlatformRadius = bodyStewartConfig.bottomPlatformRadius_mm;
	intermediatePlatformRadius = bodyStewartConfig.topPlatformRadius_mm;
	topPlatformRadius = headStewartConfig.topPlatformRadius_mm;
	bottomRodLength = bodyStewartConfig.rodLength_mm;
	topRodLength = headStewartConfig.rodLength_mm;

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

Pose BodyKinematics::computeHeadStewartPose(const Pose& bodyPose, const Pose &relPoseAboveBellyButton) {

	// regular headPose is right above the bodys origin with its own orientation.

	// This function computes this relative
	// head pose out of an absolute pose that is right above the belly button
	// headPose*bodyPose = poseAboveBellyButton
	// -> headPose =  poseAboveBellyButton * bodyPose^-1

	Pose poseAboveBellyButton(relPoseAboveBellyButton);
	poseAboveBellyButton.position += bodyPose.position;

	// compute head pose in relative to the body platform
	HomogeneousMatrix bodyBaseTransformation(4,4);
	createTransformationMatrix(bodyPose, bodyBaseTransformation);
	HomogeneousMatrix inverseBodyTransformation = computeInverseTransformationMatrix(bodyBaseTransformation);

	HomogeneousMatrix poseAboveBellyTransformation(4,4);
	createTransformationMatrix(poseAboveBellyButton, poseAboveBellyTransformation);
	HomogeneousMatrix headTransformation = inverseBodyTransformation*poseAboveBellyTransformation;
	Pose headPose = getPoseByTransformationMatrix(headTransformation);

	return headPose;
}

