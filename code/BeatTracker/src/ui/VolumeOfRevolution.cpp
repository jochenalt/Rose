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
    m_segments     = 30;
    m_angles       = 30;
}


VolumeOfRevolution::~VolumeOfRevolution()
{
}



void VolumeOfRevolution::display(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, const GLfloat* color)
{
    unsigned int i, j;

	glPushAttrib(GL_CURRENT_BIT);
   	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	glColor4fv(color);
	glPushMatrix();
    glMatrixMode(GL_MODELVIEW);


   float x1 = 0,y1 = 0,z1 = 0,r1 = 0;
   float x = 0,y = 0,z = 0,r = 0;
   Point centre, centre1;

   float theta;

   float startx = basePose.position.z;
   float endx = bodyPose.position.z + headPose.position.z;
   const int nx = 20;				//number of slices along x-direction
   const int ntheta = 20;			//number of angular slices
   const float dx = (endx - startx) / nx;	//x step size
   const float dtheta = 2*M_PI / ntheta;		//angular step size

   x = startx;
   r = genRadius( basePose, bodyPose, headPose, x);
   centre = genCentre( basePose, bodyPose, headPose, x);

   glPushMatrix();

   for ( i = 0; i < nx; ++i ) {			//step through x
	   theta = 0;

	   // compute next z position and radius
	   x1 = x + dx;
	   r1 = genRadius( basePose, bodyPose, headPose, x1);
	   centre1 = genCentre( basePose, bodyPose, headPose, x1);

	   glBegin( GL_LINE_STRIP );
	   // glBegin( GL_QUAD_STRIP );

	   for ( j = 0; j <= ntheta; ++j ) {
		   	  theta += dtheta;
		   	  double cosa = cos( theta );
		   	  double sina = sin ( theta );
			  y = r * cosa + centre.x;
			  z = r * sina + centre.y;
		   	  y1 = r1 * cosa + centre1.x;	//current and next y
		   	  z1 = r1 * sina + centre1.y;	//current and next z

		  glVertex3f (z, x, y);
		  glVertex3f (z1, x1, y1);

	   }
	    glEnd();

	    // define next point as current point
	    r = r1;
		centre = centre1;
		x = x1;

	} //for i

	glPopMatrix();
    glPopAttrib();
}


float VolumeOfRevolution::genRadius(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, float z)
{
	if (z < bodyPose.position.z) {
		double t = (z-basePose.position.z)/(bodyPose.position.z - basePose.position.z) ;
 		return (1.0-t)*baseRadius + t*bodyRadius;
	}

	double t = (z-bodyPose.position.z)/headPose.position.z;
	return (1.0-t)*bodyRadius + t*headRadius;
}


Point bezierCurve (double t, const Point& a, const Point& aSupport, const Point& b, const Point& bSupport) {
	// formula of cubic bezier curve (wikipedia)
	assert ((t >= 0) && (t <= 1.0));
	return a*(1-t)*(1-t)*(1-t) + aSupport*3*t*(1-t)*(1-t) + bSupport*3*t*t*(1-t) + b*t*t*t;
}

Point VolumeOfRevolution::genCentre(const Pose& basePose, const Pose& bodyPose, const Pose& headPose, float z) {

	if (z < bodyPose.position.z) {
		double t = (z-basePose.position.z)/(bodyPose.position.z - basePose.position.z) ;
		// return basePose.position + bodyPose.position*t;
		Point supportA = basePose.position + Point(0,0,(bodyPose.position.z + basePose.position.z)/2.0);
		Point supportB = headPose.position/headPose.position.length() *(-1.0) * (bodyPose.position.length())/2.0;

		return bezierCurve(t, basePose.position, supportA, bodyPose.position, supportB);
	}

	double t = (z-bodyPose.position.z)/(headPose.position.z) ;
	return bodyPose.position + headPose.position*t;
	// return bodyPose.position + bezierCurve(t, basePose.position, bodyPose.position, bodyPose.position, basePose.position);
}
