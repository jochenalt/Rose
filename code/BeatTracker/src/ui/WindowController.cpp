
#include <stdio.h>

#include "basics/logger.h"
#include "basics/util.h"
#include "MoveMaker.h"
#include "WindowController.h"


#include "BotView.h"
#include "uiconfig.h"
#include "setup.h"

using namespace std;

// Initial main window size
int WindowWidth = 600;
int WindowHeight = 750;

// GLUI Window handlers
int wMain;			// main window
int wMainBotView; 	// sub window with dancing bot

GLUI_RadioGroup* currentDancingModeWidget = NULL;
int dancingModeLiveVar = 0;

GLUI_RadioGroup* currentSequenceModeWidget = NULL;
int currentSequenceModeLiveVar = 0;

// each mouse motion call requires a display() call before doing the next mouse motion call. postDisplayInitiated
// is a semaphore that coordinates this across several threads
// (without that, we have so many motion calls that rendering is bumpy)
// postDisplayInitiated is true, if a display()-invokation is pending but has not yet been executed (i.e. allow a following display call)
volatile static bool postDisplayInitiated = true;


void WindowController::postRedisplay() {
	int saveWindow = glutGetWindow();
	glutSetWindow(wMain);
	postDisplayInitiated = true;
	glutPostRedisplay();
	glutSetWindow(saveWindow );
}


/* Handler for window-repaint event. Call back when the window first appears and
 whenever the window needs to be re-painted. */
void displayMainView() {
	if (!postDisplayInitiated)
		return;

	postDisplayInitiated = false;
	glClear(GL_COLOR_BUFFER_BIT);
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
	WindowController::getInstance().postRedisplay();
}



void WindowController::setBodyPose(const Pose& bodyPose) {
	mainBotView.setBodyPose(bodyPose);

	// post a refresh with 50Hz max
	static uint32_t lastCall = 0;
	uint32_t now = millis();
	if (now - lastCall > 20)
		glutPostRedisplay();
	lastCall = now;
}

void setDancingMoveWidget() {
	currentDancingModeWidget->set_int_val((int)MoveMaker::getInstance().getCurrentMove());
}


void currentDancingMoveCallback(int widgetNo) {
	MoveMaker::getInstance().setCurrentMove((MoveMaker::MoveType)dancingModeLiveVar);
}

void setSequenceModeWidget() {
	currentSequenceModeWidget->set_int_val((int)MoveMaker::getInstance().getSequenceMode());
}

void currentSequenceModeCallback(int widgetNo) {
	MoveMaker::getInstance().setSequenceMode((MoveMaker::SequenceModeType)currentSequenceModeLiveVar);
}

GLUI* WindowController::createInteractiveWindow(int mainWindow) {

	string emptyLine = "                                               ";

	GLUI *windowHandle= GLUI_Master.create_glui_subwindow( wMain,  GLUI_SUBWINDOW_BOTTOM);
	windowHandle->set_main_gfx_window( wMain );

	GLUI_Panel* interactivePanel = new GLUI_Panel(windowHandle,"interactive panel", GLUI_PANEL_NONE);

	GLUI_Panel* dancingModePanel = new GLUI_Panel(interactivePanel,"Dancing Mode Panel", GLUI_PANEL_NONE);
	GLUI_StaticText* text = new GLUI_StaticText(dancingModePanel,"Current Dance Move");
	text->set_alignment(GLUI_ALIGN_LEFT);

	currentDancingModeWidget =  new GLUI_RadioGroup(dancingModePanel, &dancingModeLiveVar, 0, currentDancingMoveCallback);

	for (int i = 0;i<MoveMaker::getInstance().getNumMoves();i++) {
		MoveMaker::MoveType move = (MoveMaker::MoveType)i;
		new GLUI_RadioButton(currentDancingModeWidget, MoveMaker::getInstance().getMoveName(move).c_str());
	}

	windowHandle->add_column_to_panel(interactivePanel, false);

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

	// wait until UI is ready
	unsigned long startTime  = millis();
	do { delay_ms(10); }
	while ((millis() - startTime < 20000) && (!uiReady));

	return uiReady;
}


// Idle callback is called by GLUI when nothing is to do.
void idleCallback( void )
{
	const milliseconds emergencyRefreshRate = 1000; 		// refresh everything once a second at least due to refresh issues


	milliseconds now = millis();
	static milliseconds lastDisplayRefreshCall = millis();

	// update all screens once a second in case of refresh issues (happens)
	if ((now - lastDisplayRefreshCall > emergencyRefreshRate)) {
		WindowController::getInstance().mainBotView.postRedisplay();

		setDancingMoveWidget();
	}
}


void WindowController::UIeventLoop() {
	LOG(DEBUG) << "BotWindowCtrl::UIeventLoop";

	glutInitWindowSize(WindowWidth, WindowHeight);
    wMain = glutCreateWindow("Private Dancer"); // Create a window with the given title
	glutInitWindowPosition(20, 20); // Position the window's initial top-left corner
	glutDisplayFunc(displayMainView);
	glutReshapeFunc(reshape);

	GLUI_Master.set_glutReshapeFunc( GluiReshapeCallback );
	GLUI_Master.set_glutIdleFunc( idleCallback);

	wMainBotView= mainBotView.create(wMain,"");

	// Main Bot view has comprehensive mouse motion
	glutSetWindow(wMainBotView);

	// double buffering
	glutInitDisplayMode(GLUT_DOUBLE);

	// initialize all widgets
	createInteractiveWindow(wMain);

	uiReady = true; 							// flag to tell calling thread to stop waiting for ui initialization
	LOG(DEBUG) << "starting GLUT main loop";
	glutMainLoop();
}

