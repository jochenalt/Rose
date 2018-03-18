/*
 * VolumeOfRevolution.cpp
 *
 *  Created on: Mar 7, 2018
 *      Author: jochenalt
 */

#include <assert.h>
#include <GL/gl.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

#include <ui/VolumeOfRevolution.h>
#include "setup.h"

VolumeOfRevolution::VolumeOfRevolution()
{
    numSegments     = 12;
    numAngles       = 32;
    headRadius = 0;
    bodyRadius = 0;
    headLen = 0;
    bodyLen = 0;
    baseRadius = 0;
}


VolumeOfRevolution::~VolumeOfRevolution()
{
}


void VolumeOfRevolution::display(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, const GLfloat* bodyColor1,const GLfloat* bodyColor2, const GLfloat* gridColor )
{
	glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();
    glMatrixMode(GL_MODELVIEW);


   Pose centre, centre1;
   HomogeneousMatrix centreTrans1;
   HomogeneousMatrix centreTrans;
   double r = getRadius( basePose, bodyPose, headPose, 0);
   centre = getCentrePose( basePose, bodyPose, headPose, 0);
   centreTrans = createTransformationMatrix(centre1);
   glPushMatrix();

   bool oddLine = false;
   for ( float t = 0; t <= 1.0 + 0.01; t += 1.0/numSegments ) {
	   oddLine = !oddLine;
	   double r1 = getRadius( basePose, bodyPose, headPose, t);
	   centre1 = getCentrePose( basePose, bodyPose, headPose, t);
	   centreTrans1 = createTransformationMatrix(centre1);

	   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bodyColor1);
	   glColor4fv(bodyColor1);

	   glBegin( GL_QUAD_STRIP);

	   bool oddRow = false;
	   for ( float angle = 0; angle <= M_PI*2.0 + 0.01; angle += 2.0*M_PI / numAngles) {
		   oddRow = !oddRow;
		   double sina = sin(angle);
		   double cosa = cos(angle);

		   Point relCirclePoint (r * cosa, r * sina, 0);
		   HomogeneousMatrix targetTrans = centreTrans*createTransformationMatrix(relCirclePoint);
		   Point target = getPointByTransformationMatrix(targetTrans);

		   Point relCirclePoint1 (r1 * cosa, r1 * sina, 0);
		   HomogeneousMatrix targetTrans1 = centreTrans1*createTransformationMatrix(relCirclePoint1);
		   Point target1 = getPointByTransformationMatrix(targetTrans1);

		   if (oddRow) {
			   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bodyColor2);
			   glColor4fv(bodyColor2);
		   } else {
			   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, bodyColor1);
			   glColor4fv(bodyColor1);
		   }
		   // 1  3  4
		   // 2  4  5 ...
		   glVertex3f (target.y, target.z, target.x);
		   glVertex3f (target1.y, target1.z, target1.x);
	   }
	   glEnd();

	   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gridColor);
	   glColor4fv(gridColor);
	   glBegin( GL_LINE_STRIP  );
	   oddRow = false;
	   bool useDiamonds = false;
	   float startAngle = (oddLine && useDiamonds)?M_PI / numAngles:0;
	   for ( float angle = startAngle; angle <= M_PI*2.0+startAngle + 0.01; angle += 2.0*M_PI / numAngles) {
		   oddRow = !oddRow;

		   double sina = sin(angle);
  		   double cosa = cos(angle);

		   Point relCirclePoint ((r+1.0) * cosa, (r+1.0) * sina, 0);
		   HomogeneousMatrix targetTrans = centreTrans*createTransformationMatrix(relCirclePoint);
		   Point target = getPointByTransformationMatrix(targetTrans);

		   if (useDiamonds) {
			   //   2
			   // 1    .

	  		   double sin1a = sin(angle + M_PI / numAngles);
	  		   double cos1a = cos(angle + M_PI / numAngles);

			   Point relCirclePoint1 ((r1+1.0) * cos1a, (r1+1.0) * sin1a, 0);
			   HomogeneousMatrix targetTrans1 = centreTrans1*createTransformationMatrix(relCirclePoint1);
			   Point target1 = getPointByTransformationMatrix(targetTrans1);

			   glVertex3f (target.y, target.z, target.x);
			   glVertex3f (target1.y, target1.z, target1.x);
		   } else {

			   Point relCirclePoint1 ((r1+1.0) * cosa, (r1+1.0) * sina, 0);
			   HomogeneousMatrix targetTrans1 = centreTrans1*createTransformationMatrix(relCirclePoint1);
			   Point target1 = getPointByTransformationMatrix(targetTrans1);

			   // 2  3  6
			   // 1  4  5
			   if (oddRow) {
				   glVertex3f (target.y, target.z, target.x);
				   glVertex3f (target1.y, target1.z, target1.x);
			   }
			   else {
				   glVertex3f (target1.y, target1.z, target1.x);
				   glVertex3f (target.y, target.z, target.x);
			   }
			   // glVertex3f (target2.y, target2.z, target2.x);
		   }
	   }
	   glEnd();

	    r = r1;
		centre = centre1;
		centreTrans = centreTrans1;


	} //for i

	glPopMatrix();
    glPopAttrib();
}


