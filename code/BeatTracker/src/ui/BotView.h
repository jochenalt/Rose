/*
 * BotViewController.h
 *
 *  Created on: 07.08.2016
 *      Author: JochenAlt
 */

#ifndef UI_BOTVIEW_H_
#define UI_BOTVIEW_H_

#include <string>
#include "BotDrawer.h"
#include "BaseView.h"

using namespace std;

class BotView : public BaseView {
public:
	BotView();
	int create(int mainWindow, string pTitle);

	void display();
	void reshape(int x,int y, int w, int h);

	void hide();
	void show();

	void MotionCallback(int x, int y);
	void MouseCallback(int button, int button_state, int x, int y);

	void setBodyPose(const Pose& bodyPose, const Pose& headPose, const Point& eyeDeviation);

	void resetDisplayFlag() { justDisplayed = false; };
	bool isJustDisplayed() { return justDisplayed; };

	BotDrawer& getBotRenderer() { return botDrawer; }

private:
	void drawCoordSystem(bool withRaster );

	Pose bodyPose;
	Pose headPose;
	Point eyeDeviation;

	string title;

	int lastMouseX = 0;
	int lastMouseY = 0;
	int lastMouseScroll = 0;
	enum mousePaneType { VIEW_PANE, NO_PANE };
	mousePaneType mousePane = NO_PANE;

	BotDrawer botDrawer;
	bool justDisplayed = false;
};

#endif /* UI_BOTVIEW_H_ */
