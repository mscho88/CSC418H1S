/***********************************************************
             CSC418, FALL 2009
 
                 robot.cpp
                 author: Mike Pratscher
                 based on code by: Eron Steger, J. Radulovich

		Main source file for assignment 2
		Uses OpenGL, GLUT and GLUI libraries
  
    Instructions:
        Please read the assignment page to determine 
        exactly what needs to be implemented.  Then read 
        over this file and become acquainted with its 
        design. In particular, see lines marked 'README'.
		
		Be sure to also look over keyframe.h and vector.h.
		While no changes are necessary to these files, looking
		them over will allow you to better understand their
		functionality and capabilites.

        Add source code where it appears appropriate. In
        particular, see lines marked 'TODO'.

        You should not need to change the overall structure
        of the program. However it should be clear what
        your changes do, and you should use sufficient comments
        to explain your code.  While the point of the assignment
        is to draw and animate the character, you will
        also be marked based on your design.

***********************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "keyframe.h"
#include "timer.h"
#include "vector.h"
#include "explosion.h"
#include "material.h"
//#include "missile.h"
 

// *************** GLOBAL VARIABLES *************************


const float PI = 3.14159;

const float SPINNER_SPEED = 0.1;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;				// Glut window ID (for display)
int Win[2];					// window (x,y) size

GLUI* glui_joints;			// Glui window with joint controls
GLUI* glui_keyframe;		// Glui window with keyframe controls
GLUI* glui_render;			// Glui window for render style

char msg[256];				// String used for status message
GLUI_StaticText* status;	// Status message ("Status: <msg>")


// ---------------- ANIMATION VARIABLES ---------------------

// Camera settings
bool updateCamZPos = false;
int  lastX = 0;
int  lastY = 0;
const float ZOOM_SCALE = 0.01;

GLdouble camXPos =  0.0;
GLdouble camYPos =  0.0;
GLdouble camZPos = -7.5;

GLdouble camXRot = 0.0;
GLdouble camYRot = 0.0;

const GLdouble CAMERA_FOVY = 60.0;
const GLdouble NEAR_CLIP   = 0.1;
const GLdouble FAR_CLIP    = 1000.0;

// Light settings
GLdouble Lx = 0.0;
GLdouble Ly = 3.0;
GLdouble Lz = 2.0;

// Render settings
enum { WIREFRAME, METALIC};	// README: the different render styles
int renderStyle = METALIC;	// README: the selected render style

// Animation settings
int animate_mode = 0;			// 0 = no anim, 1 = animate

// Keyframe settings
const char filenameKF[] = "keyframes.txt";	// file for loading / saving keyframes

Keyframe* keyframes;			// list of keyframes

int maxValidKeyframe   = 0;		// index of max VALID keyframe (in keyframe list)
const int KEYFRAME_MIN = 0;
const int KEYFRAME_MAX = 32;	// README: specifies the max number of keyframes

// Frame settings
char filenameF[128];			// storage for frame filename

const float DUMP_FRAME_PER_SEC = 24.0;		// frame rate for dumped frames
const float DUMP_SEC_PER_FRAME = 1.0 / DUMP_FRAME_PER_SEC;

// Time settings
Timer* animationTimer;
Timer* frameRateTimer;

const float TIME_MIN = 0.0;
const float TIME_MAX = 20.0;	// README: specifies the max time of the animation
const float SEC_PER_FRAME = 1.0 / 60.0;

// Joint settings

// README: This is the key data structure for
// updating keyframes in the keyframe list and
// for driving the animation.
//   i) When updating a keyframe, use the values
//      in this data structure to update the
//      appropriate keyframe in the keyframe list.
//  ii) When calculating the interpolated pose,
//      the resulting pose vector is placed into
//      this data structure. (This code is already
//      in place - see the animate() function)
// iii) When drawing the scene, use the values in
//      this data structure (which are set in the
//      animate() function as described above) to
//      specify the appropriate transformations.
Keyframe* joint_ui_data;

// README: To change the range of a particular DOF,
// simply change the appropriate min/max values below
const float ROOT_TRANSLATE_X_MIN = -50.0;
const float ROOT_TRANSLATE_X_MAX =  50.0;
const float ROOT_TRANSLATE_Y_MIN = -50.0;
const float ROOT_TRANSLATE_Y_MAX =  50.0;
const float ROOT_TRANSLATE_Z_MIN = -50.0;
const float ROOT_TRANSLATE_Z_MAX =  50.0;
const float ROOT_ROTATE_X_MIN    = -360.0;
const float ROOT_ROTATE_X_MAX    =  360.0;
const float ROOT_ROTATE_Y_MIN    = -360.0;
const float ROOT_ROTATE_Y_MAX    =  360.0;
const float ROOT_ROTATE_Z_MIN    = -360.0;
const float ROOT_ROTATE_Z_MAX    =  360.0;
const float TORSO_MIN            = -180.0;
const float TORSO_MAX            = 180.0;
const float SHOULDER_MIN         = -45.0;
const float SHOULDER_MAX         =  45.0;
const float WEAPON_MIN           = -180.0;
const float WEAPON_MAX           = 180.0;
const float PELVIS_MIN           = -60.0;
const float PELVIS_MAX           = 30.0;
const float KNEE_X_MIN           = -90.0;
const float KNEE_X_MAX           = 90.0;
const float KNEE_Y_MIN           = -90.0;
const float KNEE_Y_MAX           = 90.0;
const float KNEE_Z_MIN           = -90.0;
const float KNEE_Z_MAX           = 90.0;

GLUquadricObj *quadratic = gluNewQuadric();

// ***********  FUNCTION HEADER DECLARATIONS ****************

// Initialization functions
void initDS();
void initGlut(int argc, char** argv);
void initGlui();
void initGl();

// Callbacks for handling events in glut
void reshape(int w, int h);
void animate();
void display(void);
void keyboard(unsigned char key, int x, int y);
void arrowKeyboard(int i, int x, int y);
void mouse(int button, int state, int x, int y);

void motion(int x, int y);
void lighting(void);
void materialRendering();

void drawWorld();

void drawGoliath();
void drawLeftArm();
void drawRightArm();
void drawPelvis();
void drawLeftLeg();
void drawRightLeg();

// Functions to help draw the object
Vector getInterpolatedJointDOFS(float time);
void drawCube();

// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: demo [width] [height]\n");
        printf("Using 640x480 window by default...\n");
        Win[0] = 640;
        Win[1] = 480;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }
	
    // Initialize data structs, glut, glui, and opengl
	initDS();
    initGlut(argc, argv);
    initGlui();
    initGl();

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(arrowKeyboard);
	// Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Create / initialize global data structures
void initDS()
{
	keyframes = new Keyframe[KEYFRAME_MAX];
	for( int i = 0; i < KEYFRAME_MAX; i++ )
		keyframes[i].setID(i);

	animationTimer = new Timer();
	frameRateTimer = new Timer();
	joint_ui_data  = new Keyframe();
}


// Initialize glut and create a window with the specified caption 
void initGlut(int argc, char** argv)
{
	// Init GLUT
	glutInit(&argc, argv);

    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(argv[0]);

    // Setup callback functions to handle events
    glutReshapeFunc(reshape);	// Call reshape whenever window resized
    glutDisplayFunc(display);	// Call display whenever new frame needed
	glutMouseFunc(mouse);		// Call mouse whenever mouse button pressed
	glutMotionFunc(motion);		// Call motion whenever mouse moves while button pressed
}

  
// Load Keyframe button handler. Called when the "load keyframe" button is pressed
void loadKeyframeButton(int)
{
	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();

	// Update the 'joint_ui_data' variable with the appropriate
	// entry from the 'keyframes' array (the list of keyframes)
	*joint_ui_data = keyframes[keyframeID];

	// Sync the UI with the 'joint_ui_data' values
	glui_joints->sync_live();
	glui_keyframe->sync_live();

	// Let the user know the values have been loaded
	sprintf(msg, "Status: Keyframe %d loaded successfully", keyframeID);
	status->set_text(msg);
}

// Load Keyframes From File button handler. Called when the "load keyframes from file" button is pressed
//
// ASSUMES THAT THE FILE FORMAT IS CORRECT, ie, there is no error checking!
//
void loadKeyframesFromFileButton(int)
{
	// Open file for reading
	FILE* file = fopen(filenameKF, "r");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Read in maxValidKeyframe first
	fscanf(file, "%d", &maxValidKeyframe);

	// Now read in all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fscanf(file, "%d", keyframes[i].getIDPtr());
		fscanf(file, "%f", keyframes[i].getTimePtr());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fscanf(file, "%f", keyframes[i].getDOFPtr(j));
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been loaded
	sprintf(msg, "Status: Keyframes loaded successfully");
	status->set_text(msg);
}

// Animate button handler.  Called when the "animate" button is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui_keyframe->sync_live();

  // toggle animation mode and set idle function appropriately
  if( animate_mode == 0 )
  {
	// start animation
	frameRateTimer->reset();
	animationTimer->reset();

	animate_mode = 1;
	GLUI_Master.set_glutIdleFunc(animate);

	// Let the user know the animation is running
	sprintf(msg, "Status: Animating...");
	status->set_text(msg);
  }
  else
  {
	// stop animation
	animate_mode = 0;
	GLUI_Master.set_glutIdleFunc(NULL);

	// Let the user know the animation has stopped
	sprintf(msg, "Status: Animation stopped");
	status->set_text(msg);
  }
}
 

// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}


// Initialize GLUI and the user interface
void initGlui()
{
	GLUI_Panel* glui_panel;
	GLUI_Spinner* glui_spinner;
	GLUI_RadioGroup* glui_radio_group;

    GLUI_Master.set_glutIdleFunc(NULL);


	// Create GLUI window (joint controls) ***************
	//
	glui_joints = GLUI_Master.create_glui("Joint Control", 0, Win[0]+12, 0);

    // Create controls to specify root position and orientation
	glui_panel = glui_joints->add_panel("Root");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_X));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_X_MIN, ROOT_TRANSLATE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Y));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Y_MIN, ROOT_TRANSLATE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_TRANSLATE_Z));
	glui_spinner->set_float_limits(ROOT_TRANSLATE_Z_MIN, ROOT_TRANSLATE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_X));
	glui_spinner->set_float_limits(ROOT_ROTATE_X_MIN, ROOT_ROTATE_X_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Y));
	glui_spinner->set_float_limits(ROOT_ROTATE_Y_MIN, ROOT_ROTATE_Y_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::ROOT_ROTATE_Z));
	glui_spinner->set_float_limits(ROOT_ROTATE_Z_MIN, ROOT_ROTATE_Z_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify head rotation
	glui_panel = glui_joints->add_panel("TORSO");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "TORSO:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::TORSO));
	glui_spinner->set_float_limits(TORSO_MIN, TORSO_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
	
	glui_joints->add_column(false);


	// Create controls to specify right arm
	glui_panel = glui_joints->add_panel("RIGHT shoulder");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "right shoulder:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER));
	glui_spinner->set_float_limits(SHOULDER_MIN, SHOULDER_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "right weapon:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_WEAPON));
	glui_spinner->set_float_limits(WEAPON_MIN, WEAPON_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	// Create controls to specify left arm
	glui_panel = glui_joints->add_panel("LEFT shoulder");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "left shoulder:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER));
	glui_spinner->set_float_limits(SHOULDER_MIN, SHOULDER_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "left weapon:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_WEAPON));
	glui_spinner->set_float_limits(WEAPON_MIN, WEAPON_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_joints->add_column(false);


	// Create controls to specify right leg
	glui_panel = glui_joints->add_panel("Right pelvis");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "right pelvis:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_PELVIS));
	glui_spinner->set_float_limits(PELVIS_MIN, PELVIS_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
	
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "right leg x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_KNEE_X));
	glui_spinner->set_float_limits(KNEE_X_MIN, KNEE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
	
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "right leg y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_KNEE_Y));
	glui_spinner->set_float_limits(KNEE_Y_MIN, KNEE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
	
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "right left z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_KNEE_Z));
	glui_spinner->set_float_limits(KNEE_Z_MIN, KNEE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	// Create controls to specify left leg
	glui_panel = glui_joints->add_panel("Left pelvis");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "left pelvis:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_PELVIS));
	glui_spinner->set_float_limits(PELVIS_MIN, PELVIS_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
	
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "left pelvis:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_KNEE_X));
	glui_spinner->set_float_limits(KNEE_X_MIN, KNEE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
	
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "left pelvis:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_KNEE_Y));
	glui_spinner->set_float_limits(KNEE_Y_MIN, KNEE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "left pelvis:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_KNEE_Z));
	glui_spinner->set_float_limits(KNEE_Z_MIN, KNEE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	///////////////////////////////////////////////////////////
	// TODO (for controlling light source position & additional
	//      rendering styles):
	//   Add more UI spinner elements here. Be sure to also
	//   add the appropriate min/max range values to this
	//   file, and to also add the appropriate enums to the
	//   enumeration in the Keyframe class (keyframe.h).
	///////////////////////////////////////////////////////////

	//
	// ***************************************************


	// Create GLUI window (keyframe controls) ************
	//
	glui_keyframe = GLUI_Master.create_glui("Keyframe Control", 0, 0, Win[1]+64);

	// Create a control to specify the time (for setting a keyframe)
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Time:", GLUI_SPINNER_FLOAT, joint_ui_data->getTimePtr());
	glui_spinner->set_float_limits(TIME_MIN, TIME_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
	
	/*
	// Create a control to specify a keyframe (for updating / loading a keyframe)
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Keyframe ID:", GLUI_SPINNER_INT, joint_ui_data->getIDPtr());
	glui_spinner->set_int_limits(KEYFRAME_MIN, KEYFRAME_MAX-1, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
*/
	glui_keyframe->add_separator();

	// Add buttons to load and update keyframes
	// Add buttons to load and save keyframes from a file
	// Add buttons to start / stop animation and to render frames to file
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframe", 0, loadKeyframeButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Start / Stop Animation", 0, animateButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframes From File", 0, loadKeyframesFromFileButton);
	glui_keyframe->add_column_to_panel(glui_panel, false);
	//glui_keyframe->add_button_to_panel(glui_panel, "Update Keyframe", 0, updateKeyframeButton);
	//glui_keyframe->add_button_to_panel(glui_panel, "Save Keyframes To File", 0, saveKeyframesToFileButton);
	//glui_keyframe->add_button_to_panel(glui_panel, "Render Frames To File", 0, renderFramesToFileButton);

	glui_keyframe->add_separator();

	// Add status line
	glui_panel = glui_keyframe->add_panel("");
	status = glui_keyframe->add_statictext_to_panel(glui_panel, "Status: Ready");

	// Add button to quit
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Quit", 0, quitButton);
	//
	// ***************************************************


	// Create GLUI window (render controls) ************
	//
	glui_render = GLUI_Master.create_glui("Render Control", 0, 367, Win[1]+64);

	// Create control to specify the render style
	glui_panel = glui_render->add_panel("Render Style");
	glui_radio_group = glui_render->add_radiogroup_to_panel(glui_panel, &renderStyle);
	glui_render->add_radiobutton_to_group(glui_radio_group, "Wireframe");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Metalic");
	//
	// ***************************************************


	// Tell GLUI windows which window is main graphics window
	glui_joints->set_main_gfx_window(windowID);
	glui_keyframe->set_main_gfx_window(windowID);
	glui_render->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,1.0f);
}


