#include "ofApp.h"

float near = 500;
float far = 4000;
int rate = 10;

int width;
int height;

int whichMap = 0;
int lastMap = 0;
ofImage backgroundImage;

//--------------------------------------------------------------
void ofApp::setup(){
    sampleMapStrings.push_back("maps/left_to_right.png");
	sampleMapStrings.push_back("maps/right_to_left.png");
	sampleMapStrings.push_back("maps/up_to_down.png");
	sampleMapStrings.push_back("maps/down_to_up.png");
	sampleMapStrings.push_back("maps/hard_noise.png");
	sampleMapStrings.push_back("maps/soft_noise.png");
	sampleMapStrings.push_back("maps/random_grid.png");
	sampleMapStrings.push_back("maps/video_delay.png");
    
	for(int i = 0; i < sampleMapStrings.size(); i++){
		ofImage* map = new ofImage();
		map->allocate(640, 480, OF_IMAGE_GRAYSCALE);
		map->loadImage(sampleMapStrings[i]);
		sampleMaps.push_back( map );
	}
    
    gui = new ofxUICanvas(0, 0, ofGetWidth()/2.0, ofGetHeight());
    gui->addSlider("near", 0.0, 4000, &near);
    gui->addSlider("far", 0.0, 4000, &far);
    gui->addIntSlider("whichMap", 0, sampleMaps.size()-1, &whichMap);
    gui->setVisible(false);
    gui->loadSettings("settings.xml");
    
    capacity = 75;
    
    warp.setup(capacity, *(sampleMaps[whichMap]));
    background.load("beach.mp4", capacity * 2, *(sampleMaps[whichMap]));
    
    ofSetVerticalSync(true);
    kinect.setRegistration(true);
    kinect.init();
    
    width = kinect.getWidth();
    height = kinect.getHeight();
}

//--------------------------------------------------------------
void ofApp::update(){
    if ( lastMap != whichMap ){
        lastMap = whichMap;
        warp.setMap(*(sampleMaps[whichMap]));
        background.setMap(*(sampleMaps[whichMap]));
    }
    kinect.update();
    kinect.setDepthClipping( near, far );
    
    background.update();
    
    
    if ( kinect.isFrameNew()){
        warp.update(kinect.getPixelsRef(), kinect.getDepthPixelsRef());
    }
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 3));
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofDisableDepthTest();
    float scale = fmax( (float) ofGetWidth()/640.0, (float) ofGetHeight()/480.0);
    background.getImage().draw(0,0, 640.0 * scale, 480.0 * scale);
    
    warp.draw();
    
    if ( gui->isVisible() ){
        warp.disableCamera();
        kinect.drawDepth(0, 0, 640, 480);
    } else {
        warp.enableCamera();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if ( key =='g'){
        gui->toggleVisible();
    } else if ( key =='s'){
        gui->saveSettings("settings.xml");
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
