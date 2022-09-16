//
//  MidiMapper.cpp
//  parameterMidiMapper2
//
//  Created by Nestor Rubio Garcia on 27/02/2015.
//
//

#include "MidiController.h"

//--------------------------------------------------------------
MidiController::MidiController(){
    bSendFeedback = false;
    bMapModeEnabled = false;
    bEnabled = false;
    selected = "";
    numMessagesPerFrame = 0;
    
    for(int ch = 0; ch < NUM_CHANNELS; ch++){
        for(int cc = 0; cc < NUM_CONTROLS; cc++){
            midiMap[ch][cc] = "";
            midiBuckets[ch][cc] = -1;
        }
    }
}
MidiController::~MidiController() {
    midiIn.closePort();
    midiIn.removeListener(this);
    midiOut.closePort();

    for (auto& p : panels)
    {
        delete p;
    }

    panels.clear();
}

//--------------------------------------------------------------
void MidiController::setup(){
    verdana14.loadFont("fonts/stan0755.ttf", 6, false, true);

    std::cout << "In Ports:" << std::endl;
    midiIn.listInPorts();

    std::cout << std::endl;
     
    // open and listen to midifighter an vezer portsx
    midiIn.openPort(0);
    std::cout << "Midi Status: " << midiIn.isOpen() << std::endl;
    midiIn.ignoreTypes(false);
    midiIn.addListener(this);

    midiOut.openPort("Launch Control XL 1");
    
    enable();
}

//--------------------------------------------------------------
void MidiController::addPanel(ofxPanel* panel){
    addGroup(panel);
    panels.push_back(panel);
}


//--------------------------------------------------------------
void MidiController::addGroup(ofxGuiGroup* group){
    for(int i = 0; i < group->getNumControls(); i++){
        ofxBaseGui* control = group->getControl(i);
        
        if(isGroup(control->getParameter())){
            ofxGuiGroup* group = dynamic_cast<ofxGuiGroup*>(control);
            addGroup(group);
        }
        else if(isMappeable(control->getParameter())){
            string controlName = control->getName();
            if(controlsNameCount.find(controlName) == controlsNameCount.end()) {
                // this name hasn't been added yet, init counter
                controlsNameCount[controlName] = 0;
            }else{
                // this name has already been added, increment counter
                controlsNameCount[controlName] += 1;
            }
            // get the count
            int nameCount = controlsNameCount[controlName];
            string uniqueName = controlName + "-" + ofToString(nameCount);
            
            // store control (avoiding duplicated keys)
            controls[uniqueName] = control;
            
            //cout << group->getName() << " >> " << uniqueName << " (" << control->getParameter().type() << ")" << endl;
        }
    }
}


//--------------------------------------------------------------
void MidiController::update(ofEventArgs& args){
//    if(numMessagesPerFrame > 0){
//        cout << "messages per frame " << numMessagesPerFrame << endl;
//    }
    numMessagesPerFrame = 0;
    
    // iterate over the the pending messages for this frame
    for(int channel = 0; channel < NUM_CHANNELS; channel++){
        for(int control = 0; control < NUM_CONTROLS; control++){
            
            // read value from bucket
            int value = midiBuckets[channel][control];
            
            // is bucket filled?
            if(value > -1){
                // cout << "[" << channel << "]" << "[" << control << "] = " << value << endl;
                
                // map mode: create new map entries
                // --------------------------------
                if(bMapModeEnabled && selected != ""){
                    
                    // remove the link if the selected parameter is linked to a cc
                    for(int ch = 0; ch < NUM_CHANNELS; ch++){
                        for(int cc = 0; cc < NUM_CONTROLS; cc++){
                            if(midiMap[ch][cc] == selected){
                                midiMap[ch][cc] = "";
                                break;
                            }
                        }
                    }
                    
                    // make a link
                    midiMap[channel][control] = selected;
                }
                
                // if control is linked to a parameter
                // -----------------------------------
                if(midiMap[channel][control] != ""){
                    
                    string paramName = midiMap[channel][control];
                    ofAbstractParameter& parameter = controls[paramName]->getParameter();
                    
                    if(parameter.type() == typeid(ofParameter<float>).name()){
                        ofParameter<float> & p = parameter.cast<float>();
                        p = ofMap(value, 0, 127, p.getMin(), p.getMax());
                    }
                    else if(parameter.type() == typeid(ofParameter<int>).name()){
                        ofParameter<int> & p = parameter.cast<int>();
                        p = ofMap(value, 0, 127, p.getMin(), p.getMax());
                    }
                    else if(parameter.type() == typeid(ofParameter<bool>).name()){
                        ofParameter<bool> & p = parameter.cast<bool>();
                        p = (value == 127) ? (!p.get()) : p.get(); // 127=pressed, 0=released
                    }
                    else if (parameter.type() == typeid(ofParameter<void>).name()) {
                        ofParameter<void>& p = parameter.cast<void>();
                        if (value == 127) p.trigger(); // 127=pressed, 0=released
                    }
                }
                
                // empty bucket
                midiBuckets[channel][control] = -1;
            }
        }
    }
    
    if(bSendFeedback){
        sendFeedback();
    }
}