// Calculates the interpolated joint DOF vector
// using Catmull-Rom interpolation of the keyframes
Vector getInterpolatedJointDOFS(float time)
{
	// Need to find the keyframes bewteen which
	// the supplied time lies.
	// At the end of the loop we have:
	//    keyframes[i-1].getTime() < time <= keyframes[i].getTime()
	//
	int i = 0;
	while( i <= maxValidKeyframe && keyframes[i].getTime() < time )
		i++;

	// If time is before or at first defined keyframe, then
	// just use first keyframe pose
	if( i == 0 )
		return keyframes[0].getDOFVector();

	// If time is beyond last defined keyframe, then just
	// use last keyframe pose
	if( i > maxValidKeyframe )
		return keyframes[maxValidKeyframe].getDOFVector();

	// Need to normalize time to (0, 1]
	time = (time - keyframes[i-1].getTime()) / (keyframes[i].getTime() - keyframes[i-1].getTime());

	// Get appropriate data points and tangent vectors
	// for computing the interpolation
	Vector p0 = keyframes[i-1].getDOFVector();
	Vector p1 = keyframes[i].getDOFVector();

	Vector t0, t1;
	if( i == 1 )							// special case - at beginning of spline
	{
		t0 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}
	else if( i == maxValidKeyframe )		// special case - at end of spline
	{
		t0 = (keyframes[i].getDOFVector() - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
	}
	else
	{
		t0 = (keyframes[i].getDOFVector()   - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}

	// Return the interpolated Vector
	Vector a0 = p0;
	Vector a1 = t0;
	Vector a2 = p0 * (-3) + p1 * 3 + t0 * (-2) + t1 * (-1);
	Vector a3 = p0 * 2 + p1 * (-2) + t0 + t1;

	return (((a3 * time + a2) * time + a1) * time + a0);
}


// Callback idle function for animating the scene
void animate()
{
	// Only update if enough time has passed
	// (This locks the display to a certain frame rate rather
	//  than updating as fast as possible. The effect is that
	//  the animation should run at about the same rate
	//  whether being run on a fast machine or slow machine)
	if( frameRateTimer->elapsed() > SEC_PER_FRAME )
	{
		// Tell glut window to update itself. This will cause the display()
		// callback to be called, which renders the object (once you've written
		// the callback).
		glutSetWindow(windowID);
		glutPostRedisplay();

		// Restart the frame rate timer
		// for the next frame
		frameRateTimer->reset();
	}
}


// Handles the window being resized by updating the viewport
// and projection matrices
void reshape(int w, int h)
{
	// Update internal variables and OpenGL viewport
	Win[0] = w;
	Win[1] = h;
	glViewport(0, 0, (GLsizei)Win[0], (GLsizei)Win[1]);

    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(CAMERA_FOVY, (GLdouble)Win[0]/(GLdouble)Win[1], NEAR_CLIP, FAR_CLIP);
}


void lighting(void){
	
	if (renderStyle == METALIC) {
		
		
		float light_x = joint_ui_data->getDOF(Keyframe::LIGHT_X) + Lx;
		float light_y = joint_ui_data->getDOF(Keyframe::LIGHT_Y) + Ly;
		float light_z = joint_ui_data->getDOF(Keyframe::LIGHT_Z) + Lz;
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);			
		glDisable(GL_LIGHTING);
		glPushMatrix();			
			glColor3f(1.0, 1.0, 0.0);
			glTranslatef(light_x, light_y, light_z - 5.0);
			glScalef(1.0, 1.0, 1.0);
			gluSphere(quadratic, 0.3f, 16, 16);
		glPopMatrix();
		
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_NORMALIZE);
		glDepthFunc(GL_LESS);
		
		const GLfloat ambient[4] = { 0.3, 0.3, 0.3, 1.0 };
		const GLfloat diffuse[4] = { 0.8, 0.8, 0.8, 1.0 };
		const GLfloat specular[4] = { 0.8, 0.8, 0.8, 1.0 };
		GLfloat light[4] = {light_x, light_y, light_z - 5.0, 0};
		
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
		glLightfv(GL_LIGHT0, GL_POSITION, light);
		
	} else {
		glDisable(GL_LIGHTING);
	}
}

