/*
 * BaseView.cpp
 *
 *  Created on: 26.07.2017
 *      Author: JochenAlt
 */


#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include <GL/Glui.h>

#include "basics/spatial.h"
#include "basics/util.h"

#include <BaseView.h>
#include "uiconfig.h"

#include <WindowController.h>


BaseView::BaseView() {
	currEyeDistance = ViewEyeDistance;
	baseAngle = -45;
	heightAngle = 0;
	lookAt.z = ViewBotHeight;
}

BaseView::~BaseView() {
}


void BaseView::setEyePosition(const Point& pEyePosition) {
	eyePosition = pEyePosition;
}

void BaseView::setLookAtPosition(const Point &pLookAtPosition) {
	lookAt = pLookAtPosition;
}


Point& BaseView::getLookAtPosition() {
	return lookAt;
}

void BaseView::postRedisplay() {
	int saveWindow = glutGetWindow();
	glutSetWindow(windowHandle);
	glutPostRedisplay();
	glutSetWindow(saveWindow);
}

void BaseView::setEyePosition( float pCurrEyeDistance, float angleAroundZ, float pHeightAngle) {

	baseAngle = angleAroundZ;
	heightAngle = constrain((realnum)pHeightAngle,radians(-90.0f),radians(45.0f));
	currEyeDistance = pCurrEyeDistance;

	eyePosition.x = currEyeDistance*(cosf(baseAngle) * cosf(heightAngle));
	eyePosition.y = currEyeDistance*( sinf(baseAngle) * cosf(heightAngle));
	eyePosition.z = ViewBotHeight - currEyeDistance*sinf(heightAngle);
}

float BaseView::getCurrentEyeDistance() {
	return currEyeDistance;
}

void BaseView::changeEyePosition(float pCurrEyeDistance, float angleAroundZ, float pHeightAngle) {

	currEyeDistance += pCurrEyeDistance;
	baseAngle 		+= radians(angleAroundZ);
	heightAngle 	+= radians(pHeightAngle);

	setEyePosition(currEyeDistance, baseAngle, heightAngle);
}

void BaseView::changeLookAtPosition(const Point& pLookAtPosition) {

	lookAt += pLookAtPosition;
}

void BaseView::setLights()
{
	const float lightDistance = 15000.0f;
	GLfloat light_ambient[] =  {0.1, 0.1, 0.1, 1.0};
	GLfloat light_diffuse[] =  {0.8, 0.8, 0.8, 1.0};
	GLfloat light_specular[] = {0.1, 0.1, 0.1, 1.0};

	GLfloat light_position0[] = {1*lightDistance, 1*lightDistance, 1*lightDistance, 0.0};	// ceiling left
	GLfloat light_position1[] = {-2*lightDistance, 2*lightDistance, 3*lightDistance, 0.0};	// ceiling right
	GLfloat light_position2[] = {0, 2*lightDistance, -3*lightDistance, 0.0};					// far away from the back
	GLfloat mat_ambient[] =  {0.1, 0.1, 0.1, 1.0};
	GLfloat mat_diffuse[] =  {0.4, 0.8, 0.4, 1.0};
	GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};

	glMaterialfv(GL_LIGHT0, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_LIGHT0, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_LIGHT0, GL_SPECULAR, mat_specular);

	glMaterialfv(GL_LIGHT1, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_LIGHT1, GL_DIFFUSE, mat_diffuse);

	glMaterialfv(GL_LIGHT2, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_LIGHT2, GL_DIFFUSE, mat_diffuse);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);

	glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
	glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

void BaseView::setEyePosition() {
	// Enable perspective projection with fovy, aspect, zNear and zFar
	gluPerspective(45.0f, (GLfloat)glutGet(GLUT_WINDOW_WIDTH) / (GLfloat)glutGet(GLUT_WINDOW_HEIGHT), 1.1f, currEyeDistance*3);
	gluLookAt(eyePosition.y+lookAt.y,eyePosition.z+lookAt.z,eyePosition.x+lookAt.x,
			lookAt.y,  lookAt.z,lookAt.x,
			0.0, 1.0, 0.0);
}

