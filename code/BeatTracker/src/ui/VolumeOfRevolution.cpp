/*
 * VolumeOfRevolution.cpp
 *
 *  Created on: Mar 7, 2018
 *      Author: jochenalt
 */

#include <assert.h>
#include <GL/gl.h>
#include <math.h>

#include <VolumeOfRevolution.h>
#include "setup.h"

VolumeOfRevolution::VolumeOfRevolution()
{
    numSegments     = 24;
    numAngles       = 16;
    headRadius = 0;
    bodyRadius = 0;
    headLen = 0;
    bodyLen = 0;
    baseRadius = 0;
}


VolumeOfRevolution::~VolumeOfRevolution()
{
}


void VolumeOfRevolution::display(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, const GLfloat* color, const GLfloat* gridColor )
{
	glPushAttrib(GL_CURRENT_BIT);
	glPushMatrix();
    glMatrixMode(GL_MODELVIEW);


   Pose centre, centre1;
   HomogeneousMatrix rotationTrans;
   double r = getRadius( basePose, bodyPose, headPose, 0);
   centre = getCentrePose( basePose, bodyPose, headPose, 0);
   rotationTrans = createRotationMatrix(centre.orientation);
   glPushMatrix();

   bool oddLine = true;
   for ( float t = 0; t <= 1.0; t += 1.0/numSegments ) {
	   double r1 = getRadius( basePose, bodyPose, headPose, t);
	   centre1 = getCentrePose( basePose, bodyPose, headPose, t);
	   HomogeneousMatrix rotationTrans1 = createRotationMatrix(centre1.orientation);


	   // cout << "t=" << t << " centre1=" << centre1.orientation
		// 	<< "tmp=" << getPointByTransformationMatrix(c)  << endl;
	   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	   glColor4fv(color);

	   glBegin( GL_QUAD_STRIP);
	   for ( float angle = 0; angle <= M_PI*2.0 + 0.01; angle += 2.0*M_PI / numAngles) {
		   double sina = sin(angle);
		   double cosa = cos(angle);

		   Point circle (r * cosa + centre.position.x, r * sina + centre.position.y, 0);
		   HomogeneousMatrix circleTrans = rotationTrans*createTransformationMatrix(circle);
		   Point target = getPointByTransformationMatrix(circleTrans) + Point(0,0,centre.position.z);

		   Point circle1 (r1 * cosa + centre1.position.x, r1 * sina + centre1.position.y, 0);
		   HomogeneousMatrix circleTrans1 = rotationTrans1*createTransformationMatrix(circle1);
		   Point target1 = getPointByTransformationMatrix(circleTrans1) + Point(0,0,centre1.position.z);

		  // 1  3  4
		  // 2  4  5 ...
		  glVertex3f (target.y, target.z, target.x);
		  glVertex3f (target1.y, target1.z, target1.x);

	   }
	   glEnd();
	   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gridColor);
	   glColor4fv(gridColor);
	   glBegin( GL_LINE_STRIP  );
	   float startAngle = oddLine?0:M_PI / numAngles;
	   oddLine = !oddLine;
	   for ( float angle = startAngle; angle <= M_PI*2.0+startAngle + 0.01; angle += 2.0*M_PI / numAngles) {
		   double sina = sin(angle);
		   double cosa = cos(angle);

		   double sin1a = sin(angle + M_PI / numAngles);
		   double cos1a = cos(angle + M_PI / numAngles);

		   Point circle ((r+1) * cosa + centre.position.x, (r+1) * sina + centre.position.y, 0);
		   HomogeneousMatrix circleTrans = rotationTrans*createTransformationMatrix(circle);
		   Point target = getPointByTransformationMatrix(circleTrans) + Point(0,0,centre.position.z);

		   Point circle1 ((r1+1) * cos1a + centre1.position.x, (r1+1) * sin1a + centre1.position.y, 0);
		   HomogeneousMatrix circleTrans1 = rotationTrans1*createTransformationMatrix(circle1);
		   Point target1 = getPointByTransformationMatrix(circleTrans1) + Point(0,0,centre1.position.z);
		   // 1  3  4
		   // 2  4  5 ...
		   glVertex3f (target.y, target.z, target.x);
		   glVertex3f (target1.y, target1.z, target1.x);
	   }
	   glEnd();

	    r = r1;
		rotationTrans = rotationTrans1;
		centre = centre1;


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


	Point normalizedHead = headPose.position/headLen;
	Point normalizedBody= bodyPose.position/bodyLen;

	Pose supportBase = basePose + Pose(Point(0,0,bodyLen/3.0), Rotation(0,0,0));
	HomogeneousMatrix supportBaseTrans = createTransformationMatrix(bodyPose) * createTransformationMatrix(Point(0,0,-bodyLen/3.0));
	Pose supportBaseBody =  getPoseByTransformationMatrix(supportBaseTrans);

	double bodyHeadBoundary = bodyLen / ( bodyLen + headLen);
	if (t < bodyHeadBoundary) {
		// calculate bezier curve in the centre of the body

		double localT = t/bodyHeadBoundary;

		// compute bezier curve from base to body
		Pose result = bezierCurve(localT, basePose , supportBase, bodyPose, supportBaseBody);
		// result.orientation.z = t*radians(10);
		return result;
	}
	// calculate bezier curve in the centre of the body

	// get absolute pose of head
	HomogeneousMatrix headTransformation  = createTransformationMatrix(headPose) * createTransformationMatrix(bodyPose) ;
	Pose absHeadPose = getPoseByTransformationMatrix(headTransformation);

	double localT = (t-bodyHeadBoundary)/(1.0-bodyHeadBoundary);
	supportBase = Pose(Point(0,0,headLen/3.0), Rotation(0,0,0));
	HomogeneousMatrix supportBodyTrans = createTransformationMatrix(headPose) * createTransformationMatrix(Point(0,0,-headLen/3.0));
	Pose supportHead =  getPoseByTransformationMatrix(supportBodyTrans);

	Pose localHeadResult = bezierCurve(localT, Pose(), supportBase, headPose, supportHead);
	HomogeneousMatrix tmp = createTransformationMatrix(bodyPose) * createTransformationMatrix(localHeadResult);

	return getPoseByTransformationMatrix(tmp);

	// return bodyPose.position + bezierCurve(t, basePose.position, supportBodyBase, headPose.position, supportHeadBody);
}