// display callback
//
// README: This gets called by the event handler
// to draw the scene, so this is where you need
// to build your scene -- make your changes and
// additions here. All rendering happens in this
// function. For Assignment 2, updates to the
// joint DOFs (joint_ui_data) happen in the
// animate() function.
void display(void)
{
    // Clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// Specify camera transformation
	glTranslatef(camXPos, camYPos, camZPos);
	
	// Get the time for the current animation step, if necessary
	if( animate_mode )
	{
		float curTime = animationTimer->elapsed();

		if( curTime >= keyframes[maxValidKeyframe].getTime() )
		{
			// Restart the animation
			animationTimer->reset();
			curTime = animationTimer->elapsed();
		}

		///////////////////////////////////////////////////////////
		// README:
		//   This statement loads the interpolated joint DOF vector
		//   into the global 'joint_ui_data' variable. Use the
		//   'joint_ui_data' variable below in your model code to
		//   drive the model for animation.
		///////////////////////////////////////////////////////////
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(curTime) );

		// Update user interface
		joint_ui_data->setTime(curTime);
		glui_keyframe->sync_live();
		glui_joints->sync_live();
	}


    ///////////////////////////////////////////////////////////
    // TODO:
	//   Modify this function to draw the scene.
	//   This should include function calls that apply
	//   the appropriate transformation matrices and render
	//   the individual body parts.
	//   Use the 'joint_ui_data' data structure to obtain
	//   the joint DOFs to specify your transformations.
	//   Sample code is provided below and demonstrates how
	//   to access the joint DOF values. This sample code
	//   should be replaced with your own.
	//   Use the 'renderStyle' variable and the associated
	//   enumeration to determine how the geometry should be
	//   rendered.
    ///////////////////////////////////////////////////////////

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	
	// The robot is actually drawn here ********************************
	//
	glPushMatrix();

		// setup transformation for body part
		glTranslatef(joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_X),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Y),
					 joint_ui_data->getDOF(Keyframe::ROOT_TRANSLATE_Z));
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_X) + camXRot, 1.0, 0.0, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Y) + camYRot, 0.0, 1.0, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::ROOT_ROTATE_Z), 0.0, 0.0, 1.0);
		
		if(renderStyle == WIREFRAME){
			// WIREFRAME rendering
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);			
			glPushMatrix();
				glTranslatef(0.0, -5.5, 60.0);
				drawWorld();
			glPopMatrix();
			drawGoliath();
		}else if(renderStyle == METALIC){
			lighting();
			// Metalic rendering
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
			glMaterialfv(GL_FRONT, GL_AMBIENT, bronze[0]);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, bronze[1]);
			glMaterialfv(GL_FRONT, GL_SPECULAR, bronze[2]);
			glMaterialf(GL_FRONT, GL_SHININESS, bronzeShine * 128.0);
			
			glPushMatrix();
				glTranslatef(0.0, -5.5, 60.0);
				drawWorld();
			glPopMatrix();
			
			glMaterialfv(GL_FRONT, GL_AMBIENT, metal[0]);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, metal[1]);
			glMaterialfv(GL_FRONT, GL_SPECULAR, metal[2]);
			glMaterialf(GL_FRONT, GL_SHININESS, metalShine * 128.0);
			drawGoliath();
		}
	glPopMatrix();
	 
	//
	// drawing CODE ********** 

    // Execute any GL functions that are in the queue just to be safe
    glFlush();

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers(); 
}

