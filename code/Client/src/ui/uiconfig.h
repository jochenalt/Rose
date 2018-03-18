/*
 * uiconfig.h
 *
 * Configuration of UI with colors, dimensions and stuff
 *
 * Author: JochenAlt
 */

#ifndef UI_UICONFIG_H_
#define UI_UICONFIG_H_

// constants used in the UI
const float ViewEyeDistance 		= 500.0f;	// distance of the eye to the bot
const float ViewBotHeight 			= 150.0f;	// height of the bot to be viewed

const int WindowGap=10;							// gap between frame and subwindow
const int InteractiveWindowHeight=200;			// height of the interactive window

// RAL colors
#include "ral.h"

// macro to define an openGL color array of a RAL color with a specific alpha(transparency) value
#define GL_COLOR_4v(rgb,alpha) { ((float)(rgb>>16))/255.0f, ((float)((rgb>>8) & 0xFF))/255.0f, ((float)(rgb & 0xFF))/255.0f, alpha }

// transparency values
const float glAlphaSolid = 1.0;
const float glAlphaTransparent = 0.5;

//  colors of map, raster and coordinate system
static const GLfloat glCoordSystemAreaColor4v[]			= GL_COLOR_4v(TeleGrey4, glAlphaSolid );
static const GLfloat glCoordSystemAxisColor4v[] 		= GL_COLOR_4v(PearlMouseGrey, glAlphaSolid );
static const GLfloat glRasterColor4v[] 					= GL_COLOR_4v(GraniteGrey, glAlphaSolid );
static const GLfloat glMapBackgroundColor4v[]		    = GL_COLOR_4v( AnthraciteGrey, 0.01);
static const GLfloat glMapAreaColor4v[]		    		= GL_COLOR_4v( AnthraciteGrey, glAlphaTransparent);

// colors of stewart platform
static const GLfloat glBallJointColor[] 				= GL_COLOR_4v(YellowGrey, glAlphaSolid );
static const GLfloat glServoArmColor[] 					= GL_COLOR_4v(GreenBlue, glAlphaSolid );
static const GLfloat glStewartRodColor[] 				= GL_COLOR_4v(GraniteGrey, glAlphaSolid );
static const GLfloat glStewartPlateColor[] 				= GL_COLOR_4v(GreyBeige, glAlphaSolid );

// colors of head
static const GLfloat glHeadColor[] 						= GL_COLOR_4v(NutBrown, glAlphaSolid );
static const GLfloat glEyeBallsColor[] 					= GL_COLOR_4v(BlackOlive, glAlphaSolid );
static const GLfloat glIrisColor[] 						= GL_COLOR_4v(PebbleGrey, glAlphaSolid );

// colors of body in transparent mode
static const GLfloat glTranspBodyColor1[] 				= GL_COLOR_4v(NutBrown, glAlphaTransparent );
static const GLfloat glTranspBodyColor2[] 				= GL_COLOR_4v(WhiteAluminium, glAlphaTransparent );
static const GLfloat glTranspGridColor[] 				= GL_COLOR_4v(YellowGrey, glAlphaSolid );

// colors of body in non-transparent mode
static const GLfloat glBodyColor[] 						= GL_COLOR_4v(NutBrown, glAlphaSolid );
static const GLfloat glGridColor[] 						= GL_COLOR_4v(SepiaBrown, glAlphaSolid );



#endif /* UI_UICONFIG_H_ */
