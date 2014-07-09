//
//  BackgroundImage.h
//  BeachVibes
//
//  Created by Brett Renfer on 7/7/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxSlitScan.h"

class BackgroundImage : public ofThread {
public:
    
    ~BackgroundImage(){
        waitForThread();
    }
    
    void load( string path, int cap, ofImage & map ){
        capacity = cap;
        video.loadMovie(path);
        video.setUseTexture(false);
        video.play();
        video.setVolume(0);
        
        videoImage.setUseTexture(false);
        imageBuffer.setUseTexture(false);
        
        slitscan.setup(640, 480, capacity, OF_IMAGE_COLOR);
        slitscan.setBlending(true);
        slitscan.setDelayMap(map);
        slitscan.getOutputImage().setUseTexture(false);
        
        startThread();
    }
    
    void setCapacity( int cap ){
        capacity = cap;
    }
    
    void setMap( ofImage & map ){
        slitscan.setDelayMap(map);
    }
    
    void update(){
        if ( bNew ){
            lock();
            image.setFromPixels( imageBuffer.getPixelsRef());
            bNew = false;
            unlock();
        }
    }
    
    void threadedFunction(){
        while(isThreadRunning()){
            video.update();
            videoImage.setFromPixels(video.getPixelsRef());
            videoImage.resize(640,480);
            slitscan.addImage(videoImage);
            imageBuffer.setFromPixels(slitscan.getOutputImage().getPixelsRef());
            bNew = true;
            
            slitscan.setCapacity(capacity);
            slitscan.setTimeDelayAndWidth(0, capacity);
            sleep(16);
            yield();
        }
    }
    
    ofImage & getImage(){
        return image;
    }
    
protected:
    int capacity;
    
    ofxSlitScan slitscan;
    ofImage videoImage, image, imageBuffer;
    ofVideoPlayer video;
    bool bNew;
};