void drawWorld(){
	// Draw the floor
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-80.0, 0.0, 80.0);
		glTexCoord2f(0.0, 16.0);
		glVertex3f(80.0, 0.0, 80.0);
		glTexCoord2f(16.0, 16.0);
		glVertex3f(80.0, 0.0, -80.0);
		glTexCoord2f(16.0, 0.0);
		glVertex3f(-80.0, 0.0, -80.0);
	glEnd();

	// Draw a cube which will be exploded after 12 seconds
	if(joint_ui_data->getTime() > 12){ 
		displayExplosion(); 
	}else{
		glPushMatrix();
			glColor3f(0.0, 0.0, 0.0);
			glTranslatef(15.0, 5.0, 0.0);
			glScalef(4.0, 5.0, 10.0);
			drawCube();
		glPopMatrix();
		newExplosion();
	}
	
}

void drawGoliath(){	
	glPushMatrix();
		glRotatef(joint_ui_data->getDOF(Keyframe::TORSO), 0.0, 1.0, 0.0);
		glPushMatrix();
			// draw body part
			glColor3f(0.5, 0.6, 0.7);
			glScalef(0.8, 1.0, 0.8);
			drawCube();
		glPopMatrix();
		
		drawLeftArm();
		drawRightArm();
		
	glPopMatrix();
	
	drawPelvis();
}

