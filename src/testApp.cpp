//
//  Created by Weidong Yang on 12/1/13.
//
//
#include "testApp.h"

using namespace ofxCv;
using namespace cv;

void testApp::setup() {
//    if(bUsePs3Eye){
    int numPs3Eye = ps3eye.listDevices().size();
    cout<<"Number of Ps3 Eye connected: "<< numPs3Eye<< endl;
    
    if(numPs3Eye>0){
        bUsePs3Eye=true;
        ps3eye.setDesiredFrameRate(60);
        ps3eye.initGrabber(camW, camH);
        
        ps3eye.setAutoGainAndShutter(false);
        ps3eye.setGain(camGain);   //longer shutter results in more stable images. less affected by flicker on both utility and monitor. Should set it to 1 as much as possible
        ps3eye.setShutter(camShutter);
        ps3eye.setGamma(0.4);
        ps3eye.setBrightness(0.6);
        ps3eye.setHue(camHue);
        ps3eye.setFlicker(2);/* 0 - no flicker, 1 - 50hz, 2 - 60hz */
        ps3eye.setWhiteBalance(2); /* 1 - linear, 2 - indoor, 3 - outdoor, 4 - auto */
    }else{
        bUsePs3Eye = false;
        ofSetVerticalSync(true);
        ofSetFrameRate(120);
        cam.setDeviceID(0);
        cam.initGrabber(camW, camH);
    }
    
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	ySteps = camH / stepSize;
	xSteps = camW / stepSize;
	for(int y = 0; y < ySteps; y++) {
		for(int x = 0; x < xSteps; x++) {
			mesh.addVertex(ofVec2f(x * stepSize, y * stepSize));
			mesh.addTexCoord(ofVec2f(x * stepSize, y * stepSize));
		}
	}
	for(int y = 0; y + 1 < ySteps; y++) {
		for(int x = 0; x + 1 < xSteps; x++) {
			int nw = y * xSteps + x;
			int ne = nw + 1;
			int sw = nw + xSteps;
			int se = sw + 1;
			mesh.addIndex(nw);
			mesh.addIndex(ne);
			mesh.addIndex(se);
			mesh.addIndex(nw);
			mesh.addIndex(se);
			mesh.addIndex(sw);
		}
	}
    
    faceFbo.allocate(camW*2, camH*2, GL_RGBA);
    faceFbo.begin();
    ofClear(255,255,255,0);
    faceFbo.end();
    
    if(bUseGarrett){
        setUpGarrettGrid();
    }else{
        setUpNormalGrid();
    }
//    SoundBlock::zDown = 0.05;
    
//    Audio stuff
    numPlayers = nCol*nRow;
    colSteps = xSteps / nCol;
    rowSteps = ySteps / nRow;
    
    
    midiIn.listPorts();
    midiIn.openPort(0);
    midiIn.ignoreTypes(false, false, false);
    midiIn.addListener(this);
    midiIn.setVerbose(true);
    
    //osc sender
	sender.setup(HOST, PORT);
}


void testApp::setUpNormalGrid(){
    float deltaX = camW/nCol, deltaY = camH/nRow;
    int i=0;
    for(int iCol=nCol-1; iCol>=0; iCol--){
        for(int iRow=0; iRow<nRow; iRow++){
            float x = deltaX * (iCol+0.5);
            float y = deltaY * (iRow+0.5);
            soundBlocks.push_back(SoundBlock(x, y, targetW, targetH, camW, camH, i, bLoop));
            i++;
        }
    }
    

}

void testApp::setUpGarrettGrid(){
    float deltaX = camW/nCol, deltaY = camH/nRow;
    int i=0;
    float offset = 0;
    int numCol = 0;
    for(int iRow=0; iRow<nRow; iRow++){
        if(iRow%2 == 0){
            offset = deltaX/2.0;
            numCol = nCol - 1;
        }else{
            offset = 0;
            numCol = nCol;
        }
        for(int iCol=numCol-1; iCol>=0; iCol--){
            float x = deltaX * (iCol+0.5) + offset;
            float y = deltaY * (iRow+0.5);
            soundBlocks.push_back(SoundBlock(x, y, targetW, targetH, camW, camH, i, bLoop));
            i++;
        }
    }

}

