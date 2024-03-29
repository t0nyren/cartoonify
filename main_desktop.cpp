/*****************************************************************************
*   Cartoonifier_Desktop.cpp, for Desktop.
*   Converts a real-life camera stream to look like a cartoon.
*   This file is for a desktop executable, but the cartoonifier can also be used in an Android / iOS project.
******************************************************************************
*   by Shervin Emami, 5th Dec 2012 (shervin.emami@gmail.com)
*   http://www.shervinemami.info/
******************************************************************************
*   Ch1 of the book "Mastering OpenCV with Practical Computer Vision Projects"
*   Copyright Packt Publishing 2012.
*   http://www.packtpub.com/cool-projects-with-opencv/book
*****************************************************************************/


// Try to set the camera resolution. Note that this only works for some cameras on
// some computers and only for some drivers, so don't rely on it to work!
const int DESIRED_CAMERA_WIDTH = 640;
const int DESIRED_CAMERA_HEIGHT = 480;

const int NUM_STICK_FIGURE_ITERATIONS = 40; // Sets how long the stick figure face should be shown for skin detection.

const char *windowName = "Cartoonifier";   // Name shown in the GUI window.


// Set to true if you want to see line drawings instead of paintings.
bool m_sketchMode = false;
// Set to true if you want to change the skin color of the character to an alien color.
bool m_alienMode = false;
// Set to true if you want an evil "bad" character instead of a "good" character.
bool m_evilMode = false;
// Set to true if you want to see many windows created, showing various debug info. Set to 0 otherwise.
bool m_debugMode = false;



#include <stdio.h>
#include <stdlib.h>

// Include OpenCV's C++ Interface
#include "opencv2/opencv.hpp"

// Include the rest of our code!
//#include "detectObject.h"       // Easily detect faces or eyes (using LBP or Haar Cascades).
#include "cartoon.h"            // Cartoonify a photo.
#include "ImageUtils.h"      // Shervin's handy OpenCV utility functions.

using namespace cv;
using namespace std;

int m_stickFigureIterations = 0;  // Draws a stick figure outline for where the user's face should be.

#if !defined VK_ESCAPE
    #define VK_ESCAPE 0x1B      // Escape character (27)
#endif



// Get access to the webcam.
void initWebcam(VideoCapture &videoCapture, int cameraNumber)
{
    // Get access to the default camera.
    try {   // Surround the OpenCV call by a try/catch block so we can give a useful error message!
        videoCapture.open(cameraNumber);
    } catch (cv::Exception &e) {}
    if ( !videoCapture.isOpened() ) {
        cerr << "ERROR: Could not access the camera!" << endl;
        exit(1);
    }
    cout << "Loaded camera " << cameraNumber << "." << endl;
}


// Keypress event handler. Note that it should be a 'char' and not an 'int' to better support Linux.
void onKeypress(char key)
{
    switch (key) {
    case 's':
        m_sketchMode = !m_sketchMode;
        cout << "Sketch / Paint mode: " << m_sketchMode << endl;
        break;
    case 'a':
        m_alienMode = !m_alienMode;
        cout << "Alien / Human mode: " << m_alienMode << endl;
        // Display a stick figure outline when alien skin is enabled,
        // so the user puts their face in the correct place.
        if (m_alienMode) {
            m_stickFigureIterations = NUM_STICK_FIGURE_ITERATIONS;
        }
        break;
    case 'e':
        m_evilMode = !m_evilMode;
        cout << "Evil / Good mode: " << m_evilMode << endl;
        break;
    case 'd':
        m_debugMode = !m_debugMode;
        cout << "Debug mode: " << m_debugMode << endl;
        break;
    }
}


int main(int argc, char *argv[])
{
    cout << "Cartoonifier, by Shervin Emami (www.shervinemami.info), June 2012." << endl;
    cout << "Converts real-life images to cartoon-like images." << endl;
    cout << "Compiled with OpenCV version " << CV_VERSION << endl;
    cout << endl;

    cout << "Keyboard commands (press in the GUI window):" << endl;
    cout << "    Esc:  Quit the program." << endl;
    cout << "    s:    change Sketch / Paint mode." << endl;
    cout << "    a:    change Alien / Human mode." << endl;
    cout << "    e:    change Evil / Good character mode." << endl;
    cout << "    d:    change debug mode." << endl;
    cout << endl;

    // Allow the user to specify a camera number, since not all computers will be the same camera number.
    int cameraNumber = 0;   // Change this if you want to use a different camera device.
    //if (argc > 1) {
    //    cameraNumber = atoi(argv[1]);
    //}
	if (argc == 2){
		Mat img = imread(argv[1], CV_LOAD_IMAGE_COLOR);
		Mat cartoon = Mat(img.size(), CV_8UC3);
		cartoonifyImage(img, cartoon, m_sketchMode, m_alienMode, m_evilMode, 0);
		imwrite("cartoon.jpg", cartoon);
	}
	else if (argc == 3){

		// Get access to the camera.
		VideoCapture camera(argv[1]);
		// Try to set the camera resolution. Note that this only works for some cameras on
		// some computers and only for some drivers, so don't rely on it to work!
		camera.set(CV_CAP_PROP_FRAME_WIDTH, DESIRED_CAMERA_WIDTH);
		camera.set(CV_CAP_PROP_FRAME_HEIGHT, DESIRED_CAMERA_HEIGHT);	
		if (!camera.isOpened())
		{
			cout  << "Could not open the input video: "  << endl;
			return -1;
		}
		//initWebcam(camera, cameraNumber);

		VideoWriter outputVideo;
		int ex = static_cast<int>(camera.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form
		Size S = Size((int) camera.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
					  (int) camera.get(CV_CAP_PROP_FRAME_HEIGHT));
		outputVideo.open("output.avi", ex, camera.get(CV_CAP_PROP_FPS), S, true);
		if (!outputVideo.isOpened())
		{
			cout  << "Could not open the output video for write: "  << endl;
			return -1;
		}	


		// Create a GUI window for display on the screen.
		//namedWindow(windowName); // Resizable window, might not work on Windows.

		// Run forever, until the user hits Escape to "break" out of this loop.
		while (true) {

			// Grab the next camera frame. Note that you can't modify camera frames.
			Mat cameraFrame;
			camera >> cameraFrame;
			if( cameraFrame.empty() ) {
				cout<<"CameraFrame Empty"<<endl;
				break;
			}

			Mat displayedFrame = Mat(cameraFrame.size(), CV_8UC3);

			// Use debug type 2 (for desktop) if debug mode is enabled.
			int debugType = 0;
			if (m_debugMode)
				debugType = 2;

			// Run the cartoonifier filter using the selected mode.
			cartoonifyImage(cameraFrame, displayedFrame, m_sketchMode, m_alienMode, m_evilMode, debugType);

			// Show a stick-figure outline of a face for a short duration, so the user knows where to put their face.
			if (m_stickFigureIterations > 0) {
				drawFaceStickFigure(displayedFrame);
				m_stickFigureIterations--;
			}

			//imshow(windowName, displayedFrame);
			outputVideo << displayedFrame;
			// IMPORTANT: Wait for atleast 20 milliseconds, so that the image can be displayed on the screen!
			// Also checks if a key was pressed in the GUI window. Note that it should be a "char" to support Linux.
			//char keypress = waitKey(20);  // This is needed if you want to see anything!
			//if (keypress == VK_ESCAPE) {   // Escape Key
				// Quit the program!
			 //   break;
			//}
			// Process any other keypresses.
			//if (keypress > 0) {
			//    onKeypress(keypress);
			//}

		}//end while
	}
    return EXIT_SUCCESS;
}
