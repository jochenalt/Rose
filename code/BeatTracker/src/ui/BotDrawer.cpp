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

#include "uiconfig.h"
#include "basics/util.h"
#include "basics/spatial.h"
#include "Stewart/Kinematics.h"

void BotDrawer::displayBot(const Pose & bodyPose, const Point& eyeDeviation ) {
	glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();

	// glLoadIdentity();             // Reset the model-view matrix to world coordinate system
	glRotatef(-90, 1.0,0.0,0.0);
	glRotatef(-90, 0.0,0.0,1.0);

	glTranslatef(bodyPose.position.x, bodyPose.position.y,bodyPose.position.z);

	// rotate in zyx convention, as used in Kinematics::RotationMatrix
	glRotatef(degrees(bodyPose.orientation.z), 0.0,0.0,1.0);
	glRotatef(degrees(bodyPose.orientation.y), 0.0,1.0,0.0);
	glRotatef(degrees(bodyPose.orientation.x), 1.0,0.0,0.0);

	glTranslatef(0,0,100);

	glPushMatrix();
	glRotatef(90, 0.0, 1.0, 0.0 );
	glRotatef(90, 0.0, 0.0, 1.0 );
	body.display(glEyesColor,glEyesColor);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(eyeDeviation.x, eyeDeviation.y, eyeDeviation.z);
	glutSolidSphere(6, 18, 18);
	glPopMatrix();

	double distance = eyeDeviation.length();
	double factor = 20.0/distance;
	glTranslatef(0, eyeDeviation.y*factor, eyeDeviation.z*factor);
	glRotatef(90, 0.0, 1.0, 0.0 );
	glRotatef(90, 0.0, 0.0, 1.0 );

	eyes.display(glEyesColor,glEyesColor);
	glPopMatrix();
	glPopAttrib();
}

void BotDrawer::displayStewart(const Pose & bodyPose) {
	glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();

	glLoadIdentity();             // Reset the model-view matrix to world coordinate system
	glRotatef(-90, 1.0,0.0,0.0);
	glRotatef(-90, 0.0,0.0,1.0);

	glPushMatrix();
	glRotatef(180, 0.0, 1.0, 0.0 );
	glRotatef(180, 0.0, 0.0, 1.0 );
	stewartBase.display(glEyesColor,glEyesColor);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(bodyPose.position.x, bodyPose.position.y,bodyPose.position.z);
	// rotate in zyx convention, as used in Kinematics::RotationMatrix
	glRotatef(degrees(bodyPose.orientation.z), 0.0,0.0,1.0);
	glRotatef(degrees(bodyPose.orientation.y), 0.0,1.0,0.0);
	glRotatef(degrees(bodyPose.orientation.x), 1.0,0.0,0.0);
	glRotatef(-90, 0.0, 0.0, 1.0 );
	stewartPlate.display(glEyesColor,glEyesColor);
	glPopMatrix();

	Point ballJoint_world[6];
	double servoAngles_rad[6];
	Point servoBallJoints_world[6];
	Point servoArmCentre_world[6];

	Kinematics::getInstance().getServoArmCentre(servoArmCentre_world);
	Kinematics::getInstance().computeServoAngles(bodyPose, servoAngles_rad, ballJoint_world, servoBallJoints_world );

	for (int i = 0;i<6;i++) {
		// render the servo arm
		glPushMatrix();
		glTranslatef(servoArmCentre_world[i].x, servoArmCentre_world[i].y,servoArmCentre_world[i].z);
		glRotatef(int(i/2)*120,0.0,0.0,1.0);

		double angle = degrees(servoAngles_rad[i]);
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
		glTranslatef(servoBallJoints_world[i].x, servoBallJoints_world[i].y,servoBallJoints_world[i].z);
		Point translation = ballJoint_world[i]- servoBallJoints_world[i];


		// compute rotation out of two points
		double lenXY = sqrt(sqr(translation.x) + sqr(translation.y));

		double zRotation = atan2(translation.y, translation.x);
		double xRotation = atan2(lenXY, translation.z);

		glRotatef(degrees(zRotation), 0.0,0.0,1.0);
		glRotatef(degrees(xRotation), 0.0,1.0,0.0);
		// glRotatef(30 + degrees(0*xRotation), 0.0,0.0,1.0);

		// glRotatef(degrees(xRotation), 1.0,0.0,0.0);
		stewartRod.display(glStewartRodColor,glStewartRodColor);
		glPopMatrix();

		/*
		glPushMatrix();
		glTranslatef(ballJoint_world[i].x, ballJoint_world[i].y,ballJoint_world[i].z);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glBallJointColor);
		glColor3fv(glBallJointColor);
		glutSolidSphere(8, 18, 18);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(servoBallJoints_world[i].x, servoBallJoints_world[i].y,servoBallJoints_world[i].z);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glServoCentreColor);
		glColor3fv(glServoCentreColor);
		glutSolidSphere(6, 18, 18);
		glPopMatrix();
		*/
	}

	glPopMatrix();
	glPopAttrib();
}


void BotDrawer::readSTLFiles(string path) {
	eyes.loadFile(path + "/Eyes.stl");
	body.loadFile(path + "/Body.stl");

	stewartBase.loadFile(path + "/Stewart-Platform-Base.stl");
	stewartPlate.loadFile(path + "/Stewart-Plate.stl");
	stewartServoArm.loadFile(path + "/Stewart-Servo-Arm.stl");
	stewartRod.loadFile(path + "/Stewart-Rod.stl");


}

void BotDrawer::setup() {
	static bool setupDone = false;
	if (!setupDone) {
		// search for stl files
		if (fileExists("./stl/Eyes.stl")) {
			readSTLFiles("./stl");
		} else {
			if (fileExists("./Eyes.stl"))
				readSTLFiles("./");
			if (fileExists("../../stl/Eyes.stl"))
				readSTLFiles("../../stl");
			else
				if (fileExists("../../../stl/Eyes.stl"))
					readSTLFiles("../../../stl");
		}
		setupDone = true;
	}
}


