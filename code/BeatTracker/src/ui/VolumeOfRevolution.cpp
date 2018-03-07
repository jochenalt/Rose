/*
 * VolumeOfRevolution.cpp
 *
 *  Created on: Mar 7, 2018
 *      Author: jochenalt
 */

#include <GL/gl.h>
#include <math.h>

#include <VolumeOfRevolution.h>


VolumeOfRevolution::VolumeOfRevolution()
{
    m_segments     = 30;
    m_angles       = 30;
    m_rotation     = 0.0f;
}


VolumeOfRevolution::~VolumeOfRevolution()
{
}




//polynomial interpretation for N points
float polyint ( float  points[][3], float x, int N )
{
  float y;

  float num = 1.0, den = 1.0;
  float sum = 0.0;

  for ( int i = 0; i < N; ++i ) {
    num = den = 1.0;
    for ( int j = 0; j < N; ++j ) {
      if ( j == i ) continue;

      num = num * ( x - points[j][0] );		 	//x - xj
    }
    for ( int j = 0; j < N; ++j ) {
      if ( j == i ) continue;
      den = den * ( points[i][0] - points[j][0] );	//xi - xj
    }
    sum += num / den * points[i][1];
  }
  y = sum;

  return y;
}

GLfloat ctrlpoints[4][3] = {
	{ 0.0, 0.0, 0.0}, { 1.0, 0.5, 0.0},
	{2.0, 1.2, 0.0}, {3.0, 3, 0.0}};



void VolumeOfRevolution::display(const Pose& bodyPose, const Pose& headPose, const GLfloat* color)
{
    unsigned int i, j;

	glPushAttrib(GL_CURRENT_BIT);
   	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
	glColor4fv(color);
	glPushMatrix();
    glMatrixMode(GL_MODELVIEW);


	   float x, y, z, r;				//current coordinates
	   float x1, y1, z1, r1;			//next coordinates
	   float theta;

	   const float startx = 0, endx = 3;
	   const int nx = 20;				//number of slices along x-direction
	   const int ntheta = 20;			//number of angular slices
	   const float dx = (endx - startx) / nx;	//x step size
	   const float dtheta = 2*M_PI / ntheta;		//angular step size

	   x = startx;
	   r = genRadius( bodyPose, headPose, x*100.);
	   glPushMatrix();

	   for ( i = 0; i < nx; ++i ) {			//step through x
	      theta = 0;
	      x1 = x + dx;				//next x
	      r1 = polyint( ctrlpoints, x1, 4);		//next f(x)
		   r1 = genRadius( bodyPose, headPose, x1);

	      //draw the surface composed of quadrilaterals by sweeping theta
	      glBegin( GL_LINE_STRIP );
	      // glBegin( GL_QUAD_STRIP );

	      for ( j = 0; j <= ntheta; ++j ) {
		  theta += dtheta;
		  double cosa = cos( theta );
		  double sina = sin ( theta );
		  y = r * cosa;  y1 = r1 * cosa;	//current and next y
		  z = r * sina;	 z1 = r1 * sina;	//current and next z

		  //edge from point at x to point at next x
		  glVertex3f (100.*z, 100.*x, 100.*y);
		  glVertex3f (100.*z1, 100.*x1, 100.*y1);

		  //forms quad with next pair of points with incremented theta value
		}
	      glEnd();
	      x = x1;
	      r = r1;
	   } //for i

	   glPopMatrix();


	    // restore old color
    glPopAttrib();
} // render()


float VolumeOfRevolution::genRadius(const Pose& bodyPose, const Pose& headPose, float z)
{
    return sin(z*M_PI);
}

Point VolumeOfRevolution::genCentre(const Pose& bodyPose, const Pose& headPose, float z) {
	if (z < bodyPose.position.z) {
		return Point(0,0,z);
	}
	else {
		return Point(0,0,z);
	}
}
