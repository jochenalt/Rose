/*
 * Kinematics.cpp
 *
 *  Created on: Feb 20, 2018
 *      Author: jochenalt
 */

#include <basics/util.h>
#include <basics/spatial.h>

#include <Stewart/Kinematics.h>


StewartConfiguration hipStewartConfig = { 46.5, 				// servoCentreRadius_mm
										  radians(12.4),		// servoArmCentreRadius_mm
										  50.4, 				// servoArmCentreRadius_mm
										  radians(11.5),		// servoArmCentreAngle_mm
										  40.0,					// plateJointRadius_mm
										  radians(7.7),			// plateJointAngle_rad
										  106.3,				// rodLength_mm
										  52.0,					// servoArmLength_mm
										  30.6					// servoCentreHeight_mm
									  	  };

Kinematics::Kinematics() {
}

Kinematics::~Kinematics() {
}


void Kinematics::setup() {
	// compute the servo centre for all
	config = hipStewartConfig;

	// compute the centres of all servos and the centres of all ball joints of the plate
	double zRotation;
	for (int i = 0;i<3;i++) {
		zRotation = i*radians(120.0);
		servoCentre[i*2]     = Pose(Point(config.servoCentreRadius_mm,0,config.servoCentreHeight_mm)
				                      .getRotatedAroundZ(zRotation + config.servoCentreAngle_rad),
				                    Rotation(0,0,zRotation));
		servoCentre[i*2+1]   = Pose(Point(config.servoCentreRadius_mm,0,config.servoCentreHeight_mm)
				                      .getRotatedAroundZ(zRotation - config.servoCentreAngle_rad),
				                    Rotation(0,0,zRotation));

		servoArmCentre[i*2]  = Point(config.servoArmCentreRadius_mm,0,config.servoCentreHeight_mm)
				                  .getRotatedAroundZ(zRotation + config.servoArmCentreAngle_mm);
		servoArmCentre[i*2+1]= Point(config.servoArmCentreRadius_mm,0,config.servoCentreHeight_mm)
				                  .getRotatedAroundZ(zRotation - config.servoArmCentreAngle_mm);

		plateBallJoint[i*2]   = Point(config.plateJointRadius_mm,0,0).getRotatedAroundZ(radians(120)*i + config.plateJointAngle_rad);
		plateBallJoint[i*2+1] = Point(config.plateJointRadius_mm,0,0).getRotatedAroundZ(radians(120)*i - config.plateJointAngle_rad);
	}

	// precompute inverse transformation matrixes of the servos
	// later on used to convert balljoints to servo frame
	for (int i = 0;i<6;i++) {
		HomogeneousMatrix servoTransformation = createTransformationMatrix(servoCentre[i]);
		computeInverseTransformationMatrix(servoTransformation, servoCentreTransformationInv[i]);
	}
}

void Kinematics::getServoArmCentre(Point servoArmCentre_world[6]) {
	for (int i = 0;i<6;i++) {
		servoArmCentre_world[i] = servoArmCentre[i];
	}
}

double Kinematics::computeServoAngle(int cornerNo, const Point& ballJoint_world) {

	// transform plate's balljoint  into frame of the servo by multiplying the inverse transformation matrix to the balljoint
	HomogeneousVector balljoint_world_hom = {
			ballJoint_world.x,
			ballJoint_world.y,
			ballJoint_world.z,
			1.0 };

	// this is the ballJoint from the servos perspective
	Point ballJoint_servoframe =  servoCentreTransformationInv[cornerNo] * balljoint_world_hom;

	// mirror the y axis for odd servo numbers
	if (mirrorFrame(cornerNo))
		ballJoint_servoframe.y = -ballJoint_servoframe.y;


	// got this formula from diploma thesis
	double lenSqr = ballJoint_servoframe.lengthSqr();
	double lengthXYPlane = sqrt (sqr(ballJoint_servoframe.y) + sqr(ballJoint_servoframe.z));
	double c = (sqr(config.servoArmLength_mm)  - sqr(config.rodLength_mm)  + lenSqr)/(2.0*config.servoArmLength_mm*lengthXYPlane);
	double angle_rad = asin(c) - atan2(ballJoint_servoframe.y, ballJoint_servoframe.z);

	return angle_rad;
}

void Kinematics::computeServoAngles(const Pose& plate_world, Point ballJoint_world[6], double servoAngle_rad[6], Point servoBallJoint_world[6]) {
	// compute the plate's ball joint coordinates in world coordinate
	HomogeneousMatrix plateTransformation = createTransformationMatrix(plate_world);

	Point plateBallJoints_world[6];
	for (int i = 0;i<6;i++) {
		Point currPlateBallJoint = plateBallJoint[i];
		HomogeneousVector ballJoint_plate_hom = getHomogeneousVector(currPlateBallJoint);

		Point currBallJoint_world = plateTransformation * ballJoint_plate_hom;

		ballJoint_world[i] = currBallJoint_world;

		double angle_rad = computeServoAngle(i,currBallJoint_world);
		servoAngle_rad[i] = angle_rad;

		HomogeneousMatrix servoTransform = createTransformationMatrix(servoCentre[i]);
		Pose servoArm_rotation(Pose(Point(0,0,0), Rotation(angle_rad,0,0)));
		if (mirrorFrame(i))
			servoArm_rotation.orientation.x = -servoArm_rotation.orientation.x;
		HomogeneousMatrix servoArmRotationTrans = createTransformationMatrix(servoArm_rotation);

		Pose servoArm_translation(Pose(Point(0,config.servoArmLength_mm,0), Rotation(0,0,0)));
		if (mirrorFrame(i))
			servoArm_translation.position.y = -servoArm_translation.position.y;

		HomogeneousMatrix servoArmTranslationTrans = createTransformationMatrix(servoArm_translation);
		HomogeneousMatrix servoBallJoint = servoTransform * servoArmRotationTrans * servoArmTranslationTrans ;
		Point currServoBallJoint_world = getPointByTransformationMatrix(servoBallJoint);
		servoBallJoint_world[i] = currServoBallJoint_world;
	}
}
