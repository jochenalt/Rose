/*
 * BotViewController.cpp
 *
 * Author: JochenAlt
 */


#include <ui/BotRenderer.h>
#include <thread>

#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include "basics/spatial.h"
#include "basics/util.h"

#include <ui/BotView.h>
#include <dance/Dancer.h>
#include <Stewart/StewartKinematics.h>
#include <Stewart/BodyKinematics.h>

#include <ui/WindowController.h>
#include <ui/uiconfig.h>

using namespace std;

BotView::BotView() {
	windowHandle = 0; // set by derived view class
}

void BotView::drawCoordSystem(bool withRaster) {

	// draw coordinate system
	const float axisLength = 300.0f;
	const float arrowLength = 20.0f;
	const float unitLength = 30.0f;
	const float rasterLineLength = axisLength*2;
	if (withRaster) {
		// current offset of the rasterfield, moves from 0 to unitlength, and is set back by a jump
		Point currPos;
		static Point posOffset;

		if (currPos.x - posOffset.x > unitLength) {
			posOffset.x += unitLength;
		}
		if (currPos.x - posOffset.x < -unitLength) {
			posOffset.x -= unitLength;
		}
		if (currPos.y - posOffset.y > unitLength) {
			posOffset.y += unitLength;
		}
		if (currPos.y - posOffset.y < -unitLength) {
			posOffset.y -= unitLength;
		}

		Point localRasterMovement = currPos - posOffset;
		glPushMatrix();
		glLoadIdentity();

		glTranslatef(0, 0, -localRasterMovement.x);
		glTranslatef(-localRasterMovement.y, 0, 0);

		glPushAttrib(GL_LIGHTING_BIT);

		for (float rasterX = -rasterLineLength;rasterX<=rasterLineLength;rasterX = rasterX + unitLength ) {
			for (float rasterY = -rasterLineLength;rasterY<=rasterLineLength;rasterY = rasterY + unitLength ) {
				Point raster(rasterX,rasterY,0);
                Point mapP1(rasterX+posOffset.x,				rasterY+posOffset.y);
                Point mapP2(rasterX+posOffset.x+unitLength,		rasterY+posOffset.y);
                Point mapP3(rasterX+posOffset.x,				rasterY+posOffset.y+unitLength);
                Point mapP4(rasterX+posOffset.x + unitLength,	rasterY+posOffset.y+unitLength);

                Point p1(rasterX, 				rasterY, 			mapP1.z);
                Point p2(rasterX+unitLength, 	rasterY,			mapP2.z);
                Point p3(rasterX, 				rasterY+unitLength,	mapP3.z);
                Point p4(rasterX+unitLength, 	rasterY+unitLength,	mapP4.z);

				bool highlightX = true;
				bool highlightY = true;

				glBegin(GL_LINES);
					if (highlightX) {
						glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glRasterColor4v);
						glColor3fv(glRasterColor4v);
					} else {
						glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glMapAreaColor4v);
						glColor3fv(glMapAreaColor4v);
					}
					if (rasterX > -rasterLineLength) {
						glVertex3f(p1.y, p1.z, p1.x);
						glVertex3f(p3.y, p3.z, p3.x);
					}

					if (highlightY) {
						glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glRasterColor4v);
						glColor3fv(glRasterColor4v);
					} else {
						glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glMapAreaColor4v);
						glColor3fv(glMapAreaColor4v);
					}
					if (rasterY  > -rasterLineLength) {
						glVertex3f(p1.y, p1.z, p1.x);
						glVertex3f(p2.y, p2.z, p2.x);
					}

				glEnd();
				glBegin(GL_TRIANGLES);
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glMapAreaColor4v);
					glColor4fv(glMapAreaColor4v);
		            glNormal3f(0.0,1.0,0.0);
					glVertex3f(p1.y, p1.z-3, p1.x);	glVertex3f(p2.y, p2.z-3, p2.x);
					glVertex3f(p3.y, p3.z-3, p3.x);	glVertex3f(p2.y, p2.z-3, p2.x);
					glVertex3f(p4.y, p4.z-3, p4.x); glVertex3f(p3.y, p3.z-3, p3.x);
				glEnd();
			}
		}

		glPopAttrib();
		glPopMatrix();
	}

	glBegin(GL_LINES);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, glCoordSystemAxisColor4v);
		glColor4fv(glCoordSystemAxisColor4v);


		float z = 2.0f;
		// robot's x-axis
		glVertex3f(0.0f, z, -arrowLength);glVertex3f(0.0f, z, axisLength);
		glVertex3f(0.0f, z, axisLength);glVertex3f(0.0f,+arrowLength/2, axisLength-arrowLength);
		glVertex3f(0.0f, z, axisLength);glVertex3f(0.0f,-arrowLength/2, axisLength-arrowLength);
		for (float i = 0;i<axisLength;i = i + unitLength ) {
			glVertex3f(0.0f, -arrowLength/2, i);glVertex3f(0.0f,+arrowLength/2, i);
		}

		// robot's y-axis
		glVertex3f(-arrowLength, z, 0.0f);glVertex3f(axisLength, z, 0.0f);
		glVertex3f(axisLength, z, 0.0f);glVertex3f(axisLength-arrowLength, -arrowLength/2, 0.0f);
		glVertex3f(axisLength, z, 0.0f);glVertex3f(axisLength-arrowLength, arrowLength/2, 0.0f);
		for (float i = 0;i<axisLength;i = i + unitLength ) {
			glVertex3f(i, -arrowLength/2, 0.0f);glVertex3f(i,+arrowLength/2, 0.0f);
		}

