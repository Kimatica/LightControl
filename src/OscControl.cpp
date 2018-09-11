//
//  OscControl.cpp
//  lightFixtures-osc
//
//  Created by Nestor Rubio Garcia on 26/04/2018.
//

#include "OscControl.h"

OscControl::OscControl() { }

void OscControl::setup(int localPort) {
    this->localPort = localPort;
    receiver.setup(localPort);
}

void OscControl::addParameterGroup(ofParameterGroup *parameters) {
    groups.push_back(parameters);
    
    // TODO:
    // oF 0.8.4 has a bug that does not trigger events on parameterGroups properly
    // this feature will have to be implemented in oF +0.9.0
    //if (sender) {
    //    // put all added groups into a root parameterGroup
    //    // add listener to root parameterGroup to send an osc messages onchange
    //}
}

void OscControl::update() {
    processMessagesIn();
}

void OscControl::processMessagesIn() {
    while(receiver.hasWaitingMessages()){
        ofxOscMessage message;
        receiver.getNextMessage(&message);
        vector<string> address = ofSplitString(message.getAddress(), "/", true);
        ofAbstractParameter * parameter = getGroupByName(address[0]);
        
        //cout << ofToString(address) << endl;
        
        if (parameter) {
            for (unsigned int i = 0; i < address.size(); i++) {
                if (address[i] == parameter->getName()) {  //if (address[i] == parameter->getEscapedName()){
                    if (parameter->type() == typeid(ofParameterGroup).name()) {
                        if (address.size() >= i + 1){
                            //parameter = &parameter->castGroup().get(address[i + 1]);
                            parameter = &static_cast<ofParameterGroup*>(parameter)->get(address[i + 1]);
                        }
                    }
                    else if (parameter->type() == typeid(ofParameter<float>).name() && message.getArgType(0) == OFXOSC_TYPE_FLOAT){
                        parameter->cast<float>() = message.getArgAsFloat(0);
                    }
                    else if (parameter->type() == typeid(ofParameter<int>).name() && message.getArgType(0) == OFXOSC_TYPE_INT32){
                        parameter->cast<int>() = message.getArgAsInt32(0);
                    }
                    // // 4 foat args message to ofFloatColor
                    //                    else if (parameter->type() == typeid(ofParameter<ofFloatColor>).name() && (message.getArgType(0) == OFXOSC_TYPE_FLOAT
                    //                                                                                             && message.getArgType(1) == OFXOSC_TYPE_FLOAT
                    //                                                                                             && message.getArgType(2) == OFXOSC_TYPE_FLOAT
                    //                                                                                             && message.getArgType(3) == OFXOSC_TYPE_FLOAT)){
                    //                        parameter->cast<ofFloatColor>() = ofFloatColor(message.getArgAsFloat(0), message.getArgAsFloat(1), message.getArgAsFloat(2), message.getArgAsFloat(3));
                    //                    }
                    else if (parameter->type() == typeid(ofParameter<bool>).name() && (message.getArgType(0) == OFXOSC_TYPE_INT32
                                                                                       || message.getArgType(0) == OFXOSC_TYPE_FLOAT)){
                        parameter->cast<bool>() = message.getArgAsInt32(0);
                    }
                }
            }
        }
    }
}

void OscControl::sendAllParameters(string remoteHostname, int remotePort) {
    ofxOscSender sender; // temporary sender (only lives in the scope of this function)
    sender.setup(remoteHostname, remotePort);
    for (auto group : groups) {
        sender.sendParameter(*group);
    }
}

ofAbstractParameter* OscControl::getGroupByName(const string &name) {
    for (auto group : groups) {
        if(group->getName() == name) {
            return group;
            //break;
        }
    }
    return NULL;
}

int OscControl::getLocalPort() {
    return localPort;
}

