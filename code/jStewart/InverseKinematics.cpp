/*
 * InverseKinematics.cpp
 *
 * Created: 23.11.2014 18:28:01
 *  Author: JochenAlt
 */ 

#include "Arduino.h"
#include "InverseKinematics.h"
#include "ServoLegs.h"
#include "FastMath.h"
#include "PatternBlinker.h"
#include "PlatformCircleTrajectory.h"
#include <avr/wdt.h>

PlatformCircleTrajectory circle;
extern PatternBlinker LedBlinker;

Platform platform;
extern Trajectory platformTrajectory;
Rotation rotationByPoint;
Point rotationPoint;
bool rotationPointOn = false;



void Platform::init() {
	orientation.trans.set(0,0,0);
	orientation.rot.set(0,0,0);	
}

void Platform::println() {
	Serial.print( F("platform = [("));
	Serial.print( FP2FLOAT(orientation.trans.x_fp4,4));
	Serial.print(",");
	Serial.print( FP2FLOAT(orientation.trans.y_fp4,4));
	Serial.print(",");
	Serial.print( FP2FLOAT(orientation.trans.z_fp4,4));
	Serial.print(")");

	Serial.print( F("("));
	Serial.print( FP2FLOAT(orientation.rot.x_fp4,4));
	Serial.print(",");
	Serial.print( FP2FLOAT(orientation.rot.y_fp4,4));
	Serial.print(",");
	Serial.print( FP2FLOAT(orientation.rot.z_fp4,4));
	Serial.println(")]");
}

/*
Set the 6 corners to the top legs in coordinates of the the world coordinate system, rotated and translated by
the passed vector transVec.
*/
void Platform::getLegCornersInWorldCoord(CornersType &corners, Tensor &transVec)
{
	// null out the corners first
	for (uint8_t cornerIdx = 0;cornerIdx<6;cornerIdx++)
		corners[cornerIdx].null();

	// Matrix3x3 rotateByXYZ;
	Matrix3x3 rotateByXYZ;
	rotateByXYZ.computeRotationMatrix(transVec.rot);
			
	// apply final rotation matrix taking platform corners and rotation them by all axis
	for(uint8_t cornerIdx = 0; cornerIdx<6; cornerIdx++) {
			Point point;
			point.set(PlatformCornerCoord_fp4[cornerIdx][0],
					  PlatformCornerCoord_fp4[cornerIdx][1],
					  PlatformCornerCoord_fp4[cornerIdx][2]);

			rotateByXYZ.rotatePoint(point);
			corners[cornerIdx] = point;
	}
	
	// move platform corners by translation vector given in the passed tensor
	for (uint8_t cornerIdx = 0;cornerIdx<6;cornerIdx++)
		corners[cornerIdx].translate(transVec.trans);
}

void Platform::moveIt() {	 
	CornersType corners;	// coordinates of the leg corners in world coordinate system
	// returns the corners after transformation along orientation tensor
	getLegCornersInWorldCoord(corners, orientation);
	
	// per servo, pass the leg coordinates and turn the servo accordingly
	// internally, a corner is transformed into the coord system of the servo (
	for(int cornerIdx = 0; cornerIdx < 6; cornerIdx++) {
		servoLegs.setServoAngleByLegCorner(cornerIdx,corners[cornerIdx].x_fp4, corners[cornerIdx].y_fp4, corners[cornerIdx].z_fp4);
	}
}

void Platform::setup() {
		init();
}

// value is in [mm]
void Platform::moveByf(uint8_t axisIdentifier, float valueMM) {
		moveBy(axisIdentifier, FLOAT2FP16(valueMM,4));
}

void Platform::moveBy(uint8_t axisIdentifier, int16_fp4_t valueMM_fp4) {
	orientation.trans.get(axisIdentifier) += valueMM_fp4;
}


void Platform::moveBy(Point pAdd) {
	orientation.trans.translate(pAdd);
}

void Platform::rotateByf(uint8_t axisIdentifier, float valueDegree) {
	rotateBy(axisIdentifier, FLOAT2FP16(valueDegree,4));
}

void Platform::rotateBy(uint8_t axisIdentifier, int16_fp4_t valueDegree_fp4) {
	orientation.rot.get(axisIdentifier) += valueDegree_fp4;
}

void Platform::rotateBy(const Point &pAdd) {
		orientation.rot.translate(pAdd);
}

