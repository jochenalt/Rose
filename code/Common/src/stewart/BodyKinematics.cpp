/*
 * BodyKinematics.cpp
 *
 *  Created on: Feb 23, 2018
 *      Author: jochenalt
 */

#include <stewart/BodyKinematics.h>


static StewartConfiguration bodyStewartConfig = {"body",
										  27.354, 				// servoCentreRadius_mm
										  radians(12.83),		// servoCentreAngle_rad
										  35.211, 				// servoArmCentreRadius_mm (for rendering only)
										  radians(9.93),		// servoArmCentreAngle_mm (for rendering only)
										  28.036,				// plateJointRadius_mm
										  radians(8.2),  		// plateJointAngle_rad
										  74.0,   				// rodLength_mm
										  29.478,				// servoArmLength_mm
										  31.2,					// servoCentreHeight_mm
										  -4.0,					// plateBallJointHeight_mm
										  65.0, 				// bottomPlatformRadius_mm
										  50.0, 				// topPlatformRadiusX_mm
										  75.0, 				// topPlatformRadiusY_mm
										  -radians(56.0),		// topServoLimit_rad
									      radians(86.0)			// bottomServoLimit_rad
};


BodyKinematics& BodyKinematics::getInstance() {
	static BodyKinematics instance;
	return instance;
}

StewartConfiguration& BodyKinematics::getStewartConfig() {
	return bodyStewartConfig;
}


BodyKinematics::BodyKinematics() {
}

BodyKinematics::~BodyKinematics() {
}


void BodyKinematics::getPlatformMetrics(double& basePlatformRadius, double& topPlatformRadiusX, double& topPlatformRadiusY, double& rodLength) {
	basePlatformRadius = bodyStewartConfig.bottomPlatformRadius_mm;
	topPlatformRadiusX = bodyStewartConfig.topPlatformRadiusX_mm;
	topPlatformRadiusY = bodyStewartConfig.topPlatformRadiusY_mm;

	rodLength = bodyStewartConfig.rodLength_mm;

}

void BodyKinematics::setup() {
	headKin.setup(bodyStewartConfig);

}

void BodyKinematics::computeServoAngles(const Pose& headPose, Point headServoArmCentre_world[6], double headServoAngle_rad[6], Point headBallJoint_world[6],  Point headServoBallJoint_world[6]) {
	headKin.getServoArmCentre(headServoArmCentre_world);
	headKin.computeServoAngles(headPose, headServoAngle_rad, headBallJoint_world,  headServoBallJoint_world);
}

void BodyKinematics::getServoArmCentre(Point servoArmCentre_world[6]) {
	headKin.getServoArmCentre(servoArmCentre_world);
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

Pose BodyKinematics::translateOrientation(const Pose& bodyPose, const Point centre) {

	HomogeneousMatrix centreTrans(4,4);
	centreTrans = createTransformationMatrix(Pose(centre,Rotation()));

	HomogeneousMatrix rotationTrans(4,4);
	rotationTrans = createTransformationMatrix(Pose(Point(),bodyPose.orientation));

	HomogeneousMatrix translationTrans(4,4);
	translationTrans = createTransformationMatrix(Pose(centre - bodyPose.position, Rotation()));
	HomogeneousMatrix invTranslationTrans(4,4);
	invTranslationTrans = computeInverseTransformationMatrix(translationTrans);
	HomogeneousMatrix result(4,4);
	result = centreTrans*rotationTrans*invTranslationTrans;
	return getPoseByTransformationMatrix(result);
}