void testApp::update() {
//    vector<float> maxSpeeds;
//    for(int i=0; i<numPlayers; i++){
//        maxSpeeds.push_back(0);
//    }
    
    bool bNewFrame=false;
    if(bUsePs3Eye){
        ps3eye.update();
        bNewFrame=ps3eye.isFrameNew();
    }else{
        cam.update();
        bNewFrame=cam.isFrameNew();
    }
	if(bNewFrame) {
		flow.setWindowSize(8);
        if(bUsePs3Eye){
            flow.calcOpticalFlow(ps3eye);
        }else{
            flow.calcOpticalFlow(cam);
        }
		int i = 0;
        float sumX=0, sumIntX=0, numActive=0;
        float sumY=0, sumIntY=0, sumSpeed=0;

        float maxSpeed = 0;
        int xAtMaxSpeed = -1;
        int yAtMaxSpeed = -1;
//		float distortionStrength = 4;
		for(int y = 1; y + 1 < ySteps; y++) {
			for(int x = 1; x + 1 < xSteps; x++) {
				int i = y * xSteps + x;
				ofVec2f position(x * stepSize, y * stepSize);
				ofRectangle area(position - ofVec2f(stepSize, stepSize) / 2, stepSize, stepSize);
				ofVec2f offset = flow.getAverageFlowInRegion(area);
//                mesh.setVertex(i, position + distortionStrength * offset);
				ofVec2f current_offset = mesh.getVertex(i) - position;
                ofVec2f new_offset = current_offset * (1-zFactor) + distortionStrength * offset*zFactor;
				mesh.setVertex(i, position + new_offset);
				i++;
                
                
//                audio stuff
                int iCol = floor(x / colSteps*1.0);
                int iRow = floor(y / rowSteps*1.0);
                int idx = iRow*nCol + iCol;
                
//                calculate the center of the movement
                float speed = offset.length();
                if(speed>10.0){  //for near field, may use 15.0
                    int _x=x*stepSize + stepSize/2;
                    int _y=y*stepSize + stepSize/2;
                    sumX+=_x;
                    sumY+=_y;
                    sumIntX+=speed * _x;
                    sumIntY+=speed * _y;
                    numActive++;
                    sumSpeed+=speed;
                    
                    if(speed > maxSpeed){
                        maxSpeed = speed;
                        xAtMaxSpeed = _x;
                        yAtMaxSpeed = _y;
                    }
                }
//                if(speed > maxSpeeds[idx]) {
//                    maxSpeeds[idx] = speed;
//                }
                
//                cout<<iCol<<" "<<iRow<<endl;
			}
		}
        
        if(sumSpeed>0){
            bReportXY = true;
            if(bReportMax){
                reportX = xAtMaxSpeed;
                reportY = yAtMaxSpeed;
            }else{
                reportX = sumIntX/sumSpeed;
                reportY = sumIntY/sumSpeed;
            }
            if(bMirror){
                reportX=camW-reportX;
            }
            cout<<"numActive: "<<numActive<<"     aveX: "<<reportX<<"     aveY: "<<reportY<<"   sumSpeed: "<<sumSpeed<<endl;
            
            oscSendAverageLocation(reportX, reportY);
//            oscSendVideoPosition(sumIntX/sumSpeed);
        }else{
            bReportXY=false;
        }
	}
    
    for(int i=0; i<soundBlocks.size(); i++){
        float volume = soundBlocks[i].update(mesh, stepSize, xSteps, ySteps);
        SoundBlockState currentState = soundBlocks[i].currentState;
        if(currentState == TriggerPlay){
            oscLiveTriggerPlay(i, 0);
            oscLiveSetVolume(i, volume);
            oscRenoiseNoteOn(i, i, 60, 127);
        }else if(currentState == On){
            oscLiveSetVolume(i, volume);
        }else if(currentState == TriggerStop){
            oscLiveTriggerStop(i, 0);
            oscRenoiseNoteOff(i, i, 60, 0);
        }
    }
    
    
}

