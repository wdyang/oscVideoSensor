//
//  soundBlock.cpp
//  cSwell
//
//  Created by Weidong Yang on 12/1/13.
//
//

#include "soundBlock.h"
float SoundBlock::zDown = 0.99;

SoundBlock::SoundBlock(){
}

SoundBlock::SoundBlock(float _x, float _y, float _w, float _h, int _camW, int _camH, int _id, bool _bLoop){
    id = _id;
    bLoop = _bLoop;
    init(_x, _y, _w, _h, _camW, _camH);
}

void SoundBlock::init(float _x, float _y, float _w, float _h, int _camW, int _camH){
    centerX = _x;
    centerY= _y;
    width = _w;
    height = _h;
    r = (_w+_h)/4.0;
    volume = 0;
    camW = _camW;
    camH = _camH;
    setMinMax();
}

void SoundBlock::setMinMax(){
    xmin = centerX - width/2.0;
    xmax = centerX + width/2.0;
    ymin = centerY - height/2.0;
    ymax = centerY + height/2.0;
    
    xmin = xmin < 0 ? 0 : xmin;
    xmax = xmax > (camW-1) ? (camW - 1) : xmax;
    
    ymin = ymin < 0 ? 0 : ymin;
    ymax = ymax > (camH-1) ? (camH - 1) : ymax;
    
}

void SoundBlock::detectState(){
    if(volume<triggerThreshold){
        if(currentState==TriggerStop){
            currentState = Off;
        }else if(currentState==On || currentState == TriggerPlay){
            currentState = TriggerStop;
        }
    }else{
        if(currentState==TriggerPlay){
            currentState = On;
        }else if(currentState==Off || currentState == TriggerStop){
            currentState = TriggerPlay;
        }
    }
}

float SoundBlock::update(ofMesh _mesh, float stepSize, float xSteps, float ySteps){
    int yStart = floor(ymin/stepSize);
    int xStart = floor(xmin/stepSize);
    int yStop = ceil(ymax/stepSize);
    int xStop = ceil(xmax/stepSize);
    
    float sumSpeed=0;
    float numPoints=0;
    for(int y=yStart; y<yStop; y++){
        for(int x=xStart; x<xStop; x++){
            int i = y*xSteps + x;
            ofVec2f pos(x*stepSize, y*stepSize);
            ofVec2f offset = _mesh.getVertex(i);
            float speed = (offset-pos).length();
            sumSpeed += speed;
            numPoints++;
        }
    }
//    cout<<sumSpeed<<endl;
    sumSpeed/=numPoints;
    sumSpeed = sumSpeed > maxSpeed ? maxSpeed : sumSpeed;
    float z;
    if(sumSpeed/maxSpeed>prevVolume) {
        z=zUp;
    }else{
        z=zDown;
    }
//    cout<<z<<endl;
    volume = volume * (1-z) + sumSpeed/maxSpeed * z;
    prevVolume = volume;
    
    if(cntSinceLastTrigger>0) cntSinceLastTrigger--;
    
    detectState();
    return volume;
}

void SoundBlock::draw(bool bMirror, float drawCamRatio){
    stringstream text;
    text<<id<<" "<<shortFname;
    ofEnableAlphaBlending();
    ofSetColor(255, 255, 255, volume*256);
    if(bMirror){
        if(bDrawCircle){
            ofCircle(drawCamRatio*(camW-centerX), drawCamRatio*centerY, drawCamRatio*r);
        }else{
            ofRect(drawCamRatio*(camW-xmax), drawCamRatio*ymin, drawCamRatio*(xmax-xmin), drawCamRatio*(ymax-ymin));
        }
        ofDrawBitmapString(text.str(), drawCamRatio*(camW-xmax), drawCamRatio*ymin);
    }else{
        if(bDrawCircle){
            ofCircle(drawCamRatio*centerX, drawCamRatio*centerY, drawCamRatio*r);
        }else{
            ofRect(drawCamRatio*xmin, drawCamRatio*ymin, drawCamRatio*(xmax-xmin), drawCamRatio*(ymax-ymin));
        }
        ofDrawBitmapString(text.str(), drawCamRatio*xmin, drawCamRatio*ymin);
    }
    ofDisableAlphaBlending();
//    text.str("");
}