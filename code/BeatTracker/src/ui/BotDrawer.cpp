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

	// glLoadIdentity();             // Reset the model-view matrix to world coordinate system
	glRotatef(-90, 1.0,0.0,0.0);
	glRotatef(-90, 0.0,0.0,1.0);

	glPushMatrix();
	glRotatef(180, 0.0, 1.0, 0.0 );
	glRotatef(0, 0.0, 0.0, 1.0 );
	stewartBase.display(glEyesColor,glEyesColor);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(bodyPose.position.x, bodyPose.position.y,bodyPose.position.z);

	// rotate in zyx convention, as used in Kinematics::RotationMatrix
	glRotatef(degrees(bodyPose.orientation.z), 0.0,0.0,1.0);
	glRotatef(degrees(bodyPose.orientation.y), 0.0,1.0,0.0);
	glRotatef(degrees(bodyPose.orientation.x), 1.0,0.0,0.0);
	stewartPlate.display(glEyesColor,glEyesColor);
	glPopMatrix();


	glPopMatrix();
	glPopAttrib();
}


void BotDrawer::readSTLFiles(string path) {
	eyes.loadFile(path + "/Eyes.stl");
	body.loadFile(path + "/Body.stl");

	stewartBase.loadFile(path + "/Stewart-Platform-Base.stl");
	stewartPlate.loadFile(path + "/Stewart-Plate.stl");


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