//--------------------------------------------------------------
void MidiController::draw(ofEventArgs& args){
    
    if(bMapModeEnabled){
        ofPushStyle();
        //ofSetLineWidth(2);
        
        // draw the rectangles on top of the controls
        for(auto it = controls.begin(); it != controls.end(); ++it){
            string controlName = it->first;
            ofxBaseGui* control = it->second;
            
            if(controlName == selected){
                drawOverlay(control, colorHighlight, ofColor(255));
            }else{
                drawOverlay(control, colorNormal, ofColor(0));
            }
        }
        
        // draw the labels
        for(int ch = 0; ch < NUM_CHANNELS; ch++){
            for(int cc = 0; cc < NUM_CONTROLS; cc++){
                string name = midiMap[ch][cc];
                
                if(name != ""){
                    ofxBaseGui* control = controls[name];
                    ofRectangle r = control->getShape();
                    ofSetColor(255);
                    string text = ofToString(ch+1) + "/" + ofToString(cc);
                    ofRectangle bbox = verdana14.getStringBoundingBox(text, 0, 0);
                    float x = fabs(r.getCenter().x - bbox.width / 2);
                    float y = fabs(r.getCenter().y + bbox.height / 2);
                    verdana14.drawString(text, x, y);
                }
            }
        }
        
        ofPopStyle();
    }
}

//--------------------------------------------------------------
void MidiController::sendFeedback(){
    // only sends values for the knobs
    // in midi fighter knobs are in channel 0
    int ch = 0;
    for(int cc = 0; cc < NUM_CONTROLS; cc++){
        
        // if control is linked to a parameter
        if(midiMap[ch][cc] != ""){
            
            string paramName = midiMap[ch][cc];
            ofAbstractParameter& parameter = controls[paramName]->getParameter();
            
            if(parameter.type() == typeid(ofParameter<float>).name()){
                ofParameter<float>& p = parameter.cast<float>();
                float value = ofMap(p, p.getMin(), p.getMax(), 0, 127);
                
                midiOut.sendControlChange(ch, cc, value);    // change knob value
            }
            else if(parameter.type() == typeid(ofParameter<int>).name()){
                ofParameter<int>& p = parameter.cast<int>();
                int value = ofMap(p, p.getMin(), p.getMax(), 0, 127);
                
                midiOut.sendControlChange(ch, cc, value);    // change knob value
            }
        }
    }
}

//--------------------------------------------------------------
void MidiController::refreshDeviceColors(){
    int ch = 0;
    for(int cc = 0; cc < NUM_CONTROLS; cc++){
        int colorValue = 1;
        if(midiMap[ch][cc] != ""){
            colorValue = 100;
        }
        midiOut.sendControlChange(ch, cc, colorValue);
        midiOut.sendControlChange(ch, cc, 0);
    }
}

//--------------------------------------------------------------
void MidiController::newMidiMessage(ofxMidiMessage &message){
    // we can receibe several params per frame
    numMessagesPerFrame++;
    
    // newer message values will override previous ones
    // note that channel index is 0 based
    midiBuckets[message.channel-1][message.control] = message.value;
}

//--------------------------------------------------------------
void MidiController::mouseReleased(ofMouseEventArgs& mouse){
    selected = "";
    
    for(auto it = controls.begin(); it != controls.end(); ++it){
        string controlName = it->first;
        ofxBaseGui* control = it->second;
        
        if(control->getShape().inside(mouse.x, mouse.y)){
            selected = controlName;
        }
    }
}

