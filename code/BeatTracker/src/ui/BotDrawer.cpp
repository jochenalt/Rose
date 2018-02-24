/*
 * BotDrawer.cpp
 *
 * Author: JochenAlt
 */


#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include "basics/stringhelper.h"
#include <BotDrawer.h>
#include <Stewart/StewartKinematics.h>
#include <Stewart/BodyKinematics.h>


#include "uiconfig.h"
#include "basics/util.h"
#include "basics/spatial.h"


void BotDrawer::displayBot(const Pose & bodyPose, const Pose& headPose) {
	glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();

	glLoadIdentity();             // Reset the model-view matrix to world coordinate system
	glRotatef(-90, 1.0,0.0,0.0);
	glRotatef(-90, 0.0,0.0,1.0);

	glPushMatrix();
	glRotatef(180, 0.0, 1.0, 0.0 );
	glRotatef(180, 0.0, 0.0, 1.0 );
	stewartBase.display(glStewartPlateColor,glStewartPlateColor);
	glPopMatrix();


	Point bodyBallJoint_world[6];
	Point headBallJoint_world[6];

	double bodyServoAngles_rad[6];
	double headServoAngles_rad[6];

	Point bodyServoBallJoints_world[6];
	Point headServoBallJoints_world[6];

	Point bodyServoArmCentre_world[6];
	Point headServoArmCentre_world[6];

	BodyKinematics::getInstance().
			computeServoAngles(bodyPose, bodyServoArmCentre_world, bodyServoAngles_rad, bodyBallJoint_world, bodyServoBallJoints_world,
			                   headPose, headServoArmCentre_world, headServoAngles_rad, headBallJoint_world, headServoBallJoints_world);

	for (int i = 0;i<6;i++) {
		// render the servo arm
		glPushMatrix();
		glTranslatef(bodyServoArmCentre_world[i].x, bodyServoArmCentre_world[i].y,bodyServoArmCentre_world[i].z);
		glRotatef(int(i/2)*120,0.0,0.0,1.0);

		double angle = degrees(bodyServoAngles_rad[i]);
		if (i % 2 == 0)
			glRotatef(180.0 + angle, 1.0,0.0,0.0);
		else
			glRotatef(-angle, 1.0,0.0,0.0);

		glRotatef(-90, 1.0,0.0,0.0);
		glRotatef(90, 0.0,1.0,0.0);
		stewartServoArm.display(glServoArmColor,glServoArmColor);
		glPopMatrix();

		// render the rod between servo and top plate
		glPushMatrix();
		glTranslatef(bodyServoBallJoints_world[i].x, bodyServoBallJoints_world[i].y,bodyServoBallJoints_world[i].z);
		Point translation = bodyBallJoint_world[i]- bodyServoBallJoints_world[i];

		// compute rotation out of two points
		double lenXY = sqrt(sqr(translation.x) + sqr(translation.y));
		double zRotation = atan2(translation.y, translation.x);
		double xRotation = atan2(lenXY, translation.z);

		glRotatef(degrees(zRotation), 0.0,0.0,1.0);
		glRotatef(degrees(xRotation), 0.0,1.0,0.0);
		stewartRod.display(glStewartRodColor,glStewartRodColor);
		glPopMatrix();
	}
	glPushMatrix();

	// draw body plate
	glTranslatef(bodyPose.position.x, bodyPose.position.y,bodyPose.position.z);
	glRotatef(degrees(bodyPose.orientation.z), 0.0,0.0,1.0);
	glRotatef(degrees(bodyPose.orientation.y), 0.0,1.0,0.0);
	glRotatef(degrees(bodyPose.orientation.x), 1.0,0.0,0.0);
	stewartPlate.display(glStewartPlateColor,glStewartPlateColor);

	// we are in the frame of the plate, so draw servos of second stewart platform
	for (int i = 0;i<6;i++) {
		// render the servo arm
		glPushMatrix();
		glTranslatef(headServoArmCentre_world[i].x, headServoArmCentre_world[i].y,headServoArmCentre_world[i].z);
		glRotatef(int(i/2)*120,0.0,0.0,1.0);
		double angle = degrees(headServoAngles_rad[i]);
		if (i % 2 == 0)
			glRotatef(180.0 + angle, 1.0,0.0,0.0);
		else
			glRotatef(-angle, 1.0,0.0,0.0);
		glRotatef(-90, 1.0,0.0,0.0);
		glRotatef(90, 0.0,1.0,0.0);
		stewartSmallServoArm.display(glServoArmColor,glServoArmColor);
		glPopMatrix();
		// render the rod between servo and top plate
		glPushMatrix();
		glTranslatef(headServoBallJoints_world[i].x, headServoBallJoints_world[i].y,headServoBallJoints_world[i].z);
		Point translation = headBallJoint_world[i]- headServoBallJoints_world[i];

		// compute rotation out of two points
		double lenXY = sqrt(sqr(translation.x) + sqr(translation.y));
		double zRotation = atan2(translation.y, translation.x);
		double xRotation = atan2(lenXY, translation.z);

		glRotatef(degrees(zRotation), 0.0,0.0,1.0);
		glRotatef(degrees(xRotation), 0.0,1.0,0.0);
		stewartSmallRod.display(glStewartRodColor,glStewartRodColor);
		glPopMatrix();
	}


	// draw head plate (headPose is relative to the bodyPose)
	glTranslatef(headPose.position.x, headPose.position.y,headPose.position.z);
	glRotatef(degrees(headPose.orientation.z), 0.0,0.0,1.0);
	glRotatef(degrees(headPose.orientation.y), 0.0,1.0,0.0);
	glRotatef(degrees(headPose.orientation.x), 1.0,0.0,0.0);
	stewartHead.display(glStewartPlateColor,glStewartPlateColor);

	// move along the neck in z
	glTranslatef(25,0,45);
	glRotatef(90,1.0,0,0);
	glRotatef(90,0,1.0,0.0);

	chickenHead.display(glChickenHeadColor,glChickenHeadColor);
	chickenBeak.display(glChickenBeak,glChickenBeak);
	chickenEyeBall.display(glChickenEyeBallsColor,glChickenEyeBallsColor);
	chickenEyeIris.display(glChickenEyeIrisColor,glChickenEyeIrisColor);
	glPopMatrix();


	glPopMatrix();
	glPopAttrib();
}


