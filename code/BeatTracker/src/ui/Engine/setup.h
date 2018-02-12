/*
 * Types and constants used for kinematics
 *
 * Author: JochenAlt
 */

#ifndef SETUP_H_
#define SETUP_H_

#include <valarray>
#include <stdint.h>

#include "basics/types.h"

// #define KINEMATICS_LOGGING

// general walking mode. LiftBody is the phase right after beeing woken up and standing up.
// TerrainMode is the same like Walkingmode, but knees are liftet higher and
// distance sensors are used to adapt to the terrain.
enum GeneralEngineModeType { BeingAsleep, LiftBody, WalkingMode, TerrainMode, FallASleep};

// Phases of a leg during the gait
enum LegGaitPhase { LegGaitUp = 0, LegGaitDown = 1, LegGaitDuty = 2};

// several gait types
// FourLegWalk is a gait with 4 legs only
enum GaitModeType { OneLegInTheAir, TwoLegsInTheAir, SpiderWalk, Auto, FourLegWalk, None };

enum ShutDownModeType { NoShutDownActive, InitiateShutDown, ExecuteShutdown, ShutdownDone };
enum NavigationStatusType { NavPending=0, 	 	// The goal has yet to be processed by the action server
							NavActive = 1, 		// The goal is currently being processed by the action server
							NavRecalled = 2, 	// The goal received a cancel request before it started executing and was successfully cancelled (Terminal State)
							NavRejected = 3, 	// The goal was rejected by the action server without being processed, because the goal was unattainable or invali
							NavPreempted = 4, 	// The goal received a cancel request after it started executing and has since completed its execution (Terminal State)
							NavAborted = 5, 	// The goal was aborted during execution by the action server due to some failure (Terminal State)
							NavSucceeded = 6, 	// The goal was achieved successfully by the action server (Terminal State)
							NavLost = 7 		// An action client can determine that a goal is LOST. This should not be sent over the wire by an action server
};

// maximum speed a leg uses to settle down when no movement happens
const realnum legSettleDownSpeed = 50.0/1000.0; // [mm/ms]

// maximum and minimum body height
const realnum maxBodyHeight = 260.0;			// maximum height of the body (goes along with minFootTouchPointRadius)
const realnum minBodyHeight = 45.0;				// minimum height of the body (goes along with maxFootTouchPointRadius)
const realnum standardBodyHeigh = 130.0;		// default height when walking
const realnum uprightBodyHeigh = 240.0;		    // default height when walking upright

// min/max radius of ground touch points
const realnum minFootTouchPointRadius = 200.0;
const realnum standUpFootTouchPointRadius = 260;
const realnum sitDownTouchPointRadius = 300;

// max acceleration
const realnum maxAcceleration = 70.0; 		     // [mm/s^2]

// maximum speed
const realnum maxSpeed = 100.0; 				// [mm/s]

// possible acceleration of angular speed
const realnum maxAngularSpeedAcceleration = 0.6; //  [rad/s^2]
const realnum maxAngularSpeed = maxSpeed/minFootTouchPointRadius; //  [rad/s]

// walking direction cannot be changed immediately but with that angular speed per speed*t
const realnum maxAngularSpeedPerSpeed = 0.8;	 // [rad/s /  (mm/s)] = [rad/mm]

// limits of gait frequency (for beautiness of the gait mainly)
const realnum minGaitFrequency = 0.3; 				// [Hz]
const realnum maxGaitFrequency = 2.0; 				// [Hz]

// maximum speed of virtual foot ref point
const realnum maxGaitCirclePointSpeed = 50.0; 			// [mm/s}

// maximum speed of a foot that is in the air during a gait
const realnum maxFootSpeed = 400; 					// [mm/s]

// maximum speed of a foot that is in the air during a gait
const realnum maxStartupAngleSpeed = 0.3; 			// [RAD/s]

const realnum maxBodyHeightSpeed = 80.0;

// below that distance a toe is already moving with the ground
// (prevents that a movement is blocked due to the weight of the bot, such that a leg touches the ground before its computed touch point)
const realnum moveWithGroundBelowThisGroundDistance = 15.0; // [mm]

// Typically, the top point of the knee in one gait is in the
// middle of the touch point and the point when the toe leaves the ground.
// This has the consequence, that during the touch point the leg is bent against the walking direction
// which leads to a slight breaking effect that reduces the smoothness of a gait.
// The following factor allows to move this knee-zenit-point towards the walking direction
const realnum kneeZenitPointOffset = 0.3;			// [0.0..1.0]
const realnum kneeZenitPointFactor = 0.8;			// [0..1]

const realnum moveToeWhenDistanceGreaterThan = 10.0;

// must be bigger than moveToeWhenDistanceGreaterThan, otherwise we end up in a deadlock in standing up where
// the toes do not not yet move due to a small difference but the distance is too big to stand up
const realnum standUpWhenDistanceSmallerThan = moveToeWhenDistanceGreaterThan+5.0;

// the zenith of the gait is reached at this ratio in time. A symmetric movement would be 0.5
const realnum postponeZenith = 0.7;					// [%]*100 of time

#endif /* SETUP_H_ */
