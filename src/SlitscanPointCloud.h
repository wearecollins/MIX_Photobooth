//
//  SlitscanPointCloud.h
//  BeachVibes
//
//  Created by Brett Renfer on 7/7/14.
//
//

#pragma mark once

#include "ofMain.h"
#include "ofxRCUtils.h"

struct Sorter {
    Sorter(vector<ofVec3f> & v) :
    verts(v){
    };
    vector<ofVec3f> & verts;
    bool operator() (ofIndexType i, ofIndexType j) {
        return (verts[i].z < verts[j].z);
    }
};

class SlitscanPointCloud : public ofThread {
public:
    
    SlitscanPointCloud() :
    captureThresh( 640 * 480 * .02 ),
    isFull(false){
    }
    
    ~SlitscanPointCloud(){
        waitForThread();
    }
    
    void setup( int cap, ofImage & map ){
        lastCapacity = capacity = cap;
        colorImage.setUseTexture(false);
        depthImage.setUseTexture(false);
        
        width   = 640;
        height  = 480;
        
        slitscanColor.setup(width, height, capacity, OF_IMAGE_COLOR);
        slitscanColor.setBlending(true);
        slitscanColor.setDelayMap(map);
        slitscanColor.getOutputImage().setUseTexture(false);
        
        slitscanDepth.setup(width, height, capacity, OF_IMAGE_GRAYSCALE);
        slitscanDepth.setBlending(true);
        slitscanDepth.setDelayMap(map);
        slitscanDepth.getOutputImage().setUseTexture(false);
        quicklight.move(0,0,500);

        
        pointCloud.setMode(OF_PRIMITIVE_POINTS);
        for (int x=0; x<width; x++){
            for (int y=0; y<height; y++){
                verts.push_back( ofVec3f(x,y,0) );
                colors.push_back(ofFloatColor(1.0,1.0,1.0,1.0));
                indices.push_back(y + x*height);
            }
        }
        sorter = new Sorter(verts);
        
        camera.setPosition(ofGetWidth()/2.0, ofGetHeight()/2.0, 500);
        camera.roll(180);
        fbo.allocate(ofGetWidth(), ofGetHeight());
    }
    
    void update( ofPixelsRef & color, ofPixelsRef & depth ){
        lock();
        colorImage.setFromPixels( color );
        depthImage.setFromPixels( depth );
        if ( bNew ){
            pointCloud.clear();
            pointCloud.addVertices(verts);
            pointCloud.addColors(colors);
            pointCloud.addIndices(indices);
            bNew = false;
        }
        unlock();
        
        if (!isThreadRunning()){
            startThread();
            return;
        }
        if ( fbo.getWidth() != ofGetWidth() ){
            //fbo.allocate(ofGetWidth(), ofGetHeight());
        }
    }
    
    void draw( bool bFill = true ){
        ofEnableLighting();
        quicklight.enable();
        //quicklight.setPosition(ofGetWidth()/2.0, ofGetHeight()/2.0, 500);
        quicklight.roll(sin(ofGetElapsedTimeMillis() * .001));
        ofEnableAlphaBlending();
        ofPushMatrix();
        glPushAttrib(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_POINT_SMOOTH);
        glPointSize( bFill ? 3.0 : 1.0 );
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_POINT_SPRITE);
//        glDepthMask(GL_TRUE);
        ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
        float scale = bFill ? fmin((float) (ofGetWidth())/width, (float) (ofGetHeight()) / height ) : 1.0;
        ofRotate(sin(ofGetElapsedTimeMillis() * .0005) * 30.0, 0, 1.0, 0);
        ofScale(scale, scale, scale);
        ofTranslate(-width/2.0, -height/2.0);
        pointCloud.draw();
        ofPopMatrix();
        glPopAttrib();
        ofDisableLighting();
    }
    
    void threadedFunction(){
        while(isThreadRunning()){
            slitscanColor.addImage(colorImage);
            slitscanDepth.addImage(depthImage);
//            imageBuffer.setFromPixels(slitscan.getOutputImage().getPixelsRef());
            
            ofImage & img = slitscanDepth.getOutputImage();
            ofImage & imgColor = slitscanColor.getOutputImage();
            
            int count = 0;
            for (int x=0; x<width; x++){
                for (int y=0; y<height; y++){
                    if ( depthImage.getPixelsRef().getColor(x, y).r > 10 ){
                        count++;
                    }
                }
            }
            
            for (int x=0; x<width; x++){
                for (int y=0; y<height; y++){
                    int ind = y + x*height;
                    
                    ofVec3f & v = verts[ind];//kinect.getWorldCoordinateAt(currentScan, y);
                    v.z = ofMap(img.getPixelsRef().getColor(x, y).r, 0, 255, -500, 0);
                    if ( v.z != -500 ){
                        ofColor color = imgColor.getPixelsRef().getColor(x, y);
                        color.a = 255;
                        colors[ind] = color;//ofFloatColor(color.r/255.0,color.g/255.0,color.b/255.0,color.a/255.0);//kinect.getColorAt(x, y));
                        indices[ind] = ind;
                    } else {
                        colors[ind] = ofFloatColor(0,0);
                    }
                }
            }
            
            // sort indices by depth
            //std::sort( indices.begin(), indices.end(), *sorter );
            
            if ( (!restartTimer.hasStarted() || restartTimer.isReady() ) && !isFull && count > captureThresh ){
//                cout << "CAPTURE" << endl;
                isFull = true;
                timer.start(500);
            } else if ( count < captureThresh && !emptyTimer.hasStarted() ){
//                cout << "Not full anymore?"<<endl;
                timer.stop();
                isFull = false;
                emptyTimer.start(500);
            } else if ( emptyTimer.hasStarted() && count > captureThresh ){
//                cout << "Full again?"<<endl;
                emptyTimer.stop();
            }
            
            if ( lastCapacity != capacity ){
                lastCapacity = capacity;
                slitscanColor.setCapacity(capacity);
                slitscanDepth.setCapacity(capacity);
                slitscanColor.setTimeDelayAndWidth(0, capacity);
                slitscanDepth.setTimeDelayAndWidth(0, capacity);
            }
            
            bNew = true;
            sleep(16);
            yield();
        }
    }
    
    // public props
    int captureThresh;
    
    bool getIsFull(){
        return !emptyTimer.isReady();
    }
    
    // methods
    bool shouldCapture(){
        if ( isFull && timer.isReady() ){
            isFull = false;
            restartTimer.start( 30000 );
            return true;
        } else {
            return false;
        }
    }
    
    void setCapacity( int cap ){
        capacity = cap;
    }
    
    void setMap( ofImage & map ){
        slitscanColor.setDelayMap(map);
        slitscanDepth.setDelayMap(map);
    }
    
    void disableCamera(){
        camera.disableMouseInput();
    }
    
    void enableCamera(){
        camera.enableMouseInput();
    }
    
protected:
    
    ofLight quicklight;
    ofEasyCam camera;
    int capacity, lastCapacity;
    
    // sketch
    Sorter * sorter;
    
    bool isFull;
    rc::Timer timer, emptyTimer, restartTimer;
    
    ofxSlitScan slitscanColor, slitscanDepth;
    ofImage colorImage, depthImage;
    ofVideoPlayer video;
    bool bNew;
    ofMesh  pointCloud;
    int width, height;
    
    vector<ofIndexType> indices;
    vector<ofVec3f> verts;
    vector<ofFloatColor> colors;
    ofFbo fbo;
};
