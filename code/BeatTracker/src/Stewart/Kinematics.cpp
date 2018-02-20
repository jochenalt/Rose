/*
 * Kinematics.cpp
 *
 *  Created on: Feb 20, 2018
 *      Author: jochenalt
 */

#include <basics/util.h>
#include <basics/spatial.h>

#include <Stewart/Kinematics.h>


StewartConfiguration hipStewartConfig = { 46.5, radians(12.4),
		                                  7.7, radians(40.0),
											106.0, 52.0 };


Kinematics::Kinematics() {
}

Kinematics::~Kinematics() {
}


void Kinematics::setup() {
	// compute the servo centre for all
	config = hipStewartConfig;

	// compute the centres of all servos and the centres of all ball joints of the plate
	for (int i = 0;i<3;i++) {
		servoCentre[i*2]     = Point(0,0,config.servoCentreRadius_mm).getRotatedAroundY(radians(120)*i + config.servoCentreAngle_rad);
		servoCentre[i*2+1]   = Point(0,0,config.servoCentreRadius_mm).getRotatedAroundY(radians(120)*i - config.servoCentreAngle_rad);

		plateBalJoint[i*2]   = Point(0,0,config.plateJointRadius_mm).getRotatedAroundY(radians(120)*i + config.plateJointAngle_rad);
		plateBalJoint[i*2+1] = Point(0,0,config.plateJointRadius_mm).getRotatedAroundY(radians(120)*i - config.plateJointAngle_rad);
	}
}

double Kinematics::computeServoAngle(int cornerNo, const Point& ballJoint_world) {
	// transform balljoint coordinates into coordinate system of the servo

	HomogeneousMatrix servoTransformation;
	HomogeneousMatrix inverseServoTransformation;

	createTransformationMatrix(servoCentre[cornerNo], servoTransformation);
	computeInverseTransformationMatrix(servoTransformation, inverseServoTransformation);

	HomogeneousVector balljoint_world_hom = {
			ballJoint_world.x,
			ballJoint_world.y,
			ballJoint_world.z,
			1.0 };

	Point ballJoint_servoframe = inverseServoTransformation * balljoint_world_hom;

	double lenSqr = ballJoint_servoframe.lengthSqr();

	double xyLen = sqrt (sqr(ballJoint_servoframe.x) + sqr(ballJoint_servoframe.y));
	double c = (sqr(config.servoArmLength_mm)  - sqr(config.rodLength_mm)  + lenSqr)/(2.0*config.servoArmLength_mm*xyLen);
	double angle_rad = asin(c) - atan2(ballJoint_servoframe.x, ballJoint_servoframe.y);

	return angle_rad;
}


void Kinematics::computeServoAngles(const Pose& plate, Point ballJoint_world[6], double servoAngle_rad[6], Point servoBallJoint_world[6]) {
	// commpute the plate's ball joint coordinates in world coordinate

	HomogeneousMatrix plateTransformation;
	createTransformationMatrix(plate, plateTransformation);

	Point plateBallJoints_world[6];
	for (int i = 0;i<6;i++) {
		HomogeneousVector ballJoint_plate_hom = {
				plateBalJoint[i].x,
				plateBalJoint[i].y,
				plateBalJoint[i].z,
				1.0 };

		ballJoint_world[i] = plateTransformation * ballJoint_plate_hom;

		servoAngle_rad[i] = computeServoAngle(i,ballJoint_world[i] );
		servoBallJoint_world[i] = servoCentre[i].getTranslated(Point(config.servoArmLength_mm,0,0).getRotatedAroundZ(servoAngle_rad[i]));
	}


}
