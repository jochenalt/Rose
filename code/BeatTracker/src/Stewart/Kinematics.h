/*
 * Kinematics.h
 *
 *  Created on: Feb 20, 2018
 *      Author: jochenalt
 */

#ifndef SRC_STEWART_KINEMATICS_H_
#define SRC_STEWART_KINEMATICS_H_

#include "basics/point.h"

struct StewartConfiguration {
	double servoCentreRadius_mm;       // from above, distance from centre to this servo's turning centre
	double servoCentreAngle_rad;		// angle we need to turn the in order to come to the centre point

	double servoArmCentreRadius_mm;
	double servoArmCentreAngle_mm;

	double plateJointRadius_mm;		    // distance of a plate's ball joint to the centre
	double plateJointAngle_rad;			// Angle we need to turn from middle axis in order to come to the ball joint

	double rodLength_mm;				// length of the rod between base and plate
	double servoArmLength_mm; 		    // length of the servo lever
	double servoCentreHeight_mm;
};

class Kinematics {
public:
	Kinematics();
	virtual ~Kinematics();
	static Kinematics& getInstance() {
		static Kinematics instance;
		return instance;
	}

	void setup();
	void computeServoAngles(const Pose& plate, Point ballJoint_world[6], double servoAngle_rad[6], Point servoBallJoint_world[6]);
	void getServoArmCentre(Point servoArmCentre_world[6]);

private:
	double computeServoAngle(int cornerNo, const Point& ballJoint);
	bool mirrorFrame(int cornerNo) { return (cornerNo % 2 == 1); };


	Pose servoCentre[6];
	Point servoArmCentre[6];
	Point plateBallJoint[6];

	HomogeneousMatrix servoCentreTransformationInv[6];

	StewartConfiguration config;
};

#endif /* SRC_STEWART_KINEMATICS_H_ */
