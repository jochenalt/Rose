/*
 * Kinematics.cpp
 *
 *  Created on: Feb 20, 2018
 *      Author: jochenalt
 */

#include <basics/util.h>
#include <basics/spatial.h>

#include <Stewart/Kinematics.h>


StewartConfiguration hipStewartConfig = { 46.5,radians(12.4),
										  50.4, radians(11.5),
										  40.0,radians(7.7),
										  106.0,
										  52.0,
										  30.6
									  	  };


Kinematics::Kinematics() {
}

Kinematics::~Kinematics() {
}


void Kinematics::setup() {
	// compute the servo centre for all
	config = hipStewartConfig;

	// compute the centres of all servos and the centres of all ball joints of the plate
	for (int i = 0;i<3;i++) {
		servoCentre[i*2]     = Point(config.servoCentreRadius_mm,0,config.servoCentreHeight_mm).getRotatedAroundZ(radians(120)*i + config.servoCentreAngle_rad);
		servoCentre[i*2+1]   = Point(config.servoCentreRadius_mm,0,config.servoCentreHeight_mm).getRotatedAroundZ(radians(120)*i - config.servoCentreAngle_rad);

		servoArmCentre[i*2]     = Point(config.servoArmCentreRadius_mm,0,config.servoCentreHeight_mm).getRotatedAroundZ(radians(120)*i + config.servoArmCentreAngle_mm);
		servoArmCentre[i*2+1]   = Point(config.servoArmCentreRadius_mm,0,config.servoCentreHeight_mm).getRotatedAroundZ(radians(120)*i - config.servoArmCentreAngle_mm);

		plateBallJoint[i*2]   = Point(config.plateJointRadius_mm,0,0).getRotatedAroundZ(radians(120)*i + config.plateJointAngle_rad);
		plateBallJoint[i*2+1] = Point(config.plateJointRadius_mm,0,0).getRotatedAroundZ(radians(120)*i - config.plateJointAngle_rad);
	}

}

void Kinematics::getServoArmCentre(Point servoArmCentre_world[6]) {
	for (int i = 0;i<6;i++) {
		servoArmCentre_world[i] = servoArmCentre[i];
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

	double yzLen = sqrt (sqr(ballJoint_servoframe.y) + sqr(ballJoint_servoframe.z));
	double c = (sqr(config.servoArmLength_mm)  - sqr(config.rodLength_mm)  + lenSqr)/(2.0*config.servoArmLength_mm*yzLen);
	double angle_rad = asin(c) - atan2(ballJoint_servoframe.y, ballJoint_servoframe.z);

	return angle_rad;
}


void Kinematics::computeServoAngles(const Pose& plate, Point ballJoint_world[6], double servoAngle_rad[6], Point servoBallJoint_world[6]) {
	// commpute the plate's ball joint coordinates in world coordinate

	HomogeneousMatrix plateTransformation;
	createTransformationMatrix(plate, plateTransformation);

	Point plateBallJoints_world[6];
	for (int i = 0;i<6;i++) {
		Point currPlateBallJoint = plateBallJoint[i];
		HomogeneousVector ballJoint_plate_hom = {
				currPlateBallJoint.x,
				currPlateBallJoint.y,
				currPlateBallJoint.z,
				1.0 };

		Point currBallJoint_world = plateTransformation * ballJoint_plate_hom;

		ballJoint_world[i] =currBallJoint_world;

		servoAngle_rad[i] = computeServoAngle(i,ballJoint_world[i] );
		servoBallJoint_world[i] = servoCentre[i].getRotatedAroundX(servoAngle_rad[i]).getTranslated(Point(0,config.servoArmLength_mm,0));
	}


}
