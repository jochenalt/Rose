
#include <stdio.h>
#include <assert.h>
#include <dance/Dancer.h>
#include <dance/Move.h>
#include <basics/util.h>
#include <basics/stringhelper.h>

#include <client/BotClient.h>

#include <ui/uiconfig.h>
#include <ui/setup.h>

#include <ui/BotView.h>
#include <ui/WindowController.h>

using namespace std;

// Initial main window size
int WindowWidth = 1000;
int WindowHeight = 900;

// GLUI Window handlers
int wMain = -1;			// main window
int wMainBotView = -1; 	// sub window with dancing bot

static const int DanceMoveRows = 2;
GLUI_RadioGroup* currentDancingModeWidget[DanceMoveRows] = { NULL, NULL };
int dancingModeLiveVar[DanceMoveRows] = { 0,0 };

GLUI_RadioGroup* currentSequenceModeWidget = NULL;
int currentSequenceModeLiveVar = 0;

GLUI_RadioGroup* stripperRadioButtons = NULL;
int clothesOnLiveVar = 0;

GLUI_Checkbox* transparentCheckbox = NULL;
int transparentLiveVar = 0;

GLUI_Scrollbar* ambitionSpinner = NULL;
int ambitionLiveVar = 0;

GLUI_FileBrowser* fileBrowser = NULL;

GLUI_Checkbox* musicDetectedBox  = NULL;
int musicDetectedLiveVar  = 0;
bool isMusicDetected = false;

GLUI_StaticText* statusTextWidget = NULL;
string currentStatus;
uint32_t timeOfStatus = 0;

WindowController instance;


void WindowController::setStatus(string status) {
	if (status != "") {
		currentStatus = status;
		timeOfStatus = millis();
	}
}

string getStatus() {
	if ((timeOfStatus > 0) && (millis() > timeOfStatus + 5000)) {
		currentStatus = "";
		timeOfStatus = 0;
	}
	return currentStatus;
}

WindowController& WindowController::getInstance() {
	return instance;
}

// Handler for window-repaint event. Call back when the window first appears and
// whenever the window needs to be re-painted.
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

void WindowController::setMusicDetected (bool musicDetected) {
	isMusicDetected = musicDetected;
}

void setMusicDetectedWidget() {
	musicDetectedBox->set_int_val(isMusicDetected?1:0);
}

void setDancingMoveWidget() {
	int movesPerRow = (Dancer ::getInstance().getNumMoves()+1)/DanceMoveRows  ;
	Move::MoveType move = Dancer ::getInstance().getCurrentMove();
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
	int movesPerRow = (Dancer ::getInstance().getNumMoves()+1)/DanceMoveRows ;
	int row = widgetNo;
	assert(row < DanceMoveRows);
	Move::MoveType newMove = (Move::MoveType)(dancingModeLiveVar[row] + row*movesPerRow);
	BotClient::getInstance().setMove(newMove);
	Dancer::getInstance().setCurrentMove(newMove);
}

void setSequenceModeWidget() {
	currentSequenceModeWidget->set_int_val((int)Dancer ::getInstance().getSequenceMode());
}

void setStatusWidget() {
	static string s;
	s = getStatus();
	statusTextWidget->set_text(s.c_str());
}

void currentSequenceModeCallback(int widgetNo) {
	Dancer::getInstance().setSequenceMode((Dancer::SequenceModeType)currentSequenceModeLiveVar);
	BotClient::getInstance().setMoveMode((Dancer::SequenceModeType)currentSequenceModeLiveVar);
}

void clothesOnCallback(int widgetNo) {
	WindowController::getInstance().mainBotView.getBotRenderer().setClothingMode((BotRenderer::ClothingModeType)clothesOnLiveVar);
}

void ambitionCallback(int widgetNo) {
	BotClient::getInstance().setAmbition(((float)ambitionLiveVar)/100.0);
	Dancer::getInstance().setAmbition(((float)ambitionLiveVar)/100.0);
}

