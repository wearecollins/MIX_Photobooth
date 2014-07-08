#include "ofApp.h"

// kinect thresholds
float near = 500;
float far = 4000;

int width;
int height;

int whichMap = 0;
int lastMap = 0;

int whichMapBG = 0;
int lastMapBG = 0;

bool bCapturing = false;
bool bRecording = false;
int  addedFrames = 0;
int  maxFrames   = 15;

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(false);
    ofSetFrameRate(60);
    
    // from @obviousjim's glorious addon
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
    
    // settings
    
    gui = new ofxUICanvas(0, 0, ofGetWidth()/2.0, ofGetHeight());
    gui->addSlider("near", 0.0, 4000, &near);
    gui->addSlider("far", 0.0, 4000, &far);
    gui->addIntSlider("whichMap", 0, sampleMaps.size()-1, &whichMap);
    gui->addIntSlider("whichMapBG", 0, sampleMaps.size()-1, &whichMapBG);
    gui->setVisible(false);
    gui->loadSettings("settings.xml");
    
    // setup slitscanners
    capacity = 75;
    warp.setup(capacity, *(sampleMaps[whichMap]));
    background.load("beach.mp4", capacity * 2, *(sampleMaps[whichMap]));
    
    // setup kinect
    kinect.setRegistration(true);
    kinect.init();
    
    width = kinect.getWidth();
    height = kinect.getHeight();
    
    // 3 timers!
    timers.resize(3);
    
    // recording FBO
    screenFbo.allocate(640, 480, GL_RGB);
    gifMaker.setup(640, 480);
}

//--------------------------------------------------------------
void ofApp::update(){
    if ( lastMap != whichMap ){
        lastMap = whichMap;
        warp.setMap(*(sampleMaps[whichMap]));
    }
    
    if ( lastMapBG != whichMapBG ){
        lastMapBG = whichMapBG;
        background.setMap(*(sampleMaps[whichMapBG]));
    }
    
    kinect.update();
    kinect.setDepthClipping( near, far );
    
    background.update();
    
    if ( kinect.isFrameNew()){
        warp.update(kinect.getPixelsRef(), kinect.getDepthPixelsRef());
        
        // time to start countdown
        if ( warp.shouldCapture() ){
            for (int i=0; i<timers.size(); i++){
                timers[i].start( i * 1200 );
            }
            bCapturing = true;
            whichImage = 0;
            addedFrames = 0;
            cout << "CAPTURE 2" <<endl;
        
        // capturing
        } else if ( bCapturing ){
            for (int i=0; i<timers.size(); i++){
                if ( timers[i].isReady() ){
                    timers[i].stop();
                    whichImage = i + 1;
                    cout << "CAPTURE " <<whichImage<<endl;
                    break;
                }
            }
            
            if ( whichImage == timers.size() ){
                bRecording = true;
                bCapturing = false;
                gifMaker.reset();
                cout << "RECORDING" <<endl;
            }
        // recording
        } else if ( bRecording ){
            if ( addedFrames >= maxFrames ){
                cout << "DONE" <<endl;
                bRecording = false;
                gifMaker.save( ofGetTimestampString() + ".gif");
            }
        }
    }
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 3));
}

//--------------------------------------------------------------
void ofApp::draw(){
    //bg
    ofDisableDepthTest();
    float scale = fmax( (float) ofGetWidth()/width, (float) ofGetHeight()/height);
    background.getImage().draw(0,0, width * scale, height * scale);
    
    //fg
    warp.draw();
    
    if ( bCapturing ){
        ofDrawBitmapString(ofToString(timers.size() - whichImage), rc::ofCenter() );
    } else if ( bRecording ){
        ofDrawBitmapString("GET WEIRD!", rc::ofCenter() );
        
        // draw both at normal scale
        screenFbo.begin();
        ofClear(255);
        background.getImage().draw(0,0);
        warp.draw( false );
        screenFbo.end();
        
        static ofPixels renderPix;
        screenFbo.readToPixels(renderPix);
        
        gifMaker.addFrame(renderPix.getPixels(), 640, 480);
        addedFrames++;
    }
    
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