void BotDrawer::readSTLFiles(string path) {
	chickenHead.loadFile(path + "/Chickenhead-Head.stl");
	chickenEyeBall.loadFile(path + "/Chickenhead-Eyeball.stl");
	chickenEyeIris.loadFile(path + "/Chickenhead-Iris.stl");
	chickenBeak.loadFile(path + "/Chickenhead-Beak.stl");

	stewartBase.loadFile(path + "/Stewart-Platform-Base.stl");
	stewartPlate.loadFile(path + "/Stewart-Plate.stl");
	stewartServoArm.loadFile(path + "/Stewart-Servo-Arm.stl");
	stewartRod.loadFile(path + "/Stewart-Rod.stl");

	stewartSmallRod.loadFile(path + "/Stewart-Small-Rod.stl");
	stewartHead.loadFile(path + "/Stewart-Head.stl");
	stewartSmallServoArm.loadFile(path + "/Stewart-Small-Servo-Arm.stl");

}

void BotDrawer::setup() {
	static bool setupDone = false;
	if (!setupDone) {
		// search for stl files
		if (fileExists("./stl/Chickenhead-Beak.stl")) {
			readSTLFiles("./stl");
		} else {
			if (fileExists("./Chickenhead-Beak.stl"))
				readSTLFiles("./");
			if (fileExists("../../stl/Chickenhead-Beak.stl"))
				readSTLFiles("../../stl");
			else
				if (fileExists("../../../stl/Chickenhead-Beak.stl"))
					readSTLFiles("../../../stl");
		}
		setupDone = true;
	}
}