void fileCallback(int widgetNo) {
	std::ostringstream statusStr;
	string filename = fileBrowser->get_file();
	 if (fileExists(filename)) {
		 if (endsWith(filename, ".wav")) {
			 string fileContent = readFileContent(filename);
			 string name = filename;
			 // std::replace( name.begin(), name.end(), ' ', '_');
			 name = name.substr(0,name.length()-4);

			new std::thread([=](){
				 WindowController::getInstance().setStatus("file being uploaded");

				 BotClient::getInstance().setWavFile(name, fileContent);
				 WindowController::getInstance().setStatus("file sent.");
			});
		 }
		 else {
			 statusStr << "file " << filename << " is no wav file" << endl;
			 WindowController::getInstance().setStatus(statusStr.str());
		 }
	 }
	 else {
		 statusStr << "file " << filename << " not found" << endl;
		 WindowController::getInstance().setStatus(statusStr.str());
	 }
}

void noCallback (int widgetNo) {};

GLUI* WindowController::createInteractiveWindow(int mainWindow) {
	GLUI *windowHandle= GLUI_Master.create_glui_subwindow( wMain,  GLUI_SUBWINDOW_BOTTOM);
	windowHandle->set_main_gfx_window( wMain );
	GLUI_StaticText* text = NULL;

	GLUI_Panel* interactivePanel = new GLUI_Panel(windowHandle,"interactive panel", GLUI_PANEL_NONE);
	GLUI_Panel* movePanel = new GLUI_Panel(interactivePanel,"Move Panel", GLUI_PANEL_RAISED);
	text = new GLUI_StaticText(movePanel, "MOVE CONSOLE");
	GLUI_Panel* moveRowsPanel = new GLUI_Panel(movePanel,"Move Panel", GLUI_PANEL_NONE);

	GLUI_Panel* dancingModePanel[DanceMoveRows];
	int moveCounter = 0;
	for (int row = 0;row < DanceMoveRows; row++) {
		dancingModePanel[row] = new GLUI_Panel(moveRowsPanel,"Dancing Mode Panel", GLUI_PANEL_NONE);

		currentDancingModeWidget[row] =  new GLUI_RadioGroup(dancingModePanel[row], dancingModeLiveVar + row, row, currentDancingMoveCallback);

		while (moveCounter < (Dancer ::getInstance().getNumMoves() +1) / DanceMoveRows * (row+1)) {
			Move& move = Move::getMove((Move::MoveType)moveCounter);
			new GLUI_RadioButton(currentDancingModeWidget[row], move.getName().c_str());

			moveCounter++;
		}

		// fill up with empty lines to have the containers of the same height
		if (row == DanceMoveRows-1) {
			for (int lines = moveCounter;lines < Dancer ::getInstance().getNumMoves()-Dancer::getInstance().getNumMoves()%2;lines++)
				new GLUI_StaticText(dancingModePanel[row],"");
		}

		// add a column for next row
		if (row < DanceMoveRows) {
			windowHandle->add_column_to_panel(moveRowsPanel, false);
		}

	}
	windowHandle->add_column_to_panel(interactivePanel, false);

	GLUI_Panel* subInteractivePanel = new GLUI_Panel(interactivePanel,"mode panel", GLUI_PANEL_RAISED);
	GLUI_Panel* interactiveModePanel = new GLUI_Panel(subInteractivePanel,"mode panel", GLUI_PANEL_NONE);

	interactiveModePanel->set_alignment(GLUI_ALIGN_LEFT);
	text = new GLUI_StaticText(interactiveModePanel, "MODE CONSOLE");
	text->set_alignment(GLUI_ALIGN_LEFT);

	GLUI_Panel* sequenceModePanel= new GLUI_Panel(interactiveModePanel,"Sequence Mode", GLUI_PANEL_NONE);
	sequenceModePanel->set_alignment(GLUI_ALIGN_LEFT);

	currentSequenceModeWidget =  new GLUI_RadioGroup(sequenceModePanel, &currentSequenceModeLiveVar, 0, currentSequenceModeCallback);
	new GLUI_RadioButton(currentSequenceModeWidget, "Automated Sequence");
	new GLUI_RadioButton(currentSequenceModeWidget, "Selected Dance Move");

	GLUI_Panel* clothesPanel= new GLUI_Panel(interactiveModePanel,"Attitude", GLUI_PANEL_NONE);
	clothesPanel->set_alignment(GLUI_ALIGN_LEFT);
	stripperRadioButtons =  new GLUI_RadioGroup(clothesPanel, &clothesOnLiveVar, 0, clothesOnCallback);
	new GLUI_RadioButton(stripperRadioButtons, "proper");
	new GLUI_RadioButton(stripperRadioButtons, "slutty");
	new GLUI_RadioButton(stripperRadioButtons, "terminator style");

	GLUI_Panel* ambitionPanel= new GLUI_Panel(interactiveModePanel,"Ambition", GLUI_PANEL_NONE);
	ambitionPanel->set_alignment(GLUI_ALIGN_LEFT);
	text = new GLUI_StaticText(ambitionPanel, "bored            ambitious");
	text->set_alignment(GLUI_ALIGN_LEFT);

	ambitionSpinner = new GLUI_Scrollbar(ambitionPanel, "ambition",1, &ambitionLiveVar, 1, ambitionCallback);
	ambitionSpinner->set_alignment(GLUI_ALIGN_LEFT);
	ambitionSpinner->set_int_limits(0,100);
	ambitionSpinner->set_int_val(100);

	musicDetectedBox = new GLUI_Checkbox(interactiveModePanel, "music detected", &musicDetectedLiveVar, 1, noCallback);
	musicDetectedBox->enable();
	musicDetectedBox->deactivate();

	text = new GLUI_StaticText(interactiveModePanel, "");

	statusTextWidget = new GLUI_StaticText(interactiveModePanel, "");

	windowHandle->add_column_to_panel(interactivePanel, false);
	GLUI_Panel* songPanel = new GLUI_Panel(interactivePanel,"song panel", GLUI_PANEL_RAISED);
	songPanel->set_alignment(GLUI_ALIGN_LEFT);
	text = new GLUI_StaticText(songPanel, "SONG CONSOLE");
	text->set_alignment(GLUI_ALIGN_LEFT);

	GLUI_Panel* filePanel = new GLUI_Panel(songPanel,"song panel", GLUI_PANEL_NONE);
	filePanel->set_alignment(GLUI_ALIGN_LEFT);

	fileBrowser = new GLUI_FileBrowser(filePanel, "select a .wav song", false, 0, fileCallback);
	fileBrowser->set_alignment(GLUI_ALIGN_LEFT);
	fileBrowser->set_h(170);
	fileBrowser->set_w(100);

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
	const int refreshRate = 50; // [Hz]
	const int idleRefreshRate = 1; // [Hz]

	const milliseconds refreshRate_ms = 1000/refreshRate/2;
	const milliseconds idleRefreshRate_ms = 1000/idleRefreshRate/2;

	milliseconds now = millis();
	static milliseconds lastDisplayRefreshCall = millis();

	// update all screens with 25Hz once a second in if new data is there
	bool refresh = ((WindowController::getInstance().mainBotView.isModified() && (now - lastDisplayRefreshCall > refreshRate_ms)) ||
					(now - lastDisplayRefreshCall > idleRefreshRate_ms));
	if (refresh) {
		cout << "refresh" << endl;
		WindowController::getInstance().mainBotView.postRedisplay(); // post if we reset the flag in a previous run
		lastDisplayRefreshCall = now;

		setDancingMoveWidget();
		setMusicDetectedWidget();
		setStatusWidget();
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

