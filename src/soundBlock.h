//
//  soundBlock.h
//  cSwell
//
//  Created by Weidong Yang on 12/1/13.
//
//

#ifndef __cSwell__soundBlock__
#define __cSwell__soundBlock__
#include "ofMain.h"

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

//
//#include "ofxMidi.h"

enum SoundBlockState {Off, TriggerPlay, On, TriggerStop};

float zDown;

class SoundBlock{
public:
    bool bDrawCircle=true;
    SoundBlock();
    SoundBlock(float _x, float _y, float _w, float _h, int _camW, int _camH, int _id, bool _bLoop);
    
    SoundBlockState currentState = Off;

    int id = -1;
    int cntSinceLastTrigger = 0;
    int camW=320;
    int camH=240;
    float centerX=160;
    float centerY=120;
    float width=40;
    float height=30;
    float r = 25;
    float xmin=0;
    float xmax=320;
    float ymin=0;
    float ymax=240;
    float triggerThreshold = 0.3;
    string fname;
    string shortFname;
    bool bLoop = true;
    
    
    void init(float _x, float _y, float _w, float _h, int _camW, int _camH);
    void setMinMax();
    void draw(bool bMirror, float drawCamRatio);
    float update(ofMesh _mesh, float stepSize, float xSteps, float ySteps);
    void detectState();
    

    float volume=0;
    float prevVolume=0;
    float zUp=0.99; //higher z faster following
    static float zDown;
    float maxSpeed = 10;
    float pan=0; //-1 to 1
    float volumeScale = 1;
    
//    std::stringstream text;
};


#endif /* defined(__cSwell__soundBlock__) */
