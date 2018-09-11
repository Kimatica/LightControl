//
//  FixtureBundle.hpp
//  Created by Nestor Rubio Garcia on 02/05/2018.
//
//
// Group fixtures of the same class to be controlled at the same time.
// Expose the common paramters, any changes will apply to all the objects in this bundle
//
//
// TODO: check that class of fixtures added is the same. This option will expose all parameters
// TODO: allow different classes? This option will only expose for simillar parameters (same name and type)

#pragma once

#include "ofMain.h"
#include "ofxKxsLightFixtures.h"


class FixtureBundle {
    
public:
    ofParameterGroup parameters;
    
    FixtureBundle() {
        bParametersCollected = false;
        parameters.setName("bundle");
    }
    
    void setName(string name) {
        parameters.setName("bundle_" + name);
    }
    
    // atm only bars...
//    void addFixture(ShowtecLedBar16* bar) {
//        // extract parameters from fixture
//        if (!bParametersCollected) {
//            for (auto& param : bar->parameters) {
//                const auto& srcParam = param->cast<float>();
//                ofParameter<float> parameter(srcParam);
//                parameters.add(parameter);
//            }
//            ofAddListener(parameters.parameterChangedE(), this, &FixtureBundle::onParameterChange);
//            bParametersCollected = true;
//        }
//        bars.push_back(bar);
//    }
    
    void addFixture(Dimmer* dimmer) {
        // extract parameters from fixture
        if (!bParametersCollected) {
            for (auto& param : dimmer->parameters) {
                const auto& srcParam = param->cast<float>();
                ofParameter<float> parameter(srcParam);
                parameters.add(parameter);
            }
            ofAddListener(parameters.parameterChangedE(), this, &FixtureBundle::onParameterChange);
            bParametersCollected = true;
        }
        dimmers.push_back(dimmer);
    }
    
private:
    //vector<ShowtecLedBar16*> bars;
    vector<Dimmer*> dimmers;
    
    bool bParametersCollected;
    
//    void onParameterChange(ofAbstractParameter &e) {
//        const string& paramName = e.getName();
//        for (auto bar : bars) {
//            bar->parameters.get(paramName).cast<float>() = e.cast<float>().get();
//       }
//    }
    
    void onParameterChange(ofAbstractParameter &e) {
        const string& paramName = e.getName();
        for (auto dimmer : dimmers) {
            dimmer->parameters.get(paramName).cast<float>() = e.cast<float>().get();
        }
    }
};