double bezierCurve (double t, double a, double aSupport, double b, double bSupport) {
	// formula of cubic bezier curve (wikipedia)
	if (t> 1.0)
		t = 1.0;
	return a*(1-t)*(1-t)*(1-t) + aSupport*3*t*(1-t)*(1-t) + bSupport*3*t*t*(1-t) + b*t*t*t;
}

Point bezierCurve (double t, const Point& a, const Point& aSupport, const Point& b, const Point& bSupport) {
	// formula of cubic bezier curve (wikipedia)
	if (t> 1.0)
		t = 1.0;
	return a*(1-t)*(1-t)*(1-t) + aSupport*3*t*(1-t)*(1-t) + bSupport*3*t*t*(1-t) + b*t*t*t;
}

Pose bezierCurve (double t, const Pose& a, const Pose& aSupport, const Pose& b, const Pose& bSupport) {
	// formula of cubic bezier curve (wikipedia)
	if (t> 1.0)
		t = 1.0;
	return a*(1-t)*(1-t)*(1-t) + aSupport*3*t*(1-t)*(1-t) + bSupport*3*t*t*(1-t) + b*t*t*t;
}



float VolumeOfRevolution::getRadius(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, float t)
{
	return bezierCurve(t,baseRadius, baseRadius, headRadius, baseRadius);
}


Pose VolumeOfRevolution::getCentrePose(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, float t) {

	// t=0..1 of the full guy, check if the body or the head piece is requested
	double bodyHeadBoundary = bodyLen / ( bodyLen + headLen);
	if (t < bodyHeadBoundary) {
		// consider the body piece

		// compute support point for bezier curve of the base which is orthogonal to the ground and a distance to the origin of length/3
		Pose supportBase = basePose + Pose(Point(0,0,bodyLen/3.0), Rotation(0,0,0));

		// compute support point of the body which is orthogonal to the body platform and goes down with a distance of length/3
		HomogeneousMatrix supportBaseTrans = createTransformationMatrix(bodyPose)  * createTransformationMatrix(Point(0,0,-bodyLen/3.0))   ;
		Pose supportBaseBody =  getPoseByTransformationMatrix(supportBaseTrans);

		// compute t=0..1 within the body witout the head piece
		double localT = t/bodyHeadBoundary;

		// compute bezier curve from base to body
		Pose result = bezierCurve(localT, basePose , supportBase, bodyPose, supportBaseBody);
		return result;
	}
	// now consider the head piece

	// define t=0..1 just between body platform and head platform
	double localT = (t-bodyHeadBoundary)/(1.0-bodyHeadBoundary);

	// compute support point of the body for bezier curve that is orthogonal to the body.
	// But we compute the head bezier curve locally first, so it is orthogonal to the ground
	Pose supportBase = Pose(Point(0,0,headLen/3.0), Rotation(0,0,0));

	// compute support point of the head platform that is orthogonal to the head and has a distance of length/3 from the head platform
	HomogeneousMatrix supportBodyTrans =  createTransformationMatrix(headPose) * createTransformationMatrix(Point(0,0,-headLen/3.0)) ;
	Pose supportHead =  getPoseByTransformationMatrix(supportBodyTrans);

	// final bezier curve with these two support points
	Pose localHeadResult = bezierCurve(localT, Pose(), supportBase, headPose, supportHead);

	// transform the local bezier curve to sit on top of the body platform
	HomogeneousMatrix bodyTransformation = createTransformationMatrix(bodyPose) * createTransformationMatrix(localHeadResult);

	Pose result = getPoseByTransformationMatrix(bodyTransformation);
	return result;
}
