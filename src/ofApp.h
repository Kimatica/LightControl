#pragma once

#include "ofMain.h"
#include "ofxKxsLightFixtures.h"
#include "FixtureBundle.h"
#include "ofxDmx.h"
#include "ofxGui.h"
#include "OscControl.h"
#include "ofxGuiLayout.h"
#include "midiController/MidiController.h"
#include "Grid.h"
#include "ofxPresets.h"

//
// TODO: auto reconnect dmx
//

class ofApp : public ofBaseApp {
public:
    void exit();
    void setup();
    void update();
    void draw();
    void keyReleased(int key);
    
    void initFixtures();
    void initDmx();
    void updateDmx(vector<DmxFixture*> fixtures);
    void initGui();
    void drawDmxScene();
    void drawGridPositions();
    
    ofxDmx dmx;
    
    // need to use vector of pointers to avoid slicing
    // https://stackoverflow.com/questions/2973301/what-is-a-possible-workaround-for-object-slicing-in-c
    // https://www.geeksforgeeks.org/new-and-delete-operators-in-cpp-for-dynamic-memory/
    vector<DmxFixture*> fixtures;
    
    vector<ofxPanel*> panels;
    bool bDrawGui;
    
    ofEasyCam camera;
    
    OscControl oscControl;
    MidiController midi;

    ofParameterGroup presetParameters;

    Grid grid;

    //FixtureBundle bundleBars;
    
    //
    // black out
    bool isBlackOutOn;
    vector<float> dimmerValues;
    void blackoutOn() {
        dimmerValues.resize(fixtures.size());
        for (int i = 0; i < fixtures.size(); i++) {
            dimmerValues[i] = fixtures[i]->dimmer;
            fixtures[i]->dimmer = 0;
        }
    }
    void blackoutOff() {
        dimmerValues.clear();
        for (int i = 0; i < fixtures.size(); i++) {
            fixtures[i]->dimmer = dimmerValues[i];
        }
    }
    // blackout end
     
    ofxGuiLayout panelLayout;

    ofxPanel guiGrid;
    ofxPanel panelGlobal;
    ofParameterGroup groupGlobal;
    ofParameter<float> smoothing;
    ofParameter<bool> useManualControl;

    ofxPresets panelPresets;

    void onSmoothing(float& val) {
        for (auto fixture : fixtures) {
            fixture->smoothing = smoothing;
        }
    }
};
