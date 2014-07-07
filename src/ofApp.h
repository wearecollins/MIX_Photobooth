#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxUI.h"
#include "ofxSlitScan.h"
#include "BackgroundImage.h"
#include "SlitscanPointCloud.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
        ofxKinect       kinect;
        ofxUICanvas *   gui;
        ofShader        render;
        vector<string> sampleMapStrings;
        vector<ofImage*> sampleMaps;
    
        SlitscanPointCloud  warp;
        BackgroundImage     background;
        int capacity;
};