void materialRendering(){
	glMaterialfv(GL_FRONT, GL_AMBIENT, metal[0]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, metal[1]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, metal[2]);
	glMaterialf(GL_FRONT, GL_SHININESS, metalShine * 128.0);
}

void drawLeftArm(){
	glPushMatrix();
		glTranslatef(0.8, 0.5, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER), 1.0, 0.0, 0.0);
		
		// left arm 
		glPushMatrix();
			glColor3f(0.0, 1.0, 0.0);
			glScalef(1.0, 0.7, 1.0);
			glTranslatef(1.0, 0.0, 0.0);
			drawCube();
			
			// Drawing missiles
			glPushMatrix();
				glMaterialfv(GL_FRONT, GL_AMBIENT, ruby[0]);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, ruby[1]);
				glMaterialfv(GL_FRONT, GL_SPECULAR, ruby[2]);
				glMaterialf(GL_FRONT, GL_SHININESS, rubyShine * 128.0);
				glTranslatef(0.0, 0.0, 1.0);
				
				for(int i = 0; i < 2; i++){
					for(int j = 0; j < 3; j++){
						glPushMatrix();
							if(j == 0){
								glTranslatef(0.6 * 0 + joint_ui_data->getDOF(Keyframe::MISSILE_X),
								 0.4 * (i % 2 == 0 ? (-1) : (1)) + joint_ui_data->getDOF(Keyframe::MISSILE_Y),
								 0.0 + joint_ui_data->getDOF(Keyframe::MISSILE_Z));
								 
								 // Remove the launched missiles after 12 seconds due to explosion of the cube
								if(joint_ui_data->getTime() < 12){
									glutWireCone(0.2, 0.2, 16, 16);
									glTranslatef(0.0, 0.0, -0.95);
									gluCylinder(quadratic, 0.2, 0.2, 1.0f, 32, 32);
									newExplosion();
								}
							}else{
								glTranslatef(0.6 * (j % 3 == 1? (-1) : (1)), 0.4 * (i % 2 == 0 ? (-1) : (1)), 0.0);
								glutWireCone(0.2, 0.2, 16, 16);
								glTranslatef(0.0, 0.0, -0.95);
								gluCylinder(quadratic, 0.2, 0.2, 1.0f, 32, 32);
							}
							
						glPopMatrix();
					}
				}
				materialRendering();
			glPopMatrix();
		glPopMatrix();
		
		glPushMatrix();
			glTranslatef(1.5, -1.0, 0.0);
			glColor3f(0.0, 1.0, 0.0);
			glScalef(0.3, 0.3, 0.3);
			drawCube();
			
			// Draw Machinegun
			glPushMatrix();
				glTranslatef(0.0, 0.0, 1.0);
				glRotatef(joint_ui_data->getDOF(Keyframe::L_WEAPON), 0.0, 0.0, 1.0);
				gluCylinder(quadratic, 0.7f, 0.7f, 1.0f, 32, 32);
				glTranslatef(0.0, 0.0, 1.0);
				gluDisk(quadratic, 0.0, 0.7, 16, 10);
				for(int i = 0; i < 2; i++){
					for(int j = 0; j < 2; j++){
						glPushMatrix();
							glTranslatef(j % 2 == 0? (0) : (0.3 * (i % 2 == 0? (-1) : (1))), j % 2 == 0? (0.3 * (i % 2 == 0? (-1) : (1))) : (0), 0.0);
							gluCylinder(quadratic, 0.2f, 0.2f, 4.0f, 32, 32);
							gluCylinder(quadratic, 0.15f, 0.15f, 4.2f, 32, 32);
						glPopMatrix();
					}
				}
			glPopMatrix();
		glPopMatrix();
	glPopMatrix();
}

