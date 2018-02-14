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
const float ViewEyeDistance 		= 1000.0f;	// distance of the eye to the bot
const float ViewBotHeight 			= 200.0f;	// height of the bot to be viewed
const float ViewMapEyeDistance 		= 5000.0f;	// distance of the eye to the map

const int WindowGap=10;							// gap between frame and subwindow
const int InteractiveWindowHeight=100;			// height of the interactive window

// RAL colors
#include "ral.h"

// macro to define an openGL color array of a RAL color with a specific alpha(transparency) value
#define GL_COLOR_4v(rgb,alpha) { ((float)(rgb>>16))/255.0f, ((float)((rgb>>8) & 0xFF))/255.0f, ((float)(rgb & 0xFF))/255.0f, alpha }

// transparency values
const float glAlphaSolid = 1.0;
const float glAlphaTransparent = 0.5;

static const GLfloat glCoordSystemAreaColor4v[]			= GL_COLOR_4v(TeleGrey4, glAlphaSolid );
static const GLfloat glCoordSystemAxisColor4v[] 		= GL_COLOR_4v(PearlMouseGrey, glAlphaSolid );
static const GLfloat glRasterColor4v[] 					= GL_COLOR_4v(GraniteGrey, glAlphaSolid );

static const GLfloat glEyesColor[] 					= GL_COLOR_4v(WineRed, glAlphaSolid );
static const GLfloat glMapBackgroundColor4v[]		    = GL_COLOR_4v( AnthraciteGrey, 0.01);
static const GLfloat glMapAreaColor4v[]		    		= GL_COLOR_4v( AnthraciteGrey, glAlphaTransparent);

#endif /* UI_UICONFIG_H_ */
