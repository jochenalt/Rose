/*
 * InverseKinematics.h
 *
 * Created: 23.11.2014 18:27:41
 *  Author: JochenAlt
 */ 


#ifndef INVERSEKINEMATICS_H_
#define INVERSEKINEMATICS_H_

#include "space.h"
/** denotes the absolute position of the platform in the global coordinate system. Consists of translation and rotation
*/
#undef KINEMATICS_DEBUG
class Platform {
	public:
		Platform() {
		}
		
		void setup();
		void init();
		// value is in [mm]
		void moveByf(uint8_t axisIdentifier, float valueMM);
		void moveBy(uint8_t axisIdentifier, int16_fp4_t valueMM_fp4);

		void moveBy(Point pAdd);
		// value is in [deg]
		void rotateBy(uint8_t axisIdentifier, int16_fp4_t valueDegree_fp4);

		// value is in [deg]
		void rotateBy(const Point & pAdd);
		void setTranslation(const Point &pSet);
		void setRotationBy(const Rotation& pSet);
		void addToRotation(const Rotation& pSet);

		void addToTranslation(uint8_t axisIdentifier, int16_fp4_t value_fp4);
		void addToTranslation(const Point& pAdd) ;
		
		void addToRotationBy(const Point &pAdd);
		void addToRotationBy(uint8_t axisIdentifier, int16_fp4_t valueDegree_fp4);

		void rotateByf(uint8_t axisIdentifier, float valueDegree);
		void addToRotationByf(uint8_t axisIdentifier, float valueDegree);
		void addToTranslationf(uint8_t axisIdentifier, float valueMM);


		void println();
		void getLegCornersInWorldCoord(CornersType &corners, Tensor &transVec);

		void moveIt();
		
		// set the new position of the platform
		void set(int16_fp4_t xValue_fp4, int16_fp4_t yValue_fp4, int16_fp4_t zValue_fp4, int16_fp4_t xRot_fp4, int16_fp4_t yRot_fp4, int16_fp4_t zRot_fp4);

		void setRotationByPoint(const Point& tp, const Rotation &rot,const Point &nullPosition);	
				
		Tensor& getOrientation() { return orientation; };
			
		int16_fp0_t getAccX() { return mul16s_rsh(orientation.rot.z_fp4,9810*DEG_TO_RAD,4); };
		int16_fp0_t getAccZ() { return mul16s_rsh(orientation.rot.x_fp4,9810*DEG_TO_RAD,4); };

		void printMenuHelp();	
		void menuController();
 	private:

		Tensor orientation;
};

extern Platform platform;

#endif /* INVERSEKINEMATICS_H_ */