void drawRightArm(){
	glPushMatrix();
		glTranslatef(-0.8, 0.5, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER), 1.0, 0.0, 0.0);
		
		// right arm 
		glPushMatrix();
			glColor3f(0.0, 1.0, 0.0);
			glScalef(1.0, 0.7, 1.0);
			glTranslatef(-1.0, 0.0, 0.0);
			drawCube();

			// Drawing missiles
			glPushMatrix();
				// Sets the color of the missiles as ruby.
				glMaterialfv(GL_FRONT, GL_AMBIENT, ruby[0]);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, ruby[1]);
				glMaterialfv(GL_FRONT, GL_SPECULAR, ruby[2]);
				glMaterialf(GL_FRONT, GL_SHININESS, rubyShine * 128.0);
				glTranslatef(0.0, 0.0, 1.0);
				
				for(int i = 0; i < 2; i++){
					for(int j = 0; j < 3; j++){
						glPushMatrix();
							if(j == 0){
								// Movement of the missiles
								glTranslatef(0.6 * 0 + joint_ui_data->getDOF(Keyframe::MISSILE_X),
								 0.4 * (i % 2 == 0 ? (-1) : (1)) + joint_ui_data->getDOF(Keyframe::MISSILE_Y),
								 0.0 + joint_ui_data->getDOF(Keyframe::MISSILE_Z));
								
								// Remove the launched missiles after 12 seconds due to explosion of the cube
								if(joint_ui_data->getTime() < 12){
									glutWireCone(0.2, 0.2, 16, 16);
									glTranslatef(0.0, 0.0, -0.95);
									gluCylinder(quadratic, 0.2, 0.2, 1.0f, 32, 32);
								}
							}else{
								// Missiles that will not be launched
								glTranslatef(0.6 * (j % 3 == 1? (-1) : (1)), 0.4 * (i % 2 == 0 ? (-1) : (1)), 0.0);
								glutWireCone(0.2, 0.2, 16, 16);
								glTranslatef(0.0, 0.0, -0.95);
								gluCylinder(quadratic, 0.2, 0.2, 1.0f, 32, 32);
							}
							
							
						glPopMatrix();
					}
				}
				
				// Sets back the color of the robot
				glMaterialfv(GL_FRONT, GL_AMBIENT, metal[0]);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, metal[1]);
				glMaterialfv(GL_FRONT, GL_SPECULAR, metal[2]);
				glMaterialf(GL_FRONT, GL_SHININESS, metalShine * 128.0);
			glPopMatrix();
		glPopMatrix();
				
		// Draw right arm
		glPushMatrix();
			glTranslatef(-1.5, -1.0, 0.0);
			glColor3f(0.0, 1.0, 0.0);
			glScalef(0.3, 0.3, 0.3);
			drawCube();
			
			// Draw machinegun
			glPushMatrix();
				glTranslatef(0.0, 0.0, 1.0);
				glRotatef(joint_ui_data->getDOF(Keyframe::R_WEAPON), 0.0, 0.0, 1.0);
				gluCylinder(quadratic, 0.7f, 0.7f, 1.0f, 32, 32);
				glTranslatef(0.0, 0.0, 1.0);
				gluDisk(quadratic, 0.0, 0.7, 16, 10);
				for(int i = 0; i < 2; i++){
					for(int j = 0; j < 2; j++){
						glPushMatrix();
							glTranslatef(j % 2 == 0? (0) : (0.3 * (i % 2 == 0? (-1) : (1))), j % 2 == 0? (0.3 * (i % 2 == 0? (-1) : (1))) : (0), 0.0);
							gluCylinder(quadratic, 0.2f, 0.2f, 4.0f, 32, 32);
							gluCylinder(quadratic, 0.15f, 0.15f, 4.2f, 32, 32);
						glPopMatrix();
					}
				}
			glPopMatrix();
		glPopMatrix();
	glPopMatrix();
}