void testApp::oscLiveTriggerPlay(int track, int clip){
    ofxOscMessage m;
    stringstream ss;
    //            ss<<"/sound/"<<i;
    ss<<"/live/play/clip";
    m.setAddress(ss.str());
    m.addIntArg(track); //track
    m.addIntArg(clip); //clip, only loading one clip for now
    //            m.addFloatArg(volume);
    sender.sendMessage(m);
}

void testApp::oscLiveTriggerStop(int track, int clip){
    ofxOscMessage m;
    stringstream ss;
    //            ss<<"/sound/"<<i;
    ss<<"/live/stop/clip";
    m.setAddress(ss.str());
    m.addIntArg(track); //track
    m.addIntArg(clip); //clip, only loading one clip for now
    //            m.addFloatArg(volume);
    sender.sendMessage(m);
}

void testApp::oscLiveSetVolume(int track, float volume){
    ofxOscMessage m;
    stringstream ss;
    //            ss<<"/sound/"<<i;
    ss<<"/live/volume";
    m.setAddress(ss.str());
    m.addIntArg(track); //track
    m.addFloatArg(volume);
    sender.sendMessage(m);
//    cout<<ss<<" "<<track<<" "<<volume<<endl;
}


void testApp::oscRenoiseNoteOn(int t, int i, int n, int v){
    ofxOscMessage m;
    stringstream ss;
    //            ss<<"/sound/"<<i;
    ss<<"/renoise/trigger/note_on";
    m.setAddress(ss.str());
    m.addIntArg(i); //track
    m.addIntArg(i); //instrument
    m.addIntArg(n);
    m.addIntArg(v);
    sender.sendMessage(m);
}

void testApp::oscRenoiseNoteOff(int t, int i, int n, int v){
    ofxOscMessage m;
    stringstream ss;
    //            ss<<"/sound/"<<i;
    ss<<"/renoise/trigger/note_off";
    m.setAddress(ss.str());
    m.addIntArg(i); //track
    m.addIntArg(i); //instrument
    m.addIntArg(n);
    m.addIntArg(v);
    sender.sendMessage(m);
}

void testApp::oscSendAverageLocation(int x, int y){
    ofxOscMessage m;
    stringstream ss;
    //            ss<<"/sound/"<<i;
    ss<<"/location";
    m.setAddress(ss.str());
    m.addIntArg(x);
    m.addIntArg(y);
    sender.sendMessage(m);
}

void testApp::oscSendVideoPosition(int x){
    ofxOscMessage m;
    stringstream ss;
    //            ss<<"/sound/"<<i;
    cout<<"/video/"<<x<<endl;
    ss<<"/video";
    m.setAddress(ss.str());
    m.addFloatArg(x/200.0);
    sender.sendMessage(m);
}

