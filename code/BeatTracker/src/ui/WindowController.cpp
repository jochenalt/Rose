
#include <stdio.h>
#include <assert.h>

#include "basics/util.h"

#include "uiconfig.h"
#include "setup.h"

#include "BotView.h"
#include "MoveMaker.h"
#include "DanceMove.h"

#include "WindowController.h"

using namespace std;

// Initial main window size
int WindowWidth = 600;
int WindowHeight = 800;

// GLUI Window handlers
int wMain = -1;			// main window
int wMainBotView = -1; 	// sub window with dancing bot

static const int DanceMoveRows = 2;
GLUI_RadioGroup* currentDancingModeWidget[DanceMoveRows] = { NULL, NULL };
int dancingModeLiveVar[DanceMoveRows] = { 0,0 };

GLUI_RadioGroup* currentSequenceModeWidget = NULL;
int currentSequenceModeLiveVar = 0;


/* Handler for window-repaint event. Call back when the window first appears and
 whenever the window needs to be re-painted. */
void displayMainView() {
}

void nocallback(int value) {
}

void reshape(int w, int h) {
	int savedWindow = glutGetWindow();
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, w, h);

	int MainSubWindowWidth = w - 2 * WindowGap;
	int MainSubWindowHeight = h - InteractiveWindowHeight - 3 * WindowGap;

	WindowController::getInstance().mainBotView.reshape(WindowGap, WindowGap,MainSubWindowWidth, MainSubWindowHeight);

	glutSetWindow(savedWindow);
}

void GluiReshapeCallback( int x, int y )
{
	reshape(x,y);
	int tx, ty, tw, th;
	int saveWindow = glutGetWindow();
	glutSetWindow(wMain);
	GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
	glViewport( tx, ty, tw, th );
	glutSetWindow(saveWindow);
}


void WindowController::setBodyPose(const Pose& bodyPose, const Pose& headPose) {

	Point eyeLookAt = mainBotView.getEyePosition();
	HomogeneousMatrix bodyTransformation;
	HomogeneousMatrix inverseBodyTransformation;

	createTransformationMatrix(bodyPose, bodyTransformation);
	computeInverseTransformationMatrix(bodyTransformation, inverseBodyTransformation);

	HomogeneousVector lookAtPosition = {
						eyeLookAt.x,
						eyeLookAt.y,
						eyeLookAt.z,
						1.0 };

	Point lookAtCoordFromBodysPerspective= inverseBodyTransformation * lookAtPosition;
	mainBotView.setBodyPose(bodyPose, headPose, lookAtCoordFromBodysPerspective);
}

void setDancingMoveWidget() {
	int movesPerRow = (MoveMaker::getInstance().getNumMoves()+1)/DanceMoveRows  ;
	Move::MoveType move = MoveMaker::getInstance().getCurrentMove();
	int moveNumber = (int)move;
	int row = moveNumber / movesPerRow;
	int line = moveNumber - movesPerRow*row;

	assert(row < DanceMoveRows);
	currentDancingModeWidget[row]->set_int_val(line);

	for (int i = 0;i<DanceMoveRows;i++) {
		if (i != row)
			currentDancingModeWidget[i]->set_int_val(-1);
	}
}


void currentDancingMoveCallback(int widgetNo) {
	int movesPerRow = (MoveMaker::getInstance().getNumMoves()+1)/DanceMoveRows ;
	int row = widgetNo;
	assert(row < DanceMoveRows);
	if (row == 0) {
		MoveMaker::getInstance().setCurrentMove((Move::MoveType)(dancingModeLiveVar[row]));
	}
	else
		MoveMaker::getInstance().setCurrentMove((Move::MoveType)(dancingModeLiveVar[row] + row*movesPerRow));
}

void setSequenceModeWidget() {
	currentSequenceModeWidget->set_int_val((int)MoveMaker::getInstance().getSequenceMode());
}

void currentSequenceModeCallback(int widgetNo) {
	MoveMaker::getInstance().setSequenceMode((MoveMaker::SequenceModeType)currentSequenceModeLiveVar);
}