void drawPelvis(){
	glPushMatrix();
		// draw pelvis
		glPushMatrix();
			glColor3f(0.5, 0.5, 1.0);
			glTranslatef(0.0, -1.5, 0.0);
			glScalef(0.7, 0.5, 0.6);
			drawCube();
		glPopMatrix();
		
		drawLeftLeg();
		drawRightLeg();
		//drawLegs();
	glPopMatrix();
}

void drawLeftLeg(){
	glPushMatrix();
		glTranslatef(1.5, -1.6, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::L_PELVIS), 1.0, 0.0, 0.0);
		
		// draw left leg(upper)
		glRotatef(50.0f, 1.0f, 0.0f, 0.0f);
		glPushMatrix();
			glColor3f(1.0, 0.0, 0.5);
			glScalef(0.3, 0.8, 0.4);
			glTranslatef(0.8, -0.6, 0.0);
			drawCube();
		glPopMatrix();
		
		// draw left leg joint
		glPushMatrix();
			glColor3f(0.0, 0.0, 0.0);
			glTranslatef(-0.8, 0.0, 0.0);
			glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
			gluCylinder(quadratic, 0.3f, 0.3f, 1.4f, 32, 32);
		glPopMatrix();
		
		glPushMatrix();
			glTranslatef(0.0, -1.5, 0.0);
			
			// draw knee joint
			glPushMatrix();
				glColor3f(0.0, 0.0, 0.0);
				gluSphere(quadratic, 0.3f, 32, 32);
			glPopMatrix();
			
			// draw left leg(lower)
			glTranslatef(0.0, -0.5, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_KNEE_X), 1.0, 0.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_KNEE_Y), 0.0, 1.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::L_KNEE_Z), 0.0, 0.0, 1.0);
	
			glPushMatrix();
				glRotatef(-100.0f, 1.0f, 0.0f, 0.0f);
				glColor3f(1.0, 0.0, 0.5);
				glScalef(0.5, 1.5, 0.2);
				glTranslatef(0.0, -0.8, 0.0);
				drawCube();
			glPopMatrix();
				
			// draw foot
			glPushMatrix();
				glRotatef(-50.0f, 1.0, 0.0, 0.0);
				glTranslatef(0.0, -1.7, 2.2);
				glScalef(0.8, 0.3, 1.2);
				drawCube();
			glPopMatrix();
		glPopMatrix();
		
	glPopMatrix();
}
void drawRightLeg(){
	glPushMatrix();
		glTranslatef(-1.5, -1.6, 0.0);
		glRotatef(joint_ui_data->getDOF(Keyframe::R_PELVIS), 1.0, 0.0, 0.0);

		// draw right leg
		glRotatef(50.0f, 1.0f, 0.0f, 0.0f);
		glPushMatrix();
			glColor3f(1.0, 0.0, 0.5);
			glScalef(0.3, 0.8, 0.4);
			glTranslatef(-0.8, -0.6, 0.0);
			drawCube();
		glPopMatrix();
		
		// draw right leg joint
		glPushMatrix();
			glColor3f(0.0, 0.0, 0.0);
			glTranslatef(-0.6, 0.0, 0.0);
			glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
			gluCylinder(quadratic, 0.3f, 0.3f, 1.4f, 32, 32);
			//////////////////////////////////gluDisk(quadratic, 0.0, 0.3, 16, 10);
		glPopMatrix();
		
		glPushMatrix();
			glTranslatef(0.0, -1.5, 0.0);
			
			// draw knee joint
			glPushMatrix();
				glColor3f(0.0, 0.0, 0.0);
				gluSphere(quadratic, 0.3f, 32, 32);
			glPopMatrix();
			
			glTranslatef(0.0, -0.5, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_KNEE_X), 1.0, 0.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_KNEE_Y), 0.0, 1.0, 0.0);
			glRotatef(joint_ui_data->getDOF(Keyframe::R_KNEE_Z), 0.0, 0.0, 1.0);
		
			// draw right leg(lower)
			glPushMatrix();
				glRotatef(-100.0f, 1.0f, 0.0f, 0.0f);
				glColor3f(1.0, 0.0, 0.5);
				glScalef(0.5, 1.5, 0.2);
				glTranslatef(0.0, -0.8, 0.0);
				drawCube();
			glPopMatrix();
			
			// draw foot
			glPushMatrix();
				glRotatef(-50.0f, 1.0, 0.0, 0.0);
				glTranslatef(0.0, -1.7, 2.2);
				glScalef(0.8, 0.3, 1.2);
				drawCube();
			glPopMatrix();
		glPopMatrix();
	glPopMatrix();
}


