/*
 * BodyKinematics.h
 *
 *  Created on: Feb 23, 2018
 *      Author: jochenalt
 */

#ifndef SRC_STEWART_BODYKINEMATICS_H_
#define SRC_STEWART_BODYKINEMATICS_H_

#include <assert.h>
#include "basics/point.h"
#include "basics/spatial.h"
#include "StewartKinematics.h"

class BodyKinematics {
public:
	BodyKinematics();
	virtual ~BodyKinematics();
	void setup();
	static BodyKinematics& getInstance();

	void computeServoAngles(const Pose& headPose, Point headServoArmCentre_world[6], double headServoAngle_rad[6], Point headBallJoint_world[6],  Point headServoBallJoint_world[6]);
	void getServoArmCentre(Point servoArmCentre_world[6]);

	// sets the current speed measurement to 0, is called whenever a new move starts
	void resetSpeedMeasurement() { headKin.resetSpeedMeasurement(); };

	// compute relative head pose out of the absolute pose that is projected above the body's belly button
	Pose computeHeadStewartPose(const Pose& bodyPose, const Pose &PoseAboveBellyButton);
	Pose translateOrientation(const Pose& bodyPose, const Point transVector);

	// get static metrics of the platform
	void getPlatformMetrics(double& basePlatformRadius, double& topPlatformRadiusX, double& topPlatformRadiusY,double & rodLength);

	// get static metrics of the mouth
	void getMouthMetrics(double& mouthBockHeight_mm, double& lowerLeverLength_mm);

	// get configuration data of stewart platform
	StewartConfiguration& getStewartConfig();

	// get configuration data of mouth mechanics
	MouthConfiguration& getMouthConfig();

	void computeMouthAngles(const MouthPose& mouth, double &yawServoPose_rad, double &lowerLipServo_rad, double& angleServo_rad);

private:
	BodyKinematics(const BodyKinematics& x) { assert(false); };
	void operator=(const BodyKinematics& x) {assert (false); };
	void operator=(const BodyKinematics& x) const {assert (false); };

	StewartKinematics headKin;

};

#endif /* SRC_STEWART_BODYKINEMATICS_H_ */
