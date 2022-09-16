#include "ofApp.h"

// TODO: move out ///////
bool sortFixtureByAddress(DmxFixture* a, DmxFixture* b){
    if (a->getAddress() < b->getAddress()){
        return true;
    } else {
        return false;
    }
}
/////////////////////////

void drawGroundPlane() {
    ofPushMatrix();
    {
        ofTranslate(0, -0.01, 0);
        ofRotateZ(90);
        ofScale(15,15,15);
        
        ofPushStyle();
        ofSetColor(50);
        ofDrawGridPlane();
        ofPopStyle();
    }
    ofPopMatrix();
}


void ofApp::exit() {
    panelLayout.saveLayout("./layout.xml");
    panelPresets.save();
    midi.saveMidiMapping();
}

void ofApp::setup() {
    ofSetVerticalSync(true);
    ofSetFrameRate(30);
    ofBackground(0);
    
    camera.setDistance(400);

    // init grid
    vec2 size(800, 200);
    vec2 resolution(80, 20);
    grid.setup(size, resolution);
    
    initFixtures();
    initDmx();
    initGui();
    
    // osc
    oscControl.setup(7000);
    for (auto fixture : fixtures) {
        // hacky way to avoid adding bars parameters (we'll add the parameters of the bundle)
        //if (ofSplitString(fixture->getName(),"_")[0] != "bar") {
            oscControl.addParameterGroup(&fixture->parameters);
        //}
    }
    oscControl.addParameterGroup(&groupGlobal);
    
    isBlackOutOn = false;
}

void ofApp::initFixtures() {
    ofMatrix4x4 lookat;
    ofMatrix4x4 scale = ofMatrix4x4::newScaleMatrix(ofVec3f(1,1,-1));

    int stepX = 800 / 8;
    int x = stepX / 2;
    int y = 200 / 2;

    //Light Cans
    for (int i = 0; i < 8; i++)
    {
        ApeLabsLightCan* lightCanGroup1 = new ApeLabsLightCan();
        lightCanGroup1->setup((i*4) + 1, "LightCan" + ofToString(i + 1));
        lightCanGroup1->setGridPosition({ x + i * stepX, y });
        lightCanGroup1->setTransform({ static_cast<float>(i) * 50, 0 }, 0, {0,0,0});
        fixtures.push_back(lightCanGroup1);
    }

    //Floor Bat
    ShowtecLedBar16* lightBar = new ShowtecLedBar16();
    lightBar->setup(40, "bar");
    lightBar->setGridPosition({400, 180});
    lightBar->setTransform({ 200, 50 }, 0, { 0,0,0 });
    fixtures.push_back(lightBar);
}

void ofApp::initDmx() {
    int totalChannels = 0;
    for (auto fixture : fixtures) {
        //cout << fixture->getNumChannels() << endl;
        totalChannels += fixture->getNumChannels();
    }

    cout << totalChannels << endl;
    
//    dmx.connect("tty.usbserial-EN129635", totalChannels);
    dmx.connect(0, 300);
}

void ofApp::initGui() {

    // start the midi
    midi.setup();

    guiGrid.setup();

    // create fixture panels
    for (auto fixture : fixtures) {
        ofxPanel* panel = new ofxPanel();
        panel->setup();
        panel->setName(fixture->getName());
        panel->add(fixture->parameters);
        panels.push_back(panel);
        
        panelLayout.addPanel(panel);
    }

    presetParameters.setName("Visual");
    presetParameters.add(grid.parameters);
    guiGrid.add(presetParameters);

    panelLayout.addPanel(&guiGrid);

    //preset
    panelPresets.setup("presets", 10);
    panelPresets.setParameters(&presetParameters);
    panelPresets.load();
       
    // global
    smoothing.set("smoothing", 0.99, 0, 1);
    smoothing.addListener(this, &ofApp::onSmoothing);
    
    useManualControl.set("Manual / Visual", true);

    groupGlobal.setName("global");
    groupGlobal.add(smoothing);
    groupGlobal.add(useManualControl);
    
    panelGlobal.setup();
    panelGlobal.setName("global");
    panelGlobal.add(groupGlobal);
    
    // layout
    panelLayout.addPanel(&panelGlobal);
    panelLayout.addPanel(panelPresets.getPanel());
    panelLayout.loadLayout("layout.xml");
  
    // add the guis you want to be controlled
    midi.addPanel(&panelGlobal);
    midi.addPanel(panelPresets.getPanel());
    midi.addPanel(&guiGrid);

    for (auto& panel : panels)
    {
        midi.addPanel(panel);
    }

    midi.loadMidiMapping();
    midi.setFeedbackEnabled(true);
    
    bDrawGui = true;
}

