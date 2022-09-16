//
//  MidiMapper.h
//  parameterMidiMapper2
//
//  Created by Nestor Rubio Garcia on 27/02/2015.
//
//  Midi Control which allows mapping between parameters and midi cc commands
//
//

#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxPanel.h"

// reduce this values to save memory
// iterating less values is faster
#define NUM_CHANNELS 2
#define NUM_CONTROLS 64 //128


class MidiController : public ofxMidiListener {
    
    public:
    
        MidiController();
        ~MidiController();
        
        void setup();
        void addPanel(ofxPanel* panel);
    
        void enableMappingMode();
        void disableMappingMode();
        void toggleMappingMode();
    
        void saveMidiMapping();
        void loadMidiMapping();
    
        void setFeedbackEnabled(bool enabled);
        bool isFeedbackEnabled();
    
        // updates the knobs on the midi fighter
        // with the current values of the parameters
        void sendFeedback();
    
        void enable();
        void disable();
        bool isEnabled(){ return bEnabled; }
    
        
    private:
    
        bool isGroup(ofAbstractParameter& param);
        void addGroup(ofxGuiGroup* group);
    
        bool bSendFeedback;
        bool bMapModeEnabled;
        bool bEnabled;
        
        vector<ofxPanel*> panels;
        map<string,ofxBaseGui*> controls;   // controls (view of the parameters)
        map<string,int> controlsNameCount;  // count of control names
        string selected;                    // the selected control   
        
        // stores the names of the parameters. paramName = midiMap[ch][cc]
        string midiMap[NUM_CHANNELS][NUM_CONTROLS];
    
        // MIDI messages are received asynchronously
        // a midi message can change a parameter several times per frame
        // messagesPending can be thought as an series of buckets that store the values of midi messages
        // each bucket stores the most recent value for a message, -1 denotes that bucket is empty
        // in every update the buckets are processed and then emptied
        int midiBuckets[NUM_CHANNELS][NUM_CONTROLS];  // stores the value of the receibed "control change" (cc) messages
        int numMessagesPerFrame;                      // debug
    
        //ofxMidiIn midiIn;
        ofxMidiIn midiIn;
        ofxMidiOut midiOut;
        ofxMidiMessage midiMessage;
        
        void newMidiMessage(ofxMidiMessage& message);
    
        void draw(ofEventArgs& args);
        void update(ofEventArgs& args);
        void mouseReleased(ofMouseEventArgs& mouse);
        void keyPressed(ofKeyEventArgs& key);
    
        bool isMappeable(ofAbstractParameter& p);
    
        void refreshDeviceColors();
    
        // drawing helpers
        ofTrueTypeFont verdana14;
        const ofColor colorNormal = ofColor(0, 200, 150, 150);
        const ofColor colorHighlight = ofColor(80, 220, 100, 170);
        void drawOverlay(ofxBaseGui *control, ofColor fillColor, ofColor lineColor);
};
