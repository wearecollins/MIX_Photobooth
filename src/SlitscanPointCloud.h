//
//  SlitscanPointCloud.h
//  BeachVibes
//
//  Created by Brett Renfer on 7/7/14.
//
//

#pragma mark once

class SlitscanPointCloud : public ofThread {
public:
    
    ~SlitscanPointCloud(){
        waitForThread();
    }
    
    void setup( int capacity, ofImage & map ){
        colorImage.setUseTexture(false);
        depthImage.setUseTexture(false);
        
        slitscanColor.setup(640, 480, capacity, OF_IMAGE_COLOR);
        slitscanColor.setBlending(true);
        slitscanColor.setDelayMap(map);
        slitscanColor.getOutputImage().setUseTexture(false);
        
        slitscanDepth.setup(640, 480, capacity, OF_IMAGE_GRAYSCALE);
        slitscanDepth.setBlending(true);
        slitscanDepth.setDelayMap(map);
        slitscanDepth.getOutputImage().setUseTexture(false);
        
        width   = 640;
        height  = 480;
        
        pointCloud.setMode(OF_PRIMITIVE_POINTS);
        for (int x=0; x<width; x++){
            for (int y=0; y<height; y++){
                verts.push_back( ofVec3f(x,y,0) );
                colors.push_back(ofFloatColor(1.0,1.0,1.0,1.0));
            }
        }
        
        camera.setPosition(ofGetWidth()/2.0, ofGetHeight()/2.0, 500);
        camera.roll(180);
        fbo.allocate(ofGetWidth(), ofGetHeight());
    }
    
    void setMap( ofImage & map ){
        slitscanColor.setDelayMap(map);
        slitscanDepth.setDelayMap(map);
    }
    
    void update( ofPixelsRef & color, ofPixelsRef & depth ){
        lock();
        colorImage.setFromPixels( color );
        depthImage.setFromPixels( depth );
        if ( bNew ){
            pointCloud.clear();
            pointCloud.addVertices(verts);
            pointCloud.addColors(colors);
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
        render();
    }
    
    void render(){
        ofEnableAlphaBlending();
//        fbo.begin();
//        ofClear(255,0.0);
        ofPushMatrix();
        glEnable(GL_POINT_SMOOTH);
        glPointSize(4.0);
//        ofEnableDepthTest();
        ofTranslate(ofGetWidth()/2.0, ofGetHeight()/2.0);
        float scale = fmin((float) (ofGetWidth())/width, (float) (ofGetHeight()) / height );
        ofRotate(sin(ofGetElapsedTimeMillis() * .0005) * 30.0, 0, 1.0, 0);
        ofScale(scale, scale, scale);
        ofTranslate(-width/2.0, -height/2.0);
        pointCloud.draw();
        ofPopMatrix();
        ofDisableDepthTest();
//        fbo.end();
    }
    
    void draw(){
//        fbo.draw(0,0);
        render();
    }
    
    void threadedFunction(){
        while(isThreadRunning()){
            slitscanColor.addImage(colorImage);
            slitscanDepth.addImage(depthImage);
//            imageBuffer.setFromPixels(slitscan.getOutputImage().getPixelsRef());
            
            
            ofImage & img = slitscanDepth.getOutputImage();
            ofImage & imgColor = slitscanColor.getOutputImage();
            
            for (int x=0; x<width; x++){
                for (int y=0; y<height; y++){
                    int ind = y + x*height;
                    
                    ofVec3f & v = verts[ind];//kinect.getWorldCoordinateAt(currentScan, y);
                    v.z = ofMap(img.getPixelsRef().getColor(x, y).r, 0, 255, -500, 0);
                    if ( v.z != -500 ){
                        ofColor color = imgColor.getPixelsRef().getColor(x, y);
                        color.a = 255;
                        colors[ind] = color;//ofFloatColor(color.r/255.0,color.g/255.0,color.b/255.0,color.a/255.0);//kinect.getColorAt(x, y));
                    } else {
                        colors[ind] = ofFloatColor(0,0);
                    }
                }
            }
        
            bNew = true;
            sleep(16);
        }
    }
    
    void disableCamera(){
        camera.disableMouseInput();
    }
    
    void enableCamera(){
        camera.enableMouseInput();
    }
    
protected:
    ofEasyCam camera;
    
    ofxSlitScan slitscanColor, slitscanDepth;
    ofImage colorImage, depthImage;
    ofVideoPlayer video;
    bool bNew;
    ofMesh  pointCloud;
    int width, height;
    vector<ofVec3f> verts;
    vector<ofFloatColor> colors;
    ofFbo fbo;
};
