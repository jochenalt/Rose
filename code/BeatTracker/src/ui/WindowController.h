/*
 * WindowController.h
 *
 * Represents control of bot and interactive view
 *
 * Author: JochenAlt
 */

#ifndef UI_BOTWINDOWCTRL_H_
#define UI_BOTWINDOWCTRL_H_

#include <thread>

#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glut.h>  // GLUT, includes glu.h and gl.h
#include <GL/glui.h>

#include "basics/spatial.h"
#include "BotView.h"

class WindowController {
public:
	WindowController() {
		eventLoopThread = NULL;
		uiReady = false;
	};

	static WindowController& getInstance() {
			static WindowController instance;
			return instance;
	}

	bool readyForControllerEvent();
	bool isReady() { return uiReady; };
	bool setup(int argc, char** argv);

	void setBodyPose(const Pose& bodyPose);

	void postRedisplay();

	BotView mainBotView;
private:
	 void UIeventLoop();
	 GLUI* createInteractiveWindow(int mainWindow);

	 std::thread* eventLoopThread;
	 bool uiReady;
};


#endif /* UI_BOTWINDOWCTRL_H_ */
