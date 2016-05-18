

/*Created by Robert Pavlovskis on 31/03/2016
This sketch will render a 360° preview out of several videos layers onto one, conserving CPU
The app is dependent on Greg Borenstein's ofxImageSequenceRecorder and Lukasz Karluk's 
ofxImageSequencePlayer for writing and reading image sequences as videos.
The sketch references Mick Greirsons phasor functions from his
Maximillian addon for openFrameworks.*/


#include "ofApp.h"


//-------------------------------------------------------------
ofApp::~ofApp() {

}

//--------------------------------------------------------------
void ofApp::setup() {

	//prepare quadric for sphere
	quadric = gluNewQuadric();
	gluQuadricTexture(quadric, GL_TRUE);
	gluQuadricNormals(quadric, GLU_SMOOTH);


	//Setup sequence recorder
	recorder.setPrefix(ofToDataPath("recording1/frame")); // this directory must already exist
    //png is really slow but high res, bmp is fast but big
	recorder.setFormat("bmp"); 
	recorder.stopThread();
	record = false;

	//Place camera inside sphere at 0,0,0
	cam.setAutoDistance(false);
}

//--------------------------------------------------------------
void ofApp::update() {

	for (i = 0; i < videos.size(); i++)

		//Skip update for videos that have been written to preview
			if (!skip[i])
				videos[i].update();

	//Prevents out of bounds errors if video hasn't been uploaded
	if (videos.size() > 0) {

		width = videos[0].getWidth();
		height = videos[0].getHeight();

		//Allocates memeory to video once it has been uploaded
		if (!allocate) {
			pixelout = new unsigned char[width*height * 3];
			pixelin = new unsigned char[width*height * 3];
			allocate = true;
		}

		//Cycle throguh all frames for recording, starting at 0 
		if (record) {
			cycle();
		}
		
		//Check if new there is new frame
		if (newFrame) { 
			//Add new frame once passed through Chroma-Key filter
			addNewFrame(); 
		}

	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	ofBackground(125);
	ofSetColor(255);



	//Draw source videos of the preview
	for (i = 0; i < videos.size(); i++)
		videos[i].draw((ofGetWidth() / videos.size()*i), 0, ofGetWidth() / videos.size(), ofGetHeight() / videos.size());


	//Check if there are videos to record
	if (videos.size() > 0) { 
 						
		//Prevent recording if all the frames have been cycled through
		if (loaded) { 

			recorder.stopThread();
			record = false;

			//Get directory size
			dir.listDir("recording1");

			sequence.loadSequence("recording1/frame", "bmp", 0, dir.size() - 1, 4);
			//Preload frames, this way there is no stutter
			sequence.preloadAllFrames();
			sequence.setFrameRate(5); 
			//Play sequence bool for when it has been written
			sequenceOver = true; 

			for (int j = 0; j < videos.size(); j++)
				skip[j] = true; //Prevents original videos from playing because composite has loaded, saving memory

			loaded = false;
		}
	}

	//If sequence has been created, play it
	if (sequenceOver) 
		sequence.getFrameAtPercent(phasor(0.005, 0, 1))->draw(ofGetWidth()*0.5, ofGetHeight()*0.5, ofGetWidth()*0.5, ofGetHeight()*0.5);


	//Activated for 360 preview
	if (toggle){
		cam.begin();
		unwrap();
		cam.end();
	}


	ofSetColor(0, 130);
	ofRect(10, 10, 372, 110);
	ofSetColor(255);

	//Tooltip
	ofDrawBitmapString("Drag  & drop one video  and  then another to", 20, 30);
	ofDrawBitmapString("load them. Press R to write them both into a", 20, 50);
	ofDrawBitmapString("green-screen, one video, playback, preview, ", 20, 70);
	ofDrawBitmapString("press T to toggle 360 view.", 20, 90);
}

void ofApp::exit() {
	//Tell thread to stop
	recorder.waitForThread();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

	if (key == 'r') { 
		record = true;

		recorder.setCounter(0);

		//Resets all videos so they are recorded simultaneously
		for (i = 0; i < videos.size(); i++)
			videos[i].setFrame(0);

		recorder.startThread(false, true);

        //Make sure all videos are being updated
		for (int j = 0; j < videos.size(); j++)
			skip[j] = false;
		//And paused for manual cycling through "nextFrame()" command
		for (i = 0; i < videos.size(); i++)
			videos[i].setPaused(true);
	}

	//Turns on spherical 360 unwrap and camera controls
	if (key == 't') { 
		toggle = !toggle;
	}


}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo info) {

	//Gets video path
	path = info.files[0];

	//Creates new video player
	ofVideoPlayer player;
	player.loadMovie(path);
	player.setLoopState(OF_LOOP_NORMAL);
	player.play();
	videos.push_back(player);

}