#ifdef DISPLAY_Z_AXIS
		// robot's z-axis
		glVertex3f(0.0f, -arrowLength, 0.0f);glVertex3f(0.0f, axisLength,0.0f);
		glVertex3f(0.0f, axisLength,0.0f);glVertex3f(+arrowLength/2,axisLength-arrowLength, 0.0f);
		glVertex3f(0.0f, axisLength,0.0f);glVertex3f(-arrowLength/2, axisLength-arrowLength,0.0f);
		for (float i = 0;i<axisLength;i = i + unitLength ) {
			glVertex3f(-arrowLength/2, i,0.0f);glVertex3f(+arrowLength/2, i,0.0f);
		}
#endif
	glEnd();

	glRasterPos3f(axisLength+arrowLength, 0.0f, 0.0f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12,(const unsigned char*) "y");
	glRasterPos3f(0.0f, 0.0f, axisLength+arrowLength);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12,(const unsigned char*) "x");
#ifdef DISPLAY_Z_AXIS
	glRasterPos3f(0.0f, axisLength+arrowLength,0.0f);
	glutBitmapString(GLUT_BITMAP_HELVETICA_12,(const unsigned char*) "z");
#endif
}

void displayBotView() {
	WindowController::getInstance().mainBotView.display();
}

void BotViewMotionCallback(int x, int y) {
	WindowController::getInstance().mainBotView.MotionCallback(x,y);
	WindowController::getInstance().mainBotView.modify();
}
void BotViewMouseCallback(int button, int button_state, int x, int y ) {
	WindowController::getInstance().mainBotView.MouseCallback(button, button_state,  x,  y);
	WindowController::getInstance().mainBotView.modify();
}