void Platform::set(int16_fp4_t xValue_fp4, int16_fp4_t  yValue_fp4, int16_fp4_t  zValue_fp4, int16_fp4_t  xRot_fp4, int16_fp4_t  yRot_fp4, int16_fp4_t  zRot_fp4) {
	orientation.trans.set(xValue_fp4, yValue_fp4,zValue_fp4);
	orientation.rot.set(xRot_fp4,yRot_fp4,zRot_fp4);
}

// value is in [mm]
void Platform::addToTranslationf(uint8_t axisIdentifier, float valueMM) {
	addToTranslation(axisIdentifier,FLOAT2FP16(valueMM, 4));
}

void Platform::addToTranslation(uint8_t axisIdentifier, int16_fp4_t valueMM_fp4) {
	orientation.trans.get(axisIdentifier) += valueMM_fp4;
}

void Platform::addToTranslation(const Point &pAdd) {
	orientation.trans.translate(pAdd);
}

void Platform::addToRotationByf(uint8_t axisIdentifier, float valueDegree) {
	addToRotationBy(axisIdentifier, FLOAT2FP16(valueDegree,4));
}

void Platform::addToRotationBy(uint8_t axisIdentifier, int16_fp4_t valueDegree_fp4) {
	orientation.rot.get(axisIdentifier) += valueDegree_fp4;
}

void Platform::addToRotationBy(const Point &pAdd) {
	orientation.rot.translate(pAdd);
}

void Platform::setRotationBy(const Rotation& pSet) {
	orientation.rot = pSet;
}

void Platform::addToRotation(const Rotation& pSet) {
	orientation.rot.x_fp4 += pSet.x_fp4;
	orientation.rot.y_fp4 += pSet.y_fp4;
	orientation.rot.z_fp4 += pSet.z_fp4;
}

void Platform::setTranslation(const Point& pSet) {
	orientation.trans = pSet;
}

void Platform::setRotationByPoint(const Point &tp, const Rotation &rotation, const Point &nullPosition) {
	// dont rotate around origin, but around the passed point tp
	// compute the rotation matrix, rotating the touch point and translating it so that it comes back to its original position
	Matrix3x3 rotateByXZ;
	rotateByXZ.computeRotationMatrix(rotation);
	
	// apply rotation matrix to touchpoint
	Point rotatedTouchPoint;
	rotatedTouchPoint = tp;
	rotateByXZ.rotatePoint(rotatedTouchPoint);
	
	//  rotatedTouchpoint contains the movement of the touchpoint after rotation
	// compute the translation by taking that position and translating it to the touchpoint before rotation.
	Point translation;
	translation.set(tp.x_fp4 - rotatedTouchPoint.x_fp4 ,-rotatedTouchPoint.y_fp4,tp.z_fp4 - rotatedTouchPoint.z_fp4);

	// now we have the given rotation by the PID controller and the
	// translation that will keep the touchpoint in its original position while rotation
	platform.setRotationBy(rotation);
	platform.setTranslation(translation);
	platform.addToTranslation(nullPosition);
}



void Platform::printMenuHelp() {
	Serial.println(F("Kinematic"));
	Serial.println(F("_       - set null orientation"));
	Serial.println(F("a/w/s/y - translation (X,Z)"));
	Serial.println(F("d/f     - up/down (Y)"));
	Serial.println(F("u/i     - rotate (X)"));
	Serial.println(F("j/k     - rotate (Y)"));
	Serial.println(F("n/m     - rotate (Z)"));
	Serial.println(F("h       - this page"));

	Serial.println();
	servoLegs.printlnCalibrationData();
	platform.println();
}