/*
 * Handles the arrow key input on the keyboard from a user
 */
void arrowKeyboard(int key, int x, int y){
	
	switch (key){
		case GLUT_KEY_RIGHT:{
			camXPos++;
		}
		break;
		case GLUT_KEY_LEFT:{
			camXPos--;
		}
		break;
		case GLUT_KEY_UP:{
			camYPos++;
		}
		break;
		case GLUT_KEY_DOWN:{
			camYPos--;
		}
		break;
	}
}

/*
 * Handles the specific key input on the keyboard from a user
 */
void keyboard(unsigned char key, int x, int y){

	//int i = 0;

	switch (key) {
		case 'w':{
			//camXRot = camXRot + 10.0;
		}
		break;
		case 's':{
			//camXRot = camXRot - 10.0;
		}
		break;
		case 'a':{
			camYRot = camYRot + 10.0;
		}
		break;
		case 'd':{
			camYRot = camYRot - 10.0;
		}
		break;
		case 'r':{
			Lx++;
		}
		break;
		case 'f':{
			Lx--;
		}
		break;
		case 't':{
			Ly++;
		}
		break;
		case 'g':{
			Ly--;
		}
		break;
		case 'y':{
			Lz++;
		}
		break;
		case 'h':{
			Lz--;
		}
		break;
	}
}

// Handles mouse button pressed / released events
void mouse(int button, int state, int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( button == GLUT_RIGHT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			lastX = x;
			lastY = y;
			updateCamZPos = true;
		}
		else
		{
			updateCamZPos = false;
		}
	}
}


// Handles mouse motion events while a button is pressed
void motion(int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( updateCamZPos )
	{
		// Update camera z position
		camZPos += (x - lastX) * ZOOM_SCALE;
		lastX = x;

		// Redraw the scene from updated camera position
		glutSetWindow(windowID);
		glutPostRedisplay();
	}
}

// Draw a unit cube, centered at the current location
// README: Helper code for drawing a cube
void drawCube()
{
	glBegin(GL_QUADS);
		// draw front face
		// added normal vector of front face
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-1.0, -1.0, 1.0);
		glVertex3f( 1.0, -1.0, 1.0);
		glVertex3f( 1.0,  1.0, 1.0);
		glVertex3f(-1.0,  1.0, 1.0);
	
		// draw back face
		// added normal vector of back face
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f( 1.0, -1.0, -1.0);
		glVertex3f(-1.0, -1.0, -1.0);
		glVertex3f(-1.0,  1.0, -1.0);
		glVertex3f( 1.0,  1.0, -1.0);
	
		// draw left face
		// added normal vector of left face
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-1.0, -1.0, -1.0);
		glVertex3f(-1.0, -1.0,  1.0);
		glVertex3f(-1.0,  1.0,  1.0);
		glVertex3f(-1.0,  1.0, -1.0);
	
		// draw right face
		// added normal vector of right face
		glNormal3f(1.0, 0.0, 0.0);
		glVertex3f( 1.0, -1.0,  1.0);
		glVertex3f( 1.0, -1.0, -1.0);
		glVertex3f( 1.0,  1.0, -1.0);
		glVertex3f( 1.0,  1.0,  1.0);
	
		// draw top
		// added normal vector of top face
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-1.0,  1.0,  1.0);
		glVertex3f( 1.0,  1.0,  1.0);
		glVertex3f( 1.0,  1.0, -1.0);
		glVertex3f(-1.0,  1.0, -1.0);
	
		// draw bottom
		// added normal vector of bottom face
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-1.0, -1.0, -1.0);
		glVertex3f( 1.0, -1.0, -1.0);
		glVertex3f( 1.0, -1.0,  1.0);
		glVertex3f(-1.0, -1.0,  1.0);
	glEnd();
	
}

