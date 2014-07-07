#include "ofApp.h"

float near = 500;
float far = 4000;
int rate = 10;

int width;
int height;

ofEasyCam camera;

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
    
    whichMap = 5;
    
    capacity = 75;
    warp.setup(640, 480, capacity, OF_IMAGE_GRAYSCALE);
	warp.setBlending(true);
	warp.setDelayMap(*(sampleMaps[whichMap]));

    colorWarp.setup(640, 480, capacity, OF_IMAGE_COLOR);
	colorWarp.setBlending(true);
	colorWarp.setDelayMap(*(sampleMaps[whichMap]));
    
    background.load("beach.mp4", capacity * 2, *(sampleMaps[whichMap]));
    
    ofSetVerticalSync(true);
    kinect.setRegistration(true);
    kinect.init();
    
    width = kinect.getWidth();
    height = kinect.getHeight();
    
    pointCloud.setMode(OF_PRIMITIVE_POINTS);
    for (int x=0; x<width; x++){
        for (int y=0; y<height; y++){
                pointCloud.addVertex(ofVec3f(x,y,0));
                pointCloud.addColor(ofFloatColor(1,1,1,1));
            }
    }
    
    camera.setPosition(ofGetWidth()/2.0, ofGetHeight()/2.0, 500);
    camera.roll(180);
}

//--------------------------------------------------------------
void ofApp::update(){
    if ( lastMap != whichMap ){
        lastMap = whichMap;
        
        colorWarp.setDelayMap(*(sampleMaps[whichMap]));
        warp.setDelayMap(*(sampleMaps[whichMap]));
//        background.setDelayMap(*(sampleMaps[whichMap]));
    }
    kinect.update();
    kinect.setDepthClipping( near, far );
    static ofImage depthImage;
    depthImage.setFromPixels(kinect.getDepthPixelsRef());
    
    ofImage & img = warp.getOutputImage();
    ofImage & imgColor = colorWarp.getOutputImage();
    background.update();
    
    if ( kinect.isFrameNew()){
        
        warp.addImage(depthImage);
        colorWarp.addImage(kinect.getPixelsRef());
        for (int x=0; x<kinect.getWidth(); x++){
            for (int y=0; y<kinect.getHeight(); y++){
                int ind = y + x*kinect.getHeight();
                
                    ofVec3f v = pointCloud.getVertex(ind);//kinect.getWorldCoordinateAt(currentScan, y);
                    v.z = ofMap(img.getPixelsRef().getColor(x, y).r, 0, 255, -250, 250);
                    pointCloud.setVertex(ind, v);
                    if ( v.z != -250 ){
                        ofColor color = imgColor.getPixelsRef().getColor(x, y);
                        color.a = 255;
                        pointCloud.setColor(ind, color);//kinect.getColorAt(x, y));
                    } else {
                        pointCloud.setColor(ind, ofFloatColor(0,0));
                    }
            }
        }
    }
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 3));
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofDisableDepthTest();
    float scale = fmax( (float) ofGetWidth()/640.0, (float) ofGetHeight()/480.0);
    background.getImage().draw(0,0, 640.0 * scale, 480.0 * scale);
    
    glPointSize(4.0);
    ofEnableDepthTest();
    camera.begin();
    ofRotate(sin(ofGetElapsedTimeMillis() * .001) * 30.0, 0, 1.0, 0);
    //ofRotate(ofGetElapsedTimeMillis() * .05, 0, 1.0, 0);
    ofTranslate(-kinect.getWidth()/2.0, -kinect.getHeight()/4.0);
    pointCloud.draw();
    camera.end();
    
    if ( gui->isVisible() ){
        camera.disableMouseInput();
        kinect.drawDepth(0, 0, 640, 480);
    } else {
        camera.enableMouseInput();
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
