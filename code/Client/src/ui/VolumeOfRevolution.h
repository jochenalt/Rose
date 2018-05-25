/*
 * VolumeOfRevolution.h
 *
 *  Created on: Mar 7, 2018
 *      Author: jochenalt
 */

#ifndef SRC_UI_VOLUMEOFREVOLUTION_H_
#define SRC_UI_VOLUMEOFREVOLUTION_H_

#include "basics/spatial.h"
class VolumeOfRevolution {
public:
	VolumeOfRevolution();
	virtual ~VolumeOfRevolution();


   void display(const Pose& basePose, const Pose& headPose, const GLfloat* bodyColor1, const GLfloat* bodyColor2,const GLfloat* gridColor);
   void set(double newBaseRadius, double newHeadRadiusX, double newHeadRadiusY, double newLen) {
	   baseRadius = newBaseRadius;
	   headRadiusX = newHeadRadiusX;
	   headRadiusY = newHeadRadiusY;

	   len = newLen;

   }
 protected:
   unsigned int numSegments;
   unsigned int numAngles;

   double baseRadius;
   double bodyRadius;
   double headRadiusX;
   double headRadiusY;

   double len;
   float getRadiusX(const Pose& basePose,const Pose& headPose, float z);
   float getRadiusY(const Pose& basePose,const Pose& headPose, float z);

   Pose getCentrePose(const Pose& basePose, const Pose& headPose, float z);

};

#endif /* SRC_UI_VOLUMEOFREVOLUTION_H_ */