GLUI* WindowController::createInteractiveWindow(int mainWindow) {
	GLUI *windowHandle= GLUI_Master.create_glui_subwindow( wMain,  GLUI_SUBWINDOW_BOTTOM);
	windowHandle->set_main_gfx_window( wMain );

	GLUI_Panel* interactivePanel = new GLUI_Panel(windowHandle,"interactive panel", GLUI_PANEL_NONE);
	GLUI_StaticText* text = NULL;
	GLUI_Panel* dancingModePanel[DanceMoveRows];
	int moveCounter = 0;
	for (int row = 0;row < DanceMoveRows; row++) {
		dancingModePanel[row] = new GLUI_Panel(interactivePanel,"Dancing Mode Panel", GLUI_PANEL_RAISED);

		currentDancingModeWidget[row] =  new GLUI_RadioGroup(dancingModePanel[row], dancingModeLiveVar + row, row, currentDancingMoveCallback);

		while (moveCounter < (MoveMaker::getInstance().getNumMoves() +1) / DanceMoveRows * (row+1)) {
			Move& move = Move::getMove((Move::MoveType)moveCounter);
			new GLUI_RadioButton(currentDancingModeWidget[row], move.getName().c_str());

			moveCounter++;
		}

		// fill up with empty lines to have the containers of the same height
		if (row == DanceMoveRows-1) {
			for (int lines = moveCounter;lines < MoveMaker::getInstance().getNumMoves()-MoveMaker::getInstance().getNumMoves()%2;lines++)
				new GLUI_StaticText(dancingModePanel[row],"");
		}

		// add a column for next row
		if (row < DanceMoveRows)
			windowHandle->add_column_to_panel(interactivePanel, false);

	}

	GLUI_Panel* sequenceModePanel= new GLUI_Panel(interactivePanel,"Sequence Mode", GLUI_PANEL_RAISED);
	text = new GLUI_StaticText(sequenceModePanel,"Sequence Move");
	text->set_alignment(GLUI_ALIGN_LEFT);

	currentSequenceModeWidget =  new GLUI_RadioGroup(sequenceModePanel, &currentSequenceModeLiveVar, 0, currentSequenceModeCallback);
	new GLUI_RadioButton(currentSequenceModeWidget, "Automated Sequence");
	new GLUI_RadioButton(currentSequenceModeWidget, "Selected Dance Move");

	return windowHandle;
}


bool WindowController::setup(int argc, char** argv) {

	glutInit(&argc, argv);

	// start the initialization in a thread so that this function returns
	// (the thread runs the endless GLUT main loop)
	// main thread can do something else while the UI is running
	eventLoopThread = new std::thread(&WindowController::UIeventLoop, this);

	// wait at most 20s until UI is ready
	unsigned long startTime  = millis();
	do { delay_ms(10); }
	while ((millis() - startTime < 20000) && (!uiReady));

	return uiReady;
}

void WindowController::tearDown() {
	glutExit();
}

// Idle callback is called by GLUI when nothing is to be done
void idleCallback( void )
{
	const int refreshRate = 20; // [Hz]
	const milliseconds refreshRate_ms = 1000/refreshRate/2;
	milliseconds now = millis();
	static milliseconds lastDisplayRefreshCall = millis();

	// update all screens with 25Hz once a second in if new data is there
	if ((now - lastDisplayRefreshCall > refreshRate_ms)) {
		// use a second chance loop in order to limit fps to a maximum
		// ( there are other sources as well that can post a redisplay)
		if (WindowController::getInstance().mainBotView.isJustDisplayed()) // if it has been just display, reset display flag
			WindowController::getInstance().mainBotView.resetDisplayFlag();
		else {
			WindowController::getInstance().mainBotView.postRedisplay(); // post if we reset the flag in a previous run
		}
		lastDisplayRefreshCall = now;

		setDancingMoveWidget();
	}

	// be cpu friendly
	delay_ms(5);
}


void WindowController::UIeventLoop() {

	glutInitWindowSize(WindowWidth, WindowHeight);
    wMain = glutCreateWindow("Donna"); // Create a window with the given title
	glutInitWindowPosition(20, 20); // Position the window's initial top-left corner
	glutDisplayFunc(displayMainView);
	glutReshapeFunc(reshape);

	GLUI_Master.set_glutReshapeFunc( GluiReshapeCallback );
	GLUI_Master.set_glutIdleFunc( idleCallback);

	wMainBotView = mainBotView.create(wMain,"");

	// Main Bot view has comprehensive mouse motion
	glutSetWindow(wMainBotView);

	// double buffering
	glutInitDisplayMode(GLUT_DOUBLE);

	// initialize all widgets
	createInteractiveWindow(wMain);

	uiReady = true; 							// flag to tell calling thread to stop waiting for ui initialization
	glutMainLoop();
}

