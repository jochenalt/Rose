/*
 * BaseView.h
 *
 *  Created on: 26.07.2017
 *      Author: JochenAlt
 */

#ifndef BASEVIEW_H_
#define BASEVIEW_H_

#include "basics/Point.h"

class BaseView {
public:
	BaseView();
	virtual ~BaseView();
	void setEyePosition(const Point &eyePosition);
	void setEyePosition(float currEyeDistance, float baseAngle, float heightAngle);
	void changeEyePosition(float currEyeDistance, float baseAngle, float heightAngle);
	void changeLookAtPosition(const Point &lookAtDiff);
	Point& getLookAtPosition();
	void setLookAtPosition(const Point &lookAt);

	Point& getEyePosition() { return eyePosition; };
	float getEyeDistance() { return currEyeDistance; };

	void setLights();
	void setEyePosition();
	float getCurrentEyeDistance() ;
	float getXYPaneAngle() { return baseAngle; };

	void postRedisplay();

private:
	Point eyePosition;
	float currEyeDistance = 0;
	float baseAngle = 0;
	float heightAngle = 0;
	Point lookAt;
protected:
	int windowHandle;
};

#endif /* BASEVIEW_H_ */