int BotView::create(int mainWindow, string pTitle) {
	// initially start with zero size, will be resized in reshape
	title = pTitle;
	windowHandle = glutCreateSubWindow(mainWindow, 1,1,1,1);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);   							// Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL);    							// Set the type of depth-test
	glShadeModel(GL_SMOOTH);   							// Enable smooth shading
	glEnable(GL_BLEND); 								// enable transparent views
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  );// way of transparency
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 	// Nice perspective corrections

	setLights();
	setEyePosition(ViewEyeDistance, radians(-15), radians(-10));

	// Main Bot view has comprehensive mouse motion
	glutSetWindow(windowHandle);
	glutMotionFunc( BotViewMotionCallback);
	glutMouseFunc( BotViewMouseCallback);
	glutDisplayFunc(displayBotView);

	botDrawer.setup();

	setBodyPose(Dancer ::getInstance().getDefaultBodyPose(), Dancer ::getInstance().getDefaultHeadPose(), eyeDeviation);
	return windowHandle;
}


void BotView::display() {

	int savedWindowHandle = glutGetWindow();
	glutSetWindow(windowHandle);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
	glLoadIdentity();             // Reset the model-view matrix
	setEyePosition();
	glMatrixMode(GL_MODELVIEW);

	botDrawer.displayBot(bodyPose, headPose);
	drawCoordSystem(true);

	glutSetWindow(savedWindowHandle);
}


void BotView::setBodyPose(const Pose& newBodyPose, const Pose& newHeadPose, const Point& newEyeDeviation) {
	if ((newBodyPose != bodyPose) || (headPose != newHeadPose) || (newEyeDeviation.distance(eyeDeviation) > 1.0)) {
		bodyPose = newBodyPose;
		headPose = newHeadPose;
		eyeDeviation = newEyeDeviation;
		modify();
	}
}

void BotView::reshape(int x,int y, int w, int h) {
	glutSetWindow(windowHandle);
	glutShowWindow();
	glutPositionWindow(x, y);
	glutReshapeWindow(w, h);
	glViewport(0, 0, w, h);

	display();
}

void BotView::hide() {
	glutSetWindow(windowHandle);
	glutHideWindow();
}

void BotView::show() {
	glutSetWindow(windowHandle);
	glutShowWindow();
}

void BotView::MotionCallback(int x, int y) {
	// if display has not yet been called after the last motion, dont execute
	// this one, but wait for

	float diffX = (float) (x-lastMouseX);
	float diffY = (float) (y-lastMouseY);

	switch (mousePane) {
	case VIEW_PANE:
		WindowController::getInstance().mainBotView.changeEyePosition(0, -diffX, -diffY);
		postRedisplay();
		isModifiedFlag = true;
		break;
	default:
		if (lastMouseScroll != 0) {
			WindowController::getInstance().mainBotView.changeEyePosition(-getCurrentEyeDistance()*2*lastMouseScroll/100, 0,0);
			postRedisplay();
			lastMouseScroll = 0;
			isModifiedFlag = true;
		}
	}

	if (mousePane != NO_PANE) {
		lastMouseX = x;
		lastMouseY = y;
	}
}


void BotView::MouseCallback(int button, int button_state, int x, int y )
{
	mousePane = NO_PANE;

	bool withShift = glutGetModifiers() & GLUT_ACTIVE_SHIFT;
	bool withCtrl= glutGetModifiers() & GLUT_ACTIVE_CTRL;
	bool withAlt = glutGetModifiers() & GLUT_ACTIVE_ALT;

	if ( button == GLUT_LEFT_BUTTON && button_state == GLUT_DOWN && !withShift && !withCtrl && !withAlt) {
		mousePane = VIEW_PANE;
	} else {

		// Wheel reports as button 3(scroll up) and button 4(scroll down)
		if ((button == 3) || (button == 4)) // It's a wheel event
		{
			// Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
			if (button_state != GLUT_UP) { // Disregard redundant GLUT_UP events
				if (button == 3)
					lastMouseScroll++;
				else
					lastMouseScroll--;
				BotViewMotionCallback(x,y); // scroll wheel does not trigger a glut call of MotionCallback, so we do it manually
			}
		}
	}

	if (mousePane != NO_PANE) {
	    lastMouseX = x;
	    lastMouseY = y;
	}
}