void ofApp::update() {
    oscControl.update();

    // update grid
    if (!useManualControl)
    {
        grid.fade();

        if (grid.useNoise)
            grid.fillWithNoise();
        
        else if (grid.useShape)
            grid.fillWithShape();

        else if (grid.useRandom)
            grid.fillWithRandom();
    }
    
   /* if (dmx.isConnected())
    {*/
        // dmx
        updateDmx(fixtures);

        dmx.update();
    //}
}


void ofApp::updateDmx(vector<DmxFixture*> fixtures) {
    for (auto fixture : fixtures) {

        if (useManualControl)
            fixture->update();
        else
            fixture->update(grid);
        
        int channel = fixture->getAddress();
        auto channelsValues = fixture->getChannels();
        
        for (int value : channelsValues) {
            dmx.setLevel(channel, value);
            channel++;
        }
    }
}


void ofApp::draw() {

    ofPushMatrix();
    {
        ofTranslate(15, ofGetHeight() - 200);

        ofSetColor(dmx.isConnected() ? ofColor::green : ofColor::red);
        ofDrawBitmapString("dmx", 0, 0);
        ofDrawCircle(35, -4, 5);

        ofSetColor(midi.isEnabled() ? ofColor::green : ofColor::red);
        ofDrawBitmapString("midi", 0, 20);
        ofDrawCircle(35, 20 - 4, 5);

        ofSetColor(180);

        // dmx info`
        stringstream s;
        for (auto fixture : fixtures) {
            s << fixture->getName() << ": " << fixture->getAddress() << " (" << fixture->getNumChannels() << "ch)" << std::endl;
        }

        ofDrawBitmapString(s.str(), 0, 40);
    }
    ofPopMatrix();

    if (!useManualControl)
        grid.draw();
    
    drawDmxScene();

    drawGridPositions();

    for (auto panel : panels) {
        panel->draw();
    }

    guiGrid.draw();
    panelPresets.draw();
    
    panelGlobal.draw();
    
    ofDrawBitmapString("osc port in: " + ofToString(oscControl.getLocalPort()), ofGetWidth() - 150, 25);
}

void ofApp::drawDmxScene() {
    ofPushMatrix();
    {
        ofTranslate(ofGetWidth()/2 - 300, ofGetHeight() - 200);

        for (auto fixture : fixtures) {
            fixture->draw();
        }
    }
    ofPopMatrix();
}


void ofApp::drawGridPositions() {
    ofPushStyle();
    {
        ofSetColor(ofColor::red);

        for (auto fixture : fixtures) {
            ofDrawRectangle(fixture->getGridPosition(), 5, 5);
        }
    }
    ofPopStyle();
}


void ofApp::keyReleased(int key) {
    switch (key) {
        case 'f':
            ofToggleFullscreen();
            break;
        case 'g':
            bDrawGui = !bDrawGui;
            break;
        case 'b':
            isBlackOutOn = !isBlackOutOn;
            {
                if (isBlackOutOn) blackoutOn();
                else              blackoutOff();
            }
            break;
        case 'm':
        case 'M':
            midi.toggleMappingMode();
            break;
        case 's':
            midi.saveMidiMapping();
            panelLayout.saveLayout("./layout.xml");
            panelPresets.save();
            break;
        case 'x':
            if (midi.isEnabled()) {
                midi.disable();
            }
            else {
                midi.enable();
            }
            break;
        default:
            break;
    }
}
