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


   void display(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, const GLfloat* bodyColor1, const GLfloat* bodyColor2,const GLfloat* gridColor);
   void set(double newBaseRadius, double newBodyRadius, double newHeadRadius, double newBodyLen, double newHeadLen) {
	   baseRadius = newBaseRadius;
	   bodyRadius = newBodyRadius;
	   headRadius = newHeadRadius;
	   bodyLen = newBodyLen;
	   headLen = newHeadLen;

   }

   void setDiamonds(bool ok) { useDiamonds = ok; };
 protected:

   bool useDiamonds;
   unsigned int numSegments;
   unsigned int numAngles;

   double baseRadius;
   double bodyRadius;
   double headRadius;
   double bodyLen;
   double headLen;
   float getRadius(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, float z);
   Pose getCentrePose(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, float z);

};

#endif /* SRC_UI_VOLUMEOFREVOLUTION_H_ */
