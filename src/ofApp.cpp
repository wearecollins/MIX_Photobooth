#include "ofApp.h"

// kinect thresholds
float near = 500;
float far = 4000;

// tumblr gifs can only be 500px wide and < 1 MB,
// wouldya believe it?
float gifScale = .5;

int width;
int height;

int lastCapacity = 0;

int whichMap = 0;
int lastMap = 0;

int whichMapBG = 0;
int lastMapBG = 0;

bool bCapturing = false;
bool bRecording = false;
int  addedFrames = 0;
int  maxFrames   = 15;

// last-ditch kinect reconnect
bool bReconnect = false;

bool bDrawOverlay = false;

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(false);
    ofSetFrameRate(60);
    ofSetCircleResolution(100);
    
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
    
    // images
    overlay.loadImage( gifScale == 1.0 ? "overlay.png" : "overlay_small.png");
    endcard.loadImage("endcard.png");
    
    ofDirectory dir;
    dir.allowExt("png");
    
    int n = dir.listDir(gifScale == 1.0 ? "minute" : "minute/small");
    for ( int i=0; i<n; i++){
        ofFile f = dir.getFile(i);
        string dig = f.getBaseName().substr(0,1);
        string lr = f.getBaseName().substr(2,1);
        if ( lr == "L" ){
            min_left[ofToInt(dig)].loadImage(dir.getPath(i));
        } else if ( lr == "R" ){
            min_right[ofToInt(dig)].loadImage(dir.getPath(i));
        } else {
        }
    }
    n = dir.listDir("countdown");
    for ( int i=0; i<n; i++){
        ofFile f = dir.getFile(i);
        string name = f.getBaseName();
        countdown[ofToInt(name)].loadImage(dir.getPath(i));
    }
    
    n = dir.listDir(gifScale == 1.0 ? "hours" : "hours/small");
    
    for ( int i=0; i<n; i++){
        ofFile f = dir.getFile(i);
        string name = f.getBaseName();
        hours[ofToInt(name)].loadImage(dir.getPath(i));
    }
    
    // settings
    capacity = 75;
    lastCapacity = capacity;
    
    gui = new ofxUICanvas(0, 0, ofGetWidth()/2.0, ofGetHeight());
    gui->addSlider("near", 0.0, 4000, &near);
    gui->addSlider("far", 0.0, 4000, &far);
    gui->addIntSlider("capacity", 0, 300, &capacity);
    gui->addIntSlider("whichMap", 0, sampleMaps.size()-1, &whichMap);
    gui->addIntSlider("whichMapBG", 0, sampleMaps.size()-1, &whichMapBG);
    gui->addToggle("reconnect Kinect", &bReconnect);
    gui->addToggle("draw overlay", &bDrawOverlay);
    gui->setVisible(false);
    gui->loadSettings("settings.xml");
    
    // setup slitscanners
    warp.setup(capacity, *(sampleMaps[whichMap]));
    background.load("beach.mov", capacity * 2, *(sampleMaps[whichMap]));
    
    // setup kinect
    kinect.setRegistration(true);
    kinect.init();
    
    width = kinect.getWidth();
    height = kinect.getHeight();
    
    // 3 timers!
    timers.resize(4);
    
    // recording FBO
    screenFbo.allocate(width * gifScale, height * gifScale, GL_RGB);
    gifMaker.setup(width * gifScale, height * gifScale);
    ofAddListener(gifMaker.OFX_GIF_SAVE_FINISHED, this, &ofApp::saveComplete);
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
    
    if ( lastCapacity != capacity ){
        lastCapacity = capacity;
        warp.setCapacity( capacity );
        background.setCapacity( capacity );
    }
    
    if ( bReconnect ){
        bReconnect = false;
        kinect.open();
    }
    
    kinect.update();
    kinect.setDepthClipping( near, far );
    
    background.update();
    
    if ( true || kinect.isFrameNew()){
        warp.update(kinect.getPixelsRef(), kinect.getDepthPixelsRef());
        
        // time to start countdown
        if ( warp.shouldCapture() ){
            timers.front().start(1500);
            bCapturing = true;
            whichImage = 0;
            addedFrames = 0;
            cout << "CAPTURE 2" <<endl;
        
        // capturing
        } else if ( bCapturing ){
            
            if ( !warp.getIsFull() ){
                cout <<"No longer full?"<<endl;
                for (int i=0; i<timers.size(); i++){
                    timers[i].stop();
                    bCapturing = false;
                    bRecording = false;
                }
            }
            for (int i=0; i<timers.size(); i++){
                if ( timers[i].isReady() ){
                    timers[i].stop();
                    whichImage = i + 1;
                    if ( whichImage < timers.size() )
                        timers[whichImage].start(1500);
                    cout << "CAPTURE " <<whichImage<<endl;
                    break;
                }
            }
            
            if ( whichImage == timers.size() ){
                bRecording = true;
                bCapturing = false;
                gifMaker.reset();
                cout << "RECORDING" <<endl;
                frameTimer.start(500);
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
void ofApp::drawOverlay( float scale ){
    overlay.draw(0,0, overlay.width * scale, overlay.height * scale);
    int h = ofGetHours();
    
    if ( h > 12 ) h -= 12;
    if ( h < 6 ) h = 6;
    
    string m = ofToString(ofGetMinutes());
    if ( m.length() == 1 ){
        m = "0" + m;
    }
    
    int ml = ofToInt(m.substr(0,1));
    int mr = ofToInt(m.substr(1,1));
    
    hours[h].draw(0, 0, hours[h].width * scale, hours[h].height * scale);
    min_left[ml].draw(0, 0, min_left[ml].width * scale, min_left[ml].height * scale);
    min_right[mr].draw(0, 0, min_right[mr].width * scale, min_right[mr].height * scale);
}

//--------------------------------------------------------------
void ofApp::draw(){
    //bg
    ofDisableDepthTest();
    float scale = fmax( (float) ofGetWidth()/width, (float) ofGetHeight()/height);
    background.getImage().draw(0,0, width * scale, height * scale);
    
    //fg
    warp.draw();
    
    // overlay
    if ( bDrawOverlay ) drawOverlay(scale);
    
    if ( bCapturing ){
        ofEnableAlphaBlending();
        ofSetColor(255, 255.0 - timers[whichImage].getPercentage() * 150.0 );
        countdown[4-whichImage].draw(rc::ofCenter() - ofPoint(countdown[4-whichImage].width/2.0, countdown[4-whichImage].height/2.0));
        ofSetColor(255);
        //ofDrawBitmapString(ofToString(timers.size() - whichImage), rc::ofCenter() );
    } else if ( bRecording){
        //ofDrawBitmapString("GET WEIRD!", rc::ofCenter() );
        
        ofSetColor(180, 0, 0, fabs( sin(ofGetElapsedTimeMillis() * .001)) * 255.0);
        ofCircle(75, ofGetHeight() - 75, 25);
        ofSetColor(255);
        
        if ( frameTimer.isReady() ){
        
            // draw both at normal scale
            screenFbo.begin();
            ofClear(255);
            ofPushMatrix();
                ofScale(gifScale, gifScale);
                background.getImage().draw(0,0);
                warp.draw( false );
            ofPopMatrix();
            drawOverlay();
            screenFbo.end();
            
            static ofPixels renderPix;
            screenFbo.readToPixels(renderPix);
            
            gifMaker.addFrame(renderPix.getPixels(), width * gifScale, height * gifScale);
            addedFrames++;
        }
    }
    
    if ( showTimer.isReady() ){
        showTimer.stop();
        enderTimer.start(5000);
    }
    
    if ( enderTimer.hasStarted() ){
        ofEnableAlphaBlending();
        if ( enderTimer.isReady() ) enderTimer.stop();
        ofSetColor(255, fabs( sin(ofGetElapsedTimeMillis() * .01)) * 255.0 );
        endcard.draw(rc::ofCenter() - ofPoint(endcard.width/2.0, endcard.height/2.0));
        ofSetColor(255);
    }
    
    if ( gui->isVisible() ){
        warp.disableCamera();
        kinect.drawDepth(0, 0, 640, 480);
    } else {
        warp.enableCamera();
    }
}

//--------------------------------------------------------------
void ofApp::saveComplete( string & filepath ){
    showTimer.start(2000);
    
    ofFile f(filepath);
    f.moveTo("../../images");
    
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
