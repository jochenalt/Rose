/*
 * BotDrawer.cpp
 *
 * Author: JochenAlt
 */


#include <ui/BotRenderer.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include "basics/stringhelper.h"
#include <Stewart/StewartKinematics.h>
#include <Stewart/BodyKinematics.h>


#include "uiconfig.h"
#include "basics/util.h"
#include "basics/spatial.h"


void BotRenderer::displayBot(const Pose& headPose) {
	glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();

	Point headBallJoint_world[6];
	double headServoAngles_rad[6];
	Point headServoBallJoints_world[6];
	Point headServoArmCentre_world[6];

	BodyKinematics::getInstance().computeServoAngles(headPose, headServoArmCentre_world, headServoAngles_rad, headBallJoint_world, headServoBallJoints_world);

	glLoadIdentity();             // Reset the model-view matrix to world coordinate system
	glRotatef(-90, 1.0,0.0,0.0);
	glRotatef(-90, 0.0,0.0,1.0);
	baseStewart.display(glStewartPlateColor,glStewartPlateColor);

	glPushMatrix();
	// draw head plate (headPose is relative to the bodyPose)
	glTranslatef(headPose.position.x, headPose.position.y,headPose.position.z);
	glRotatef(degrees(headPose.orientation.z), 0.0,0.0,1.0);
	glRotatef(degrees(headPose.orientation.y), 0.0,1.0,0.0);
	glRotatef(degrees(headPose.orientation.x), 1.0,0.0,0.0);
	topPlatform.display(glStewartPlateColor,glStewartPlateColor);

	// draw head
	servoBlock.display(glHeadColor,glHeadColor);
	mouthServoArmUpper.display(glEyeBallsColor,glEyeBallsColor);
	mouthServoArmUpper.display(glEyeBallsColor,glEyeBallsColor);

	lowerLip.display(glIrisColor,glIrisColor);
	upperLip.display(glIrisColor,glIrisColor);
	mouthLever.display(glIrisColor,glIrisColor);

	glPopMatrix();


	// current frame is body plate, now draw servo arms towards of head plate
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

		baseStewartServoArm.display(glServoArmColor,glServoArmColor);
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
		baseStewartRod.display(glStewartRodColor,glStewartRodColor);
		glPopMatrix();
	}

	glPopMatrix();

	double r1, r2X,r2Y, h;
	BodyKinematics::getInstance().getPlatformMetrics(r1,r2X, r2Y, h);
	body.set(r1, r2X, r2Y,h); //

	// draw body as flexible volume of revolution along a bezier curve
	switch (clothingMode) {
		case NORMAL_MODE: body.display(Pose(),headPose, glBodyColor, glBodyColor, glGridColor); break;
		case TRANSPARENT_MODE: body.display(Pose(), headPose, glTranspBodyColor1, glTranspBodyColor2, glTranspGridColor); break;
		default:
			break;
	}

	glPopMatrix(); // restore old model matrix
	glPopAttrib(); // restore old color
}


void BotRenderer::readSTLFiles(string path) {
	// read in Stewart Platform STLs
	baseStewart.loadFile(path + "/BottomPlatform.stl");
	baseStewartRod.loadFile(path + "/BaseStewartRod.stl");
	baseStewartServoArm.loadFile(path + "/BaseStewartServoArm.stl");
	topPlatform.loadFile(path + "/TopPlatform.stl");


	// read in mouth STLs
	servoBlock.loadFile(path + "/MouthServoBlock.stl");
	mouthServoArmUpper.loadFile(path + "/MouthServoArmUpperLip.stl");
	mouthServoArmLower.loadFile(path + "/MouthServoArmBottomLip.stl");

	lowerLip.loadFile(path + "/BottomLip.stl");
	upperLip.loadFile(path + "/UpperLip.stl");
	mouthLever.loadFile(path + "/MouthLever.stl");


}

void BotRenderer::setup() {
	static bool setupDone = false;
	if (!setupDone) {
		// search for stl files
		if (fileExists("./stl/BottomPlatform.stl")) {
			readSTLFiles("./stl");
		} else {
			if (fileExists("./BottomPlatform.stl"))
				readSTLFiles("./");
			if (fileExists("../../stl/BottomPlatform.stl"))
				readSTLFiles("../../stl");
			else
				if (fileExists("../../../stl/BottomPlatform.stl"))
					readSTLFiles("../../../stl");
		}
		setupDone = true;
	}
}


