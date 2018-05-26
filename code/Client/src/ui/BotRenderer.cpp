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


void BotRenderer::displayBot(const TotalBodyPose& pose) {

	BodyKinematics& bodyKinematics = BodyKinematics::getInstance();
	glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();

	Point headBallJoint_world[6];
	double headServoAngles_rad[6];
	Point headServoBallJoints_world[6];
	Point headServoArmCentre_world[6];

	// kinematics of stewart platform
	bodyKinematics.computeServoAngles(pose.head, headServoArmCentre_world, headServoAngles_rad, headBallJoint_world, headServoBallJoints_world);

	// kinematics of mouth
	double mouthYawAngle_rad, mouthLowerServoAngle_rad, mouthOpenServoAngle_rad;
	bodyKinematics.computeMouthAngles(pose.mouth, mouthYawAngle_rad, mouthLowerServoAngle_rad, mouthOpenServoAngle_rad);

	glLoadIdentity();             // Reset the model-view matrix to world coordinate system
	glRotatef(-90, 1.0,0.0,0.0);
	glRotatef(-90, 0.0,0.0,1.0);
	baseStewart.display(glStewartPlateColor,glStewartPlateColor);

	glPushMatrix();
	// draw head plate (headPose is relative to the bodyPose)
	glTranslatef(pose.head.position.x, pose.head.position.y,pose.head.position.z);
	glRotatef(degrees(pose.head.orientation.z), 0.0,0.0,1.0);
	glRotatef(degrees(pose.head.orientation.y), 0.0,1.0,0.0);
	glRotatef(degrees(pose.head.orientation.x), 1.0,0.0,0.0);
	topPlatform.display(glStewartPlateColor,glStewartPlateColor);

	// draw block containing all mouth servos
	glPushMatrix();
	glRotatef(degrees(pose.mouth.yaw_rad), 0.0,0.0,1.0);
	servoBlock.display(glHeadColor,glHeadColor);
	glPopMatrix();

	// draw lower lip
	glPushMatrix();
	glTranslatef(0,0,bodyKinematics.getMouthConfig().mouthBaseHeight_mm);
	glRotatef(degrees(mouthLowerServoAngle_rad), 0.0,1.0,0.0);
	mouthServoArmLower.display(glEyeBallsColor,glEyeBallsColor);
	glTranslatef(bodyKinematics.getMouthConfig().lowerLipLeverLength_mm,0,0);
	glRotatef(degrees(-mouthLowerServoAngle_rad + mouthOpenServoAngle_rad), 0.0,1.0,0.0);
	lowerLip.display(glIrisColor,glIrisColor);
	glPopMatrix();


	// draw upper servo lever
	glPushMatrix();
	glTranslatef(0,0,bodyKinematics.getMouthConfig().mouthBaseHeight_mm);
	glRotatef(degrees(mouthOpenServoAngle_rad), 0.0,1.0,0.0);
	mouthServoArmUpper.display(glEyeBallsColor,glEyeBallsColor);
	glTranslatef(0,0,bodyKinematics.getMouthConfig().lowerLipAngleServoArmLength_mm);
	glRotatef(degrees(-mouthOpenServoAngle_rad + mouthLowerServoAngle_rad), 0.0,1.0,0.0);
	mouthLever.display(glIrisColor,glIrisColor);
	glPopMatrix();

	// draw upper lip with the same angle as the lower lip
	glTranslatef(bodyKinematics.getMouthConfig().upperLipX_mm, 0,bodyKinematics.getMouthConfig().upperLipHeight_mm);
	double upperLipAngle_rad = mouthOpenServoAngle_rad;
	if (upperLipAngle_rad > 0)
		upperLipAngle_rad = 0;
	glRotatef(degrees(upperLipAngle_rad), 0.0,1.0,0.0);
	upperLip.display(glIrisColor,glIrisColor);

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
		Point rodTranslation = headBallJoint_world[i]- headServoBallJoints_world[i];

		// to render the rod along rodTranslation, we need to compute the rotation from rod start to rod end
		// we dont care of the z orientation of the rod
		double lenXY = sqrt(sqr(rodTranslation.x) + sqr(rodTranslation.y));
		double zRotation = atan2(rodTranslation.y, rodTranslation.x);
		double xRotation = atan2(lenXY, rodTranslation.z);

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
		case NORMAL_MODE: body.display(Pose(),pose.head, glBodyColor, glBodyColor, glGridColor); break;
		case TRANSPARENT_MODE: body.display(Pose(), pose.head, glTranspBodyColor1, glTranspBodyColor2, glTranspGridColor); break;
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
	mouthServoArmLower.loadFile(path + "/MouthServoArmLowerLip.stl");

	lowerLip.loadFile(path + "/LowerLip.stl");
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


