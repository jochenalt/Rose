

#ifndef UI_UTILS_H_
#define UI_UTILS_H_


#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include <math.h>

void drawFilledCircle(GLfloat x, GLfloat y,GLfloat z, GLfloat radius){
	int i;
	int triangleAmount = 20; //# of triangles used to draw circle

	GLfloat anglePerTriangle = M_PI*2.0/triangleAmount;

	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(x, y); // center of circle
		for(i = 0; i <= triangleAmount;i++) {
			glVertex3f(
		        x + (radius * cos(i * anglePerTriangle)),
		       z,
			    y + (radius * sin(i * anglePerTriangle))
			);
		}
	glEnd();
}


#endif
