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

void BotDrawer::displayBot( ) {
	glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();

	// glLoadIdentity();             // Reset the model-view matrix to world coordinate system
	glRotatef(-90, 1.0,0.0,0.0);
	glRotatef(-90, 0.0,0.0,1.0);

	// move to body's centre
	Pose bodyPose (Point(0,0,100), Rotation(0,0,0));

	glTranslatef(bodyPose.position.x, bodyPose.position.y,bodyPose.position.z);

	// rotate in zyx convention, as used in Kinematics::RotationMatrix
	glRotatef(degrees(bodyPose.orientation.z), 0.0,0.0,1.0);
	glRotatef(degrees(bodyPose.orientation.y), 0.0,1.0,0.0);
	glRotatef(degrees(bodyPose.orientation.x), 1.0,0.0,0.0);

	glPushMatrix();
	glTranslatef(0,0,0);
	body.display(glBotBodyColor,glBotBodyColor);
	glPopMatrix();
	glPopMatrix();
	glPopAttrib();
}



void BotDrawer::readSTLFiles(string path) {
	body.loadFile(path + "/body.stl");
}

void BotDrawer::setup() {
	static bool setupDone = false;
	if (!setupDone) {
		// search for stl files
		if (fileExists("./stl/hip.stl")) {
			readSTLFiles("./stl");
		} else {
			if (fileExists("./hip.stl"))
				readSTLFiles("./");
			if (fileExists("../../cad/simplified/hip.stl"))
				readSTLFiles("../../cad/simplified");
			else
				if (fileExists("../../../cad/simplified/hip.stl"))
					readSTLFiles("../../../cad/simplified");
		}
		setupDone = true;
	}
}
