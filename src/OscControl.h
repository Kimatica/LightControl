//
//  OscControl.hpp
//  lightFixtures-osc
//
//  Created by Nestor Rubio Garcia on 26/04/2018.
//

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"


class OscControl {
    
public:
    OscControl();
    
    void setup(int localPort);
    
    void update();
    
    void addParameterGroup(ofParameterGroup *parameters);
    
    // usefull to expose all available addresses to external apps (eg: osculator)
    void sendAllParameters(string remoteHostNmae, int remotePort);
    
    int getLocalPort();
    
private:
    ofxOscReceiver receiver;
    
    vector<ofAbstractParameter*> groups;
    ofAbstractParameter* getGroupByName(const string &name);
    
    void processMessagesIn();
    
    int localPort;
    string remoteHostname;
    int remotePort;
};

