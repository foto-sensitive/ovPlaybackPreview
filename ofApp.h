

/*Created by Robert Pavlovskis on 31/03/2016
This sketch will render a 360° preview out of several videos layers onto one, conserving CPU
The app is dependent on Greg Borenstein's ofxImageSequenceRecorder and Lukasz Karluk's
ofxImageSequencePlayer for writing and reading image sequences as videos.
The sketch references Mick Greirsons phasor functions from his
Maximillian addon for openFrameworks.*/



#pragma once

#include "ofMain.h"
#include "ofxImageSequence.h"
#include "ofxImageSequenceRecorder.h"

class ofApp : public ofBaseApp {

public:
	~ofApp();/* destructor is very useful */
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo info);
	void gotMessage(ofMessage msg);
	double phasor(double frequency, double startphase, double endphase);
	//Unwrap media 360° 
	void unwrap();
	void addNewFrame();
	void cycle();
	void exit();

	string path; //Path of dragged file

				 //Contain videos
	vector <ofVideoPlayer> videos;
	ofImage preview;

	unsigned char * pixelout;
	unsigned char * pixelin;
	unsigned char * pixels;

	//Iterators
	int i, j, u; 
	
	//Size
	int s;
	//Dimension
	int width, height;

	//Threshold for Chroma Key
	int thre;

    //Skip update for videos that have been written to preview
	bool skip[9]; 

	//Records Image Sequence
	ofxImageSequenceRecorder recorder;
	//Playback Image Sequence
	ofxImageSequence sequence;
	//Phase along the sequence timeline
	double phase;
	//Directory of sequence
	ofDirectory dir;

	//Allocates memeory to video once it has been uploaded
	bool allocate = false;
	//Sequence Recording is finished
	bool sequenceOver = false;
	//Check if new there is new frame
	bool newFrame = false;
	//Start recording cycle from frame 0
	bool record = false;
	//Prevent recording if ran out of frames
	bool loaded = false;
	//Toggle 360° unwrap
	bool toggle = false;


	//Colour channel values
	int red, green, blue;

	//360 Vision
	GLUquadricObj *quadric;
	ofEasyCam cam;

	//Store location of various pixels when looping through colour channels
	int pointer;

};