//--------------------------------------------------------------
void MidiController::keyPressed(ofKeyEventArgs& args){
    switch(args.key){
        case OF_KEY_BACKSPACE:
            // remove the link if the selected parameter is linked to a cc
            for(int ch = 0; ch < NUM_CHANNELS; ch++){
                for(int cc = 0; cc < NUM_CONTROLS; cc++){
                    if(midiMap[ch][cc] == selected){
                        midiMap[ch][cc] = "";
                        break;
                    }
                }
            }
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void MidiController::enableMappingMode(){
    if(!bEnabled || bMapModeEnabled){
        return;
    }
    
    bMapModeEnabled = true;
    
    for(int i = 0; i < panels.size(); ++i){
        panels[i]->unregisterMouseEvents();
    }
    
    ofAddListener(ofEvents().mouseReleased, this, &MidiController::mouseReleased);
    ofAddListener(ofEvents().keyPressed, this, &MidiController::keyPressed);
}

//--------------------------------------------------------------
void MidiController::disableMappingMode(){
    if(!bEnabled || !bMapModeEnabled){
        return;
    }
    
    bMapModeEnabled = false;
    selected = "";
    
    for(int i = 0; i < panels.size(); ++i){
        panels[i]->registerMouseEvents();
    }
    
    ofRemoveListener(ofEvents().mouseReleased, this, &MidiController::mouseReleased);
    ofRemoveListener(ofEvents().keyPressed, this, &MidiController::keyPressed);
    
    refreshDeviceColors();
}

//--------------------------------------------------------------
void MidiController::toggleMappingMode(){
    //bMapModeEnabled = !bMapModeEnabled;
    if(bMapModeEnabled){
        disableMappingMode();
    }else{
        enableMappingMode();
    }
}

//--------------------------------------------------------------
void MidiController::enable(){
    if(!bEnabled){
        ofAddListener(ofEvents().update, this, &MidiController::update);
        ofAddListener(ofEvents().draw, this, &MidiController::draw);
        
        bEnabled = true;
    }
}

//--------------------------------------------------------------
void MidiController::disable(){
    if(bEnabled){
        ofRemoveListener(ofEvents().update, this, &MidiController::update);
        ofRemoveListener(ofEvents().draw, this, &MidiController::draw);
        
        disableMappingMode();
        
        bEnabled = false;
    }
}

//--------------------------------------------------------------
void MidiController::setFeedbackEnabled(bool enabled){
    bSendFeedback = enabled;
}

//--------------------------------------------------------------
bool MidiController::isFeedbackEnabled(){
    return bSendFeedback;
}

//--------------------------------------------------------------
void MidiController::loadMidiMapping(){
    ofXml XML;
   
    if (!XML.load("midi-map.xml"))
        return;

    auto link = XML.findFirst("//map/link");

    do {
        int ch = link.getChild("ch").getIntValue();
        int cc = link.getChild("cc").getIntValue();
        string controlName = link.getChild("param").getValue();

        midiMap[ch][cc] = controlName;

        // check if we've loaded a valid mapping
        // the parameter names loaded from the xml must match
        // the control names added through addPanel()
        if (controls.find(controlName) == controls.end()) {
            cout << "[ error ] MidiController: loaded control [" << controlName << "] hasen't been added" << endl;
            midiMap[ch][cc] = "";
        }
    } while (link = link.getNextSibling()); // go to the next link
    

    sendFeedback();
    refreshDeviceColors();
}

//--------------------------------------------------------------
void MidiController::saveMidiMapping(){
    ofXml XML;
    auto map = XML.appendChild("map");

    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        for (int cc = 0; cc < NUM_CONTROLS; cc++) {

            if (midiMap[ch][cc] != "") {
                auto link = map.appendChild("link");
                link.appendChild("ch").set(ch);
                link.appendChild("cc").set(cc);
                link.appendChild("param").set(midiMap[ch][cc]);
            }
        }
    }

    XML.save("midi-map.xml");
}

//--------------------------------------------------------------
bool MidiController::isMappeable(ofAbstractParameter & p) {
    // at the moment, there's no way to control multivalue parameters (colors, vectors, etc) with midi
    // a workarround would be exposing the color components (r,g,b) as individual parameters
    // eg: color1_r, color1_g, color1_b and color2_r, color2_g, color2_b
    if(p.type() == typeid(ofParameter<float>).name()
       || p.type() == typeid(ofParameter<int>).name()
       || p.type() == typeid(ofParameter<bool>).name()
       || p.type() == typeid(ofParameter<void>).name()){
        return true;
    }
    return false;
}

//--------------------------------------------------------------
bool MidiController::isGroup(ofAbstractParameter& p){
    if(p.type() == typeid(ofParameterGroup).name()){
        return true;
    }
    return false;
}

//--------------------------------------------------------------
void MidiController::drawOverlay(ofxBaseGui *control, ofColor fillColor, ofColor lineColor){
    ofRectangle r = control->getShape();
    ofFill();
    ofSetColor(fillColor);
    ofRect(r);
    //ofNoFill();
    //ofSetColor(lineColor);
    //ofRect(r);
}