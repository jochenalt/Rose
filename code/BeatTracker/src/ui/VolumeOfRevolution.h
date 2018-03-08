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


   void display(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, const GLfloat* color);
   void set(double newBaseRadius, double newBodyRadius, double newHeadRadius) {
	   baseRadius = newBaseRadius;
	   bodyRadius = newBodyRadius;
	   headRadius = newHeadRadius;
   }


 protected:

   unsigned int m_segments;
   unsigned int m_angles;

   double baseRadius;
   double bodyRadius;
   double headRadius;
   float genRadius(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, float z);
   Point genCentre(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, float z);

};

#endif /* SRC_UI_VOLUMEOFREVOLUTION_H_ */
