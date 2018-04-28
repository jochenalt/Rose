/*
 * Kinematics.cpp
 *
 *  Created on: Feb 20, 2018
 *      Author: jochenalt
 */

#include <assert.h>
#include <basics/util.h>
#include <basics/spatial.h>

#include <stewart/StewartKinematics.h>


StewartKinematics::StewartKinematics() {
}

StewartKinematics::~StewartKinematics() {
}


void StewartKinematics::setup(StewartConfiguration newConfig) {
	config = newConfig;
	// compute the centres of all servos and the centres of all ball joints of the plate
	double zRotation;
	currentMaxSpeed = 0;
	Pose servoCentre[6];
	for (int i = 0;i<3;i++) {
		zRotation = i*radians(120.0);
		assert(i*2+1 < 6);
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

		plateBallJoint[i*2]   = Point(config.plateJointRadius_mm,0,-config.plateBallJointHeight_mm).getRotatedAroundZ(radians(120)*i + config.plateJointAngle_rad);
		plateBallJoint[i*2+1] = Point(config.plateJointRadius_mm,0,-config.plateBallJointHeight_mm).getRotatedAroundZ(radians(120)*i - config.plateJointAngle_rad);

		lastAngle[i*2] = 0;
		lastAngle[i*2+1] = 0;

	}

	// precompute inverse transformation matrixes of the servos
	// later on used to convert balljoints to servo frame
	for (int i = 0;i<6;i++) {
		assert(i<6);
		HomogeneousMatrix servoTransformation = createTransformationMatrix(servoCentre[i]);
		servoCentreTransformationInv[i] = computeInverseTransformationMatrix(servoTransformation);

		// precompute transformation from
		servoTransform[i] = createTransformationMatrix(servoCentre[i]);
	}
}

void StewartKinematics::getServoArmCentre(Point servoArmCentre_world[6]) {
	for (int i = 0;i<6;i++) {
		servoArmCentre_world[i] = servoArmCentre[i];
	}
}

double StewartKinematics::computeServoAngle(int cornerNo, const Point& ballJoint_world) {

	// transform plate's balljoint  into frame of the servo by multiplying the inverse transformation matrix to the balljoint
	HomogeneousVector balljoint_world_hom = {
			ballJoint_world.x,
			ballJoint_world.y,
			ballJoint_world.z,
			1.0 };

	// this is the ballJoint from the servos perspective
	assert(cornerNo<6);
	Point ballJoint_servoframe =  servoCentreTransformationInv[cornerNo] * balljoint_world_hom;

	// mirror the y axis for odd servo numbers
	if (mirrorFrame(cornerNo))
		ballJoint_servoframe.y = -ballJoint_servoframe.y;


	// got this formula from diploma thesis
	double lenSqr = ballJoint_servoframe.lengthSqr();
	double lengthXYPlane = sqrt (sqr(ballJoint_servoframe.y) + sqr(ballJoint_servoframe.z));
	double c = (sqr(config.servoArmLength_mm)  - sqr(config.rodLength_mm)  + lenSqr)/(2.0*config.servoArmLength_mm*lengthXYPlane);
	double angle_rad = asin(c) - atan2(ballJoint_servoframe.y, ballJoint_servoframe.z);

	// check the limizs
	if ((angle_rad > 0) && (abs(angle_rad) > config.topServoLimit_rad))
		cout << std::fixed << std::setprecision(2) << config.name << " servo " << cornerNo << " above limit of " << degrees(config.topServoLimit_rad) << "° (" << abs(degrees(angle_rad)) << ")" << endl;
	if ((angle_rad < 0) && (angle_rad < config.bottomServoLimit_rad))
		cout << std::fixed << std::setprecision(2) << config.name << " servo " << cornerNo << " below limit of " << degrees(config.bottomServoLimit_rad) << "° (" << degrees(angle_rad) << ")" << endl;
	return angle_rad;
}

void StewartKinematics::resetSpeedMeasurement() {
	currentMaxSpeed = 0;
}

void StewartKinematics::computeServoAngles(const Pose& plate_world, double servoAngle_rad[6], Point ballJoint_world[6],  Point servoBallJoint_world[6]) {
	// compute the plate's ball joint coordinates in world coordinate
	HomogeneousMatrix plateTransformation(4,4);
	createTransformationMatrix(plate_world,plateTransformation);

	Point plateBallJoints_world[6];
	double dT = timer.dT();

	for (int i = 0;i<6;i++) {
		Point currPlateBallJoint = plateBallJoint[i];
		HomogeneousVector ballJoint_plate_hom = getHomogeneousVector(currPlateBallJoint);

		HomogeneousVector currBallJoint_world_vec = plateTransformation * ballJoint_plate_hom;

		Point currBallJoint_world(currBallJoint_world_vec);
		ballJoint_world[i] = currBallJoint_world;

		double angle_rad = computeServoAngle(i,currBallJoint_world);
		servoAngle_rad[i] = angle_rad;

		// compute max speed with weak low pass
		if (dT > floatPrecision) {
			double speed = abs(angle_rad-lastAngle[i])/dT;
			if (speed > currentMaxSpeed) {
				currentMaxSpeed = currentMaxSpeed*0.5 +  0.5*degrees(speed)/60.0;
			}

		}

		lastAngle[i] = angle_rad;

		Pose servoArm_rotation(Pose(Point(0,0,0), Rotation(angle_rad,0,0)));
		if (mirrorFrame(i))
			servoArm_rotation.orientation.x = -servoArm_rotation.orientation.x;
		HomogeneousMatrix servoArmRotationTrans;
		createTransformationMatrix(servoArm_rotation, servoArmRotationTrans);

		Pose servoArm_translation(Pose(Point(0,config.servoArmLength_mm,0), Rotation(0,0,0)));
		if (mirrorFrame(i))
			servoArm_translation.position.y = -servoArm_translation.position.y;

		// translation of servo lever
		HomogeneousMatrix servoArmTranslationTrans;
		createTransformationMatrix(servoArm_translation, servoArmTranslationTrans);

		// compute end point of servo ball joints by concatenating servo centre
		// position -> rotation by servo angle -> translation by servo length
		HomogeneousMatrix servoBallJoint = servoTransform[i] * servoArmRotationTrans * servoArmTranslationTrans ;
		Point currServoBallJoint_world = getPointByTransformationMatrix(servoBallJoint);
		servoBallJoint_world[i] = currServoBallJoint_world;
	}

}