//--------------------------------------------------------------

double ofApp::phasor(double frequency, double startphase, double endphase) {
	//This Phasor takes a value for the start and end of the ramp. 
	//credit to Mick Grierson and his Maximillian addon
	if (phase<startphase) {
		phase = startphase;
	}
	if (phase >= endphase) phase = startphase;
	phase += ((endphase - startphase) / (1 / (frequency)));
	return(phase);
}


//--------------------------------------------------------------
void ofApp::unwrap() {

	ofSetColor(255, 255, 255);

	sequence.getTextureReference().bind();

	ofPushMatrix();
	ofRotateX(90);

	//Apply operations to texture matrix
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

    //Scale texture to sphere
	ofScale(sequence.getWidth(), sequence.getHeight());

	//Apply operations to model matrix
	glMatrixMode(GL_MODELVIEW);

	//Enable depth comparisons, needed so you see the correct planes from your perspective
	glEnable(GL_DEPTH_TEST);
	//Disables the use of ARB textures, has to be disabled for drawing sphere
	ofDisableArbTex();

	gluSphere(quadric, 200, 100, 100);

	//Disables depth comparisons, needed for performing texture operations
	glDisable(GL_DEPTH_TEST);
	//Enables the use of ARB textures, needed to bind textures
	ofEnableArbTex(); //needed for textures to work with gluSphe

	//Apply operations to texture matrix
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	sequence.getTextureReference().unbind();

	ofPopMatrix();
}


//--------------------------------------------------------------
void ofApp::addNewFrame() {

	//Compiles all videos into new frame for the image sequence being recorded

	if (record) {
	    for (u = 0; u < videos.size(); u++) {

			pixelin = videos[u].getPixels();

			//Writes pixels if Chroma-Key trheshold hasn't been reached, otherwise assumes pixel from previous layer
			for (i = 0; i < width; i++) {
				for (j = 0; j < height; j++) { 

					//Set threshold
 					thre = 50;
					 
					//Navigate one dimensional array of a two dimensional image
					pointer = (j*width + i);

					//Iterate through seperate colour channels
					red = pixelin[pointer * 3 + 0];//red
					green = pixelin[pointer * 3 + 1];//green
					blue = pixelin[pointer * 3 + 2];//blue

	                //Determines weather to draw pixel, based on the green value and other colours present in it.
					if (u == 0 || green <= blue || green <= red || green < thre) {

						pixelout[(j*width + i) * 3 + 0] = red;
						pixelout[(j*width + i) * 3 + 1] = green;
						pixelout[(j*width + i) * 3 + 2] = blue;

					}
				}
			}
			//Writes pixels to image
			preview.setFromPixels(pixelout, width, height, OF_IMAGE_COLOR);
		}

	}
	//Writes image to sequence
	recorder.addFrame(preview);
	newFrame = false;
}

//--------------------------------------------------------------
void ofApp::cycle() { 
	//Cycle throguh all frames for recording
	for (i = 0; i < videos.size(); i++) {

		videos[i].nextFrame();
		newFrame = true;
		
		//End cycle
		if (videos[0].getCurrentFrame() >= videos[0].getTotalNumFrames() || videos[0].getPosition() >= 0.99) {
			videos[i].setFrame(0);
			record = false;
			loaded = true;
		}
	}
}