void Platform::menuController() {
		while (true) {
			wdt_reset();
			platformTrajectory.loop();
			LedBlinker.loop();
			
			if (Serial.available()) {
				char inputChar = Serial.read();
				
				// no, check interactive commands
				switch (inputChar) {
					case 'h':
						platform.printMenuHelp();
						break;
					case 's':
						if (rotationPointOn) {
							rotationPoint.setf(X_INDEX, rotationPoint.getf(X_INDEX) + 1);
							rotationPoint.println("rotationPoint");
						}else
						platform.addToTranslationf(X_INDEX,1);
						platform.moveIt();
						break;
					case 'a':
						if (rotationPointOn) {
							rotationPoint.setf(X_INDEX, rotationPoint.getf(X_INDEX) -1);
							rotationPoint.println("rotationPoint");
						}else
						platform.addToTranslationf(X_INDEX,-1);
						platform.moveIt();
						break;
					case 'w':
						if (rotationPointOn) {
							rotationPoint.setf(Z_INDEX, rotationPoint.getf(Z_INDEX) + 1);
							rotationPoint.println("rotationPoint");
						}else
						platform.addToTranslationf(Z_INDEX,1);
						platform.moveIt();
						break;
					case 'y':
						if (rotationPointOn) {
							rotationPoint.setf(Z_INDEX, rotationPoint.getf(Z_INDEX) - 1);
							rotationPoint.println("rotationPoint");
						}else
						platform.addToTranslationf(Z_INDEX,-1);
						platform.moveIt();
						break;
					case 'd':
						platform.addToTranslationf(Y_INDEX,1);
						platform.moveIt();
						break;
					case 'f':
						platform.addToTranslationf(Y_INDEX,-1);
						platform.moveIt();
						break;
					case 'j':
						if (rotationPointOn) {
							rotationPoint.setf(Y_INDEX, rotationPoint.getf(Y_INDEX) + 0.5);
							Point p;
						
							platform.setRotationByPoint(rotationPoint, rotationByPoint, p);
						} else
						platform.addToRotationByf(Y_INDEX,0.5);
						platform.moveIt();
						break;
					case 'k':
						if (rotationPointOn) {
							rotationPoint.setf(Y_INDEX, rotationPoint.getf(Y_INDEX) - 0.5);
							Point p;
							platform.setRotationByPoint(rotationPoint, rotationByPoint,p);
						} else
						platform.addToRotationByf(Y_INDEX,-0.5);
						platform.moveIt();
						break;
					case 'u':
						if (rotationPointOn) {
							rotationPoint.setf(X_INDEX, rotationPoint.getf(X_INDEX) + 0.5);
							rotationByPoint.println("rot");
							Point p;

							platform.setRotationByPoint(rotationPoint, rotationByPoint,p);
						} else
						platform.addToRotationByf(X_INDEX,0.5);
						platform.moveIt();
						break;
					case 'i':
						if (rotationPointOn) {
							rotationPoint.setf(X_INDEX, rotationPoint.getf(X_INDEX) - 0.5);
							rotationByPoint.println("rot");
							Point p;
							platform.setRotationByPoint(rotationPoint, rotationByPoint,p);
						} else
						platform.addToRotationByf(X_INDEX,-0.5);
						platform.moveIt();
						break;
					case 'n':
						if (rotationPointOn) {
							rotationPoint.setf(Z_INDEX, rotationPoint.getf(Z_INDEX) + 0.5);
							rotationByPoint.println("rot");
							Point p;
							platform.setRotationByPoint(rotationPoint, rotationByPoint,p);
							} else {
							platform.addToRotationByf(Z_INDEX,0.5);
							platform.getOrientation().rot.println("rot");
						}
						platform.moveIt();
						break;
					case 'm':
						if (rotationPointOn) {
							rotationPoint.setf(Z_INDEX, rotationPoint.getf(Z_INDEX) - 0.5);
							rotationByPoint.println("rot");
							Point p;
							platform.setRotationByPoint(rotationPoint, rotationByPoint,p);
							} else {
							platform.addToRotationByf(Z_INDEX,-0.5);
							platform.getOrientation().rot.println("rot");
						}
						platform.moveIt();
						break;
					case 'D':
						if (debug) {
							Serial.println(F("debug off "));
							debug = false;
						}
						else {
							Serial.println(F("debug on "));
							debug = true;
						}
						break;
					case 't':
						if (rotationPointOn) {
							Serial.println(F("set rotation point off"));
							rotationPointOn= false;
						}
						else {
							Serial.println(F("set rotation point on"));
							rotationPointOn = true;
						}
						break;
					case ' ': {
						Point home;
						Rotation rot;
						platform.setTranslation(home);
						platform.setRotationBy(rot);
						platform.moveIt();
					}
					break;
					case 'c': {
						platformTrajectory.setActualTrajectory(&circle);
						Point center;
						center.setf(0,0,0);
						circle.setCircleTrajectory(center,10,35);
						Serial.println(F("set platform circle movement"));
						}
						break;
					case '_': {
						Serial.println(F("set null position"));
						servoLegs.defineCurrentPositionAsNull();
						platform.setTranslation(Point());
						platform.setRotationBy(Rotation());
						platform.moveIt();
						break;
						}
					case '\e':
						return;

					default:
						break;
				} // switch input char
			} // if (Serial.available)
		}
}

