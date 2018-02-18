/*
 * space.cpp
 *
 * Created: 25.11.2014 16:37:16
 *  Author: JochenAlt
 */ 
#include "space.h"
#include "FastMath.h"
#include "FixedPoint.h"


Point::Point() {
	x_fp4 = 0;
	y_fp4 = 0;
	z_fp4 = 0;
}

Point::Point(const Point& p) {
	x_fp4 = p.x_fp4;
	y_fp4 = p.y_fp4;
	z_fp4 = p.z_fp4;
}


void Point::null() { set(0,0,0);};

void Point::operator= (Point p) {
	x_fp4 = p.x_fp4;
	y_fp4 = p.y_fp4;
	z_fp4 = p.z_fp4;
}

void Point::print() {
	Serial.print("(");
	Serial.print(getf(X_INDEX),1);
	Serial.print(",");
	Serial.print(getf(Y_INDEX),1);
	Serial.print(",");
	Serial.print(getf(Z_INDEX),1);
	Serial.print(") ");
}

void Point::println() {
	print();
	Serial.println();
}

void Point::print(String s) {
	Serial.print(s);
	Serial.print("=");
	print();
}

void Point::println(String s) {
	print(s);
	Serial.println();
}

void Point::translate(Point pPoint) {
	x_fp4 += pPoint.x_fp4;
	y_fp4 += pPoint.y_fp4;
	z_fp4 += pPoint.z_fp4;
}

void Point::setf(float pX, float pY,float pZ) {
	setf(X_INDEX, pX);
	setf(Y_INDEX, pY);
	setf(Z_INDEX, pZ);
}

void Tensor::setPoint (const Point& pPoint) {
		trans = pPoint;
}

void Tensor::setRotation (const Rotation& pRot) {
		rot = pRot;
}

void Matrix3x3::setRotationMatrixX(int16_fp4_t degree) {
	// set rotation matrix to rotate by x-axis
	float s = sin_FP6(degree<<2);
	float c = cos_FP6(degree<<2);
	// mat[3][3] = { {1,0,0}, {0,c,s}, {0,-s,c} };
	mat[0][0] = FP(1,14);
	mat[0][1] = 0;
	mat[0][2] = 0;

	mat[1][0] = 0;
	mat[1][1] = c;
	mat[1][2] = s;

	mat[2][0] = 0;
	mat[2][1] = -s;
	mat[2][2] = c;
}

void Matrix3x3::setRotationMatrixY(int16_fp4_t degree) {
	float s = sin_FP6(degree<<2);
	float c = cos_FP6(degree<<2);
	// mat[3][3] = { {1,0,0}, {0,c,s}, {0,-s,c} };

	mat[0][0] = c;
	mat[0][1] = 0;
	mat[0][2] = -s;

	mat[1][0] = 0;
	mat[1][1] = FP(1,14);
	mat[1][2] = 0;

	mat[2][0] = s;
	mat[2][1] = 0;
	mat[2][2] = c;
}
void Matrix3x3::setRotationMatrixZ(int16_fp4_t degree) {
	float s = sin_FP6(degree<<2);
	float c = cos_FP6(degree<<2);

	mat[0][0] = c;
	mat[0][1] = -s;
	mat[0][2] = 0;

	mat[1][0] = s;
	mat[1][1] = c;
	mat[1][2] = 0;

	mat[2][0] = 0;
	mat[2][1] = 0;
	mat[2][2] = FP(1,14);
}

void Matrix3x3::multiply(Matrix3x3 a,Matrix3x3 b) {
	null();
	for(uint8_t i = 0; i<3; i++) {
		int16_fp14_t rYi0 = b.mat[i][0];
		int16_fp14_t rYi1 = b.mat[i][1];
		int16_fp14_t rYi2 = b.mat[i][2];

		for(uint8_t j = 0; j<3; j++) {
			
			int16_fp14_t rXkj = a.mat[0][j];
			if ((rXkj != 0) && (rYi0 != 0))
				mat[i][j] += mul16s_rsh(rYi0,rXkj,14);

			rXkj = a.mat[1][j];
			if ((rXkj != 0) && (rYi1 != 0))
				mat[i][j] += mul16s_rsh(rYi1,rXkj,14);

			rXkj = a.mat[2][j];
			if ((rXkj != 0) && (rYi2 != 0))
				mat[i][j] += mul16s_rsh(rYi2,rXkj,14);

		}
	}
}


void Matrix3x3::computeRotationMatrix(Rotation rotation) {
	// set rotation matrix to rotate by x-axis
	Matrix3x3 rotateByX;
	rotateByX.setRotationMatrixX( rotation.x_fp4);
		
	// set rotation matrix to rotate by y-axis
	Matrix3x3 rotateByY;
	rotateByY.setRotationMatrixY(rotation.y_fp4);
		
	// set rotation matrix to rotate by z-axis
	Matrix3x3 rotateByZ;
	rotateByZ.setRotationMatrixZ(rotation.z_fp4);

	// compute rotation matrix rotating by x and y by multiplying both rotation matrix
	Matrix3x3 rotateByXY;
	rotateByXY.multiply(rotateByX,rotateByY);
		
	// compute rotation matrix rotating by x,y,z by multiplying both rotation matrix
	multiply(rotateByXY,rotateByZ);			
}


void Matrix3x3::rotatePoint(Point &point) {
	int16_fp4_t x = 0;
	int16_fp4_t y = 0;
	int16_fp4_t z = 0;

	for(uint8_t k = 0; k<3; k++) {
		int16_fp4_t coord_fp4 = point.get(k);
		if (coord_fp4 != 0) {
			int16_fp14_t matxk = mat[0][k];
			if (matxk != 0)
				x  += mul16s_rsh(matxk,coord_fp4,14);
			matxk = mat[1][k];
			if (matxk != 0)
				y  += mul16s_rsh(matxk,coord_fp4,14);
			matxk = mat[2][k];
			if (matxk != 0)
				z  += mul16s_rsh(matxk,coord_fp4,14);						
		}
	}
	point.x_fp4 = x;
	point.y_fp4 = y;
	point.z_fp4 = z;

}

void Matrix3x3::null() {
	for (int i = 0;i<3;i++)
		for (int j = 0;j<3;j++)
			mat[i][j] = 0;
}