void testApp::draw() {
    faceFbo.begin();
    ofBackground(0);
	ofScale(2, 2);
    if(bUsePs3Eye){
        ps3eye.getTextureReference().bind();
        mesh.draw();
        ps3eye.getTextureReference().unbind();
    }else{
        cam.getTextureReference().bind();
        mesh.draw();
        cam.getTextureReference().unbind();
    }
	
    if(ofGetMousePressed()) {
		mesh.drawWireframe();
	}
    faceFbo.end();
    
	ofBackground(0);
//	ofScale(2, 2);
//	cam.getTextureReference().bind();
//    
//	mesh.draw();
//	cam.getTextureReference().unbind();
//	if(ofGetMousePressed()) {
//		mesh.drawWireframe();
//	}

//    faceFbo.getTextureReference()
    if(bMirror)
        faceFbo.draw(drawW, 0, -drawW, drawH);
    else
        faceFbo.draw(0, 0, drawW, drawH);

    
    
    ofEnableAlphaBlending();
    ofSetColor(255, 255, 255, 255);
    stringstream reportStream;
    reportStream<<"distortion strength: "<< distortionStrength<<" +/- increase/decrease"<<endl;
    reportStream<<"zFactor: "<<zFactor<<" a/z increase/decrease"<<endl;
    reportStream<<"0: fast enhance"<<endl;
    reportStream<<"1: smooth enhance"<<endl;
    reportStream<<"2: extreme smooth enhance\n";
    reportStream<<"3: reverse smooth enhance\n";
    reportStream<<"4: fast, large enhance\n";
    reportStream<<"m: switch mirror image\n\n";
    reportStream<<"bLoop (cycle) is: "<<(bLoop ? "true" : "false")<<endl<<endl;
    reportStream<<"Sets (<  >):\n";
    reportStream<<"zDown "<<SoundBlock::zDown<<endl;
    if(bReportMax){
        reportStream<<"Reporting max speed loaction, 'x' to toggle\n";
    }else{
        reportStream<<"Reporting average motion location, 'x' to toggle\n";
    }
    if(bReportXY) reportStream<<"Reporting X:"<<reportX<<" Y: "<<reportY<<endl;

    ofDrawBitmapString(reportStream.str(), 810, 20);
    ofDisableAlphaBlending();

    
    for(int i=0; i<soundBlocks.size(); i++){
        soundBlocks[i].draw(bMirror, drawCamRatio);
    }

}



void testApp::keyPressed(int key){
    switch (key) {
        case '=':
            if(SoundBlock::zDown<0.99)
                SoundBlock::zDown+=0.01;
            break;
        case '-':
            if(SoundBlock::zDown>0.01)
                SoundBlock::zDown-=0.01;
            break;
        case 'a':
            if(zFactor<0.99)zFactor+=0.01;
            break;
        case 'z':
            if(zFactor>0.01)zFactor-=0.01;
            break;
        case 'm':
            bMirror= !bMirror;
            break;
        case 'x':
            bReportMax = !bReportMax;
            break;
        case '0':
            distortionStrength = 4.0;
            zFactor = 0.99;
            break;
        case '1':
            distortionStrength = 12.0;
            zFactor = 0.2;
            break;
        case '2':
            distortionStrength=100.0;
            zFactor= 0.01;
            break;
        case '3':
            distortionStrength=-100.0;
            zFactor= 0.01;
            break;
        case '4':
            distortionStrength=10.0;
            zFactor= 0.5;
            break;
        case 'q':
            if(camGain<1.0) camGain+=0.1;
            ps3eye.setGain(camGain);
            cout<<"gain is "<<camGain<<endl;
            break;
        case 'Q':
            if(camGain>0.0) camGain-=0.1;
            ps3eye.setGain(camGain);
            cout<<"camGain is "<<camGain<<endl;
            break;
        case 'w':
            if(camShutter<1.0) camShutter+=0.1;
            ps3eye.setShutter(camShutter);
            cout<<"camShutter is "<<camShutter<<endl;
            break;
        case 'W':
            if(camShutter>0.0) camShutter-=0.1;
            ps3eye.setShutter(camShutter);
            cout<<"camShutter is "<<camShutter<<endl;
            break;
        case 'e':
            if(camHue<1.0) camHue+=0.1;
            ps3eye.setHue(camHue);
            cout<<"camHue is "<<camHue<<endl;
            break;
        case 'E':
            if(camHue>0.0) camHue-=0.1;
            ps3eye.setHue(camHue);
            cout<<"camHue is "<<camHue<<endl;
            break;
            
        default:
            break;
    }
        
}

void testApp::newMidiMessage(ofxMidiMessage &msg){
//    midiMessage = msg;
    cout<<"channel: "<<msg.channel<<" control: "<<msg.control<<" value: "<<msg.value<<endl;
    
    int control = msg.control;
    int value = msg.value;
    
    
}
