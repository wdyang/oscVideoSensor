//
//  Created by Weidong Yang on 12/1/13.
//  Kinetech Arts
//
//
#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxMidi.h"
#include "soundBlock.h"
#include "ofxMacamPs3Eye.h"
#include "ofxOsc.h"

#define HOST "localhost"
//#define PORT 12345
#define PORT 8000

class testApp : public ofBaseApp, public ofxMidiListener {
public:
	void setup();
	void update();
	void draw();
    
    void keyPressed  (int key);

    bool bUsePs3Eye=true;
    
    int camW=320;
    int camH=240;
    int drawW=800;
    int drawH=600;
    float drawCamRatio = 2.5;
	
    float camGain = 0.2;
    float camShutter = 1.0;
    float camHue = 0.5;
    
    float distortionStrength = 12.0;
    float zFactor = 0.2;
    
	ofVideoGrabber cam;
	ofxCv::FlowFarneback flow;
	ofMesh mesh;
	int stepSize = 2;  //Controls grid size for mesh in optical flow. Used to be 8, to save processing power
    int xSteps, ySteps;
    bool bMirror=true;
    bool bReportMax = true;
    
    bool bReportXY; //if detected speed exceed threshold, it will be set to true.
    int reportX=0;
    int reportY=0;
    
    
    ofFbo faceFbo;
    
//    vector<ofSoundPlayer> players;
    int nCol = 5;
    int nRow = 3;
    int targetW=40;
    int targetH=30;
    bool bUseGarrett = true; //http://www.youtube.com/channel/UCP7pEisWsX4sSzsIqd7CdtA?feature=watch

    void setUpNormalGrid();
    void setUpGarrettGrid();
    
    int colSteps, rowSteps;
    int numPlayers;
//    vector<float> playerVolume;
//    vector<float> playerSpeed;

    bool bLoop = false;
    vector<SoundBlock> soundBlocks;

    void newMidiMessage(ofxMidiMessage& eventArgs);
    
    ofxMidiIn midiIn;
//    ofxMidiMessage midiMessage;
//    stringstream text;
    
    ofxMacamPs3Eye ps3eye;
    
    
    ofxOscSender sender;
    void oscLiveTriggerPlay(int track, int clip); //0, 1, ... We use clip as sets here
    void oscLiveTriggerStop(int track, int clip);
    void oscLiveSetVolume(int track, float volume);
    void oscRenoiseNoteOn(int t, int i, int n, int v); //track(0,1,..), instrument, note(60 is C4), velocity (0-127)
    void oscRenoiseNoteOff(int t, int i, int n, int v);
    void oscSendAverageLocation(int x, int y);
    void oscSendVideoPosition(int x);

};