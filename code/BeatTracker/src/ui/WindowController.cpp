
#include <stdio.h>

#include "basics/logger.h"
#include "basics/util.h"

#include "WindowController.h"

#include "BotView.h"
#include "uiconfig.h"
#include "setup.h"

using namespace std;

// Initial Window size
int WindowWidth = 1200;
int WindowHeight = 750;

// GLUI Window handlers
int wMain, wMainBotView, wSLAMView;

// kinematics widgets
// target body position, orientation and speed
Pose inputBodyPose;
realnum inputWalkingDirection = 0;
realnum inputSpeed = 0;
realnum inputRotate = 0;
realnum inputNoseOrientation = 0;

// body pose widget
GLUI_Spinner* bodyPosePositionSpinner[3] = { NULL, NULL, NULL};
int bodyPosePositionSpinnerLiveVar[3] = {0,0,0};
const int bodyPosePositionWidgetNo = 0;
GLUI_Spinner* bodyPoseOrientationSpinner[3] = { NULL,NULL, NULL};
int bodyPoseOrientationSpinnerLiveVar[3] = {0,0,0};
const int bodyPoseOrientationWidgetNo = 3;
const int bodyPoseResetWidgetNo = 7;

// choice of single leg or full pentapod
GLUI_Checkbox* singleLegActiveCheckBox = NULL;
int singleLegActiveLiveVar;

// gait controls
GLUI_Spinner* gaitWakingDirectionSpinner = NULL;
GLUI_Spinner* gaitSpeedSpinner = NULL;
GLUI_Spinner* gaitRotateSpinner = NULL;
GLUI_Spinner* gaitNoseOrientationSpinner = NULL;

int gaitDirectionLiveVar = 0;
int gaitSpeedLiveVar = 0;
int gaitRotateLiveVar = 0;
int gaitNoseOrientationLiveVar = 0;

const int WalkingDirectionID = 0;
const int MovementSpeedID = 1;
const int MovementRotationID = 2;
const int MovementOrientationID = 3;
const int MovementResetID = 4;
GLUI_RadioGroup* gaitRadioGroup = NULL;
int gaitLiveVar = 0;

GLUI_RadioGroup* scriptRadioGroup = NULL;
GLUI_StaticText* scriptText = NULL;
int scriptLiveVar = 0;

GLUI_Checkbox* mapControl = NULL;
int mapLiveVar;

GLUI_Checkbox* powerCheckbox = NULL;
int powerLiveVar;
const int PowerCheckBoxID = 0;
GLUI_Checkbox* wakeUpCheckbox = NULL;
int wakeUpLiveVar;
const int WakeUpCheckBoxID = 1;

GLUI_Checkbox* terrainModeCheckbox = NULL;
int terrainLiveVar;
const int TerrainModeCheckBoxID = 2;


// each mouse motion call requires a display() call before doing the next mouse motion call. postDisplayInitiated
// is a semaphore that coordinates this across several threads
// (without that, we have so many motion calls that rendering is bumpy)
// postDisplayInitiated is true, if a display()-invokation is pending but has not yet been executed (i.e. allow a following display call)
volatile static bool postDisplayInitiated = true;

extern void copyMovementToView();
extern void mapFunction (Point &p);


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
	// glutSetWindow(wMain);
	glClear(GL_COLOR_BUFFER_BIT);
}

void nocallback(int value) {
}

void reshape(int w, int h) {
	int savedWindow = glutGetWindow();
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, w, h);

	int MainSubWindowWidth = (w - 3 * WindowGap)/2;
	int MainSubWindowHeight = (h - InteractiveWindowHeight - 3 * WindowGap);

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


// Idle callback is called by GLUI when nothing is to do.
void idleCallback( void )
{
	const milliseconds emergencyRefreshRate = 1000; 		// refresh everything once a second at least due to refresh issues

	milliseconds now = millis();

	bool newBotData = true;

	static milliseconds lastDisplayRefreshCall = millis();
	bool doSomething = false;
	// update all screens once a second in case of refresh issues (happens)
	if (newBotData || (now - lastDisplayRefreshCall > emergencyRefreshRate)) {
		WindowController::getInstance().mainBotView.postRedisplay();
		doSomething = true;
	}

	if ((now - lastDisplayRefreshCall > emergencyRefreshRate) || newBotData) {
		doSomething = true;
		lastDisplayRefreshCall = now;
	}

	// be cpu friendly
	if (!doSomething)
		delay_ms(5);
}


void WindowController::setBodyPose(const Pose& bodyPose) {
	mainBotView.setBodyPose(bodyPose);
}


GLUI* WindowController::createInteractiveWindow(int mainWindow) {

	string emptyLine = "                                               ";

	GLUI *windowHandle= GLUI_Master.create_glui_subwindow( wMain,  GLUI_SUBWINDOW_BOTTOM);
	windowHandle->set_main_gfx_window( wMain );

	GLUI_Panel* interactivePanel = new GLUI_Panel(windowHandle,"interactive panel", GLUI_PANEL_NONE);
	GLUI_Panel* kinematicsPanel = new GLUI_Panel(interactivePanel,"kinematics panel", GLUI_PANEL_NONE);

	// int i = actuatorConfigType[0].minAngle;
	GLUI_StaticText* text = new GLUI_StaticText(kinematicsPanel,"Single Leg Inverse Kinematics");
	text->set_alignment(GLUI_ALIGN_CENTER);
	return windowHandle;
}


bool WindowController::setup(int argc, char** argv) {
	glutInit(&argc, argv);

	// start the initialization in a thread so that this function returns
	// (the thread runs the endless GLUT main loop)
	// So, the main thread can do something different while the UI is running
	eventLoopThread = new std::thread(&WindowController::UIeventLoop, this);

	// wait until UI is ready
	unsigned long startTime  = millis();
	do { delay_ms(10); }
	while ((millis() - startTime < 20000) && (!uiReady));

	return uiReady;
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

