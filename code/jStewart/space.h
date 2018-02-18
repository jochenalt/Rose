/*
 * Space.h
 *
 * Created: 25.11.2014 16:19:58
 *  Author: JochenAlt
 */ 


#ifndef SPACE_H_
#define SPACE_H_

#include "Arduino.h"
#include "FixedPoint.h"

#define X_INDEX 0
#define Y_INDEX 1
#define Z_INDEX 2

class Point {
	public:
		Point();
		Point(const Point& p);
		Point(float x,float y, float z) {
			x_fp4 = FLOAT2FP16(x,4);
			y_fp4 = FLOAT2FP16(y,4);
			z_fp4 = FLOAT2FP16(z,4);
		}		

		void translate(Point pPoint);
		void setf(float pX, float pY,float pZ);
		void setf(uint8_t axisIdx, float value) {
			set(axisIdx, FLOAT2FP16(value,4));
		}
		void set(int16_fp4_t pX, int16_fp4_t pY,int16_fp4_t  pZ) {
			x_fp4 = pX;
			y_fp4 = pY;
			z_fp4 = pZ;
			
		}

		void set(uint8_t axisIdx, int16_fp4_t value) {
			switch (axisIdx) {
				case X_INDEX:
					x_fp4 = value;break;
				case Y_INDEX:
					y_fp4 = value;break;
				case Z_INDEX:
					z_fp4 = value;break;
				default:
				break;
			}
		}


		void null();
		// float operator[](uint8_t idx);
		float getf(uint8_t idx) {
			switch (idx) {
				case X_INDEX:
					return FP2FLOAT(x_fp4,4);break;
				case Y_INDEX:
					return FP2FLOAT(y_fp4,4);break;
				case Z_INDEX:
					return FP2FLOAT(z_fp4,4);break;
				default:
				break;
			}
			Serial.println(F("error in []"));
			return FP2FLOAT(x_fp4,4);
		}


		int16_fp4_t& get(uint8_t idx) {
			switch (idx) {
				case X_INDEX:
					return x_fp4;break;
				case Y_INDEX:
					return y_fp4;break;
				case Z_INDEX:
					return z_fp4;break;
				default:
				break;
			}
			Serial.println(F("error[]"));
			return x_fp4;;
		}
	
		void operator= (Point p);
		void print();
		void print(String s);
		void println();
		void println(String s);

		// members of a point, needs to be public for access (plus [] operator)
		int16_fp2_t x_fp4;
		int16_fp2_t y_fp4;
		int16_fp2_t z_fp4;
};

class Rotation : public Point {
	public:
		Rotation () {
			x_fp4 = 0;
			y_fp4 = 0;
			z_fp4 = 0;
		};
		Rotation(const Rotation& p) : Point(p) {
			x_fp4 = p.x_fp4;
			y_fp4 = p.y_fp4;
			z_fp4 = p.z_fp4;			
		};

};



// Tensor represents a position and
class Tensor {
	public:
		Tensor() { };
			void null() {
				trans.null();
				rot.null();
			}
		Tensor(Tensor& tensor) {
			trans = tensor.trans;
			rot = tensor.rot;
		 };
		void operator=(Tensor& tensor) {
			trans = tensor.trans;
			rot = tensor.rot;
		};

		void setPoint (const Point& pPoint);
		void setRotation (const Rotation& pRot);
		Point& getPoint ();
		Rotation& getRotation ();

		Point trans;
		Rotation rot;
};


typedef Point CornersType[6];

class Matrix3x3 {
	public:
		Matrix3x3() {
			null(); 
		}
		
		void setRotationMatrixX(int16_fp4_t  degree);
		void setRotationMatrixY(int16_fp4_t degree);
		void setRotationMatrixZ(int16_fp4_t degree);
		void multiply(Matrix3x3 a,Matrix3x3 b);
		
		void null();
		void computeRotationMatrix(Rotation rotation);
		void rotatePoint(Point &point);
		private:
			int16_fp14_t mat[3][3];
};


#endif /* SPACE_H_ */