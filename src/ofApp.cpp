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
    panelLayout.saveLayout("layout.xml");
}

void ofApp::setup() {
    ofSetVerticalSync(true);
    ofSetFrameRate(30);
    ofBackground(0);
    
    camera.setDistance(400);
    
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
    oscControl.addParameterGroup(&groupHouseLights);
    
    isBlackOutOn = false;
}

void ofApp::initFixtures() {
    ofMatrix4x4 lookat;
    ofMatrix4x4 scale = ofMatrix4x4::newScaleMatrix(ofVec3f(1,1,-1));
    
    /////////////////////////
    // ceiling
    
    ShowtecLedBar16* lightBar;
    AdjTriparProfile* tripar;
    AdjTriparProfilePlus* triparPlus;
    PrashPar* prashPar;
    Dimmer* dimmer;
    
    // top back
    prashPar = new PrashPar();
    prashPar->setup(201, "top_front");
    prashPar->green = 1;
    prashPar->dimmer = 0.3;
    fixtures.push_back(prashPar);
    
    // top left
    prashPar = new PrashPar();
    prashPar->setup(211, "top_left");
    prashPar->green = 1;
    prashPar->dimmer = 0.3;
    fixtures.push_back(prashPar);
    
    // top right
    prashPar = new PrashPar();
    prashPar->setup(221, "top_right");
    prashPar->green = 1;
    prashPar->dimmer = 0.3;
    fixtures.push_back(prashPar);
    
    // top center
    triparPlus = new AdjTriparProfilePlus();
    triparPlus->setup(231, "top_center");
    triparPlus->green = 1;
    triparPlus->dimmer = 0.3;
    fixtures.push_back(triparPlus);
    
    // top back
    triparPlus = new AdjTriparProfilePlus();
    triparPlus->setup(237, "top_back");
    triparPlus->green = 1;
    triparPlus->dimmer = 0.3;
    fixtures.push_back(triparPlus);
    
   
    /////////////////////////
    // floor
    
    // bars
    lightBar = new ShowtecLedBar16();
    lightBar->setup(275, "bar");
    lightBar->setTransform(ofVec3f(-60,0,-100), -90, ofVec3f(1,0,0));
    lightBar->green = 1;
    lightBar->dimmer = 0.3;
    fixtures.push_back(lightBar);
    
    // right back
    prashPar = new PrashPar();
    prashPar->setup(255, "right_back");
    prashPar->green = 1;
    prashPar->dimmer = 0.3;
    fixtures.push_back(prashPar);
    
    // left front
//    tripar = new AdjTriparProfile();
//    tripar->setup(243, "left_front");
//    tripar->green = 1;
//    tripar->dimmer = 0.3;
//    fixtures.push_back(tripar);
    
    // left back
    prashPar = new PrashPar();
    prashPar->setup(265, "left_back");
    prashPar->green = 1;
    prashPar->dimmer = 0.3;
    fixtures.push_back(prashPar);
    
    // order fixtures by address
    ofSort(fixtures, &sortFixtureByAddress);
    
    /////////////////////////
    // house
    
    dimmer = new Dimmer();
    dimmer->setup(1, "house_lights");
    fixturesHouse.push_back(dimmer);
    
    dimmer = new Dimmer();
    dimmer->setup(2, "house_lights_flood");
    fixturesHouse.push_back(dimmer);
    
    dimmer = new Dimmer();
    dimmer->setup(2, "spot_light_1");
    fixturesSpots.push_back(dimmer);
    
    dimmer = new Dimmer();
    dimmer->setup(2, "spot_light_2");
    fixturesSpots.push_back(dimmer);
}

void ofApp::initDmx() {
//    int totalChannels = 0;
//    for (auto fixture : fixtures) {
//        //cout << fixture->getNumChannels() << endl;
//        totalChannels += fixture->getNumChannels();
//    }
//
//    cout << totalChannels << endl;
    
//    dmx.connect("tty.usbserial-EN129635", totalChannels);
    dmx.connect("tty.usbserial-EN129635", 300);
}

void ofApp::initGui() {
    // create fixture panels
    for (auto fixture : fixtures) {
        ofxPanel* panel = new ofxPanel();
        panel->setup();
        panel->setName(fixture->getName());
        panel->add(fixture->parameters);
        panels.push_back(panel);
        
        panelLayout.addPanel(panel);
    }
    
    // lights public
    houseLightsMaster.set("house_lights", 0, 0, 1);
    smoothing.addListener(this, &ofApp::onHouseLights);
    
    spotLightsMaster.set("spot_lights", 0, 0, 1);
    spotLightsMaster.addListener(this, &ofApp::onSpotLights);
    
    groupHouseLights.setName("lights_public");
    groupHouseLights.add(houseLightsMaster);
    groupHouseLights.add(spotLightsMaster);
    
    panelHouseLights.setup();
    panelHouseLights.setName("lights_public");
    panelHouseLights.add(groupHouseLights);
    
    // global
    smoothing.set("smoothing", 1, 0, 1);
    smoothing.addListener(this, &ofApp::onSmoothing);
    
    groupGlobal.setName("global");
    groupGlobal.add(smoothing);
    
    panelGlobal.setup();
    panelGlobal.setName("global");
    panelGlobal.add(groupGlobal);
    
    // layout
    panelLayout.addPanel(&panelHouseLights);
    panelLayout.addPanel(&panelGlobal);
    panelLayout.loadLayout("layout.xml");
    
    bDrawGui = true;
}

void ofApp::update() {
    oscControl.update();
    
    // dmx
    updateDmx(fixtures);
    updateDmx(fixturesHouse);
    updateDmx(fixturesSpots);
    
    dmx.update();
}


void ofApp::updateDmx(vector<DmxFixture*> fixtures) {
    for (auto fixture : fixtures) {
        fixture->update();
        
        int channel = fixture->getAddress();
        auto channelsValues = fixture->getChannels();
        
        for (int value : channelsValues) {
            dmx.setLevel(channel, value);
            channel++;
        }
    }
}


void ofApp::draw() {
//    ofEnableDepthTest();
//    camera.begin();
//    {
//        ofDrawAxis(10);
//        drawGroundPlane();
//        drawDmxScene();
//    }
//    camera.end();
//    ofDisableDepthTest();
    
    ofSetColor(dmx.isConnected() ? ofColor::green : ofColor::red);
    ofDrawBitmapString("dmx", 15, 25);
    ofDrawCircle(15+35, 25-4, 5);
    ofSetColor(180);
    
    // dmx info
    stringstream s;
    for (auto fixture : fixtures) {
        s << fixture->getName() << ": " << fixture->getAddress() << " (" << fixture->getNumChannels() << "ch)" <<endl;
    }
    for (auto fixture : fixturesHouse) {
        s << fixture->getName() << ": " << fixture->getAddress() << " (" << fixture->getNumChannels() << "ch)" <<endl;
    }
    for (auto fixture : fixturesSpots) {
        s << fixture->getName() << ": " << fixture->getAddress() << " (" << fixture->getNumChannels() << "ch)" <<endl;
    }
    ofDrawBitmapString(s.str(), 15, 45);
    
    // gui
    if (bDrawGui) {
        for (auto panel : panels) {
            panel->draw();
        }
    }
    panelGlobal.draw();
    panelHouseLights.draw();
    
    ofDrawBitmapString("osc port in: " + ofToString(oscControl.getLocalPort()), ofGetWidth() - 150, 25);
}

void ofApp::drawDmxScene() {
    for (auto fixture : fixtures) {
        fixture->draw();
    }
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
        default:
            break;
    }
}
