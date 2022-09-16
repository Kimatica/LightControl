//
//  Grid.h
//  devicesController
//
//  Created by Nestor Rubio Garcia on 29/05/2018.
//

#pragma once

#include "ofMain.h"

using namespace glm;

class Grid {

public:

    Grid();

    void setup(vec2 size, vec2 resolution);

    // pixel manipulation
    void setFrom(ofPixels& pixels, int x = -1, int y = -1, int w = -1, int h = -1); // set pixel colors from a section of other pixels
    void fillWithRandom();
    void fillWithNoise();
    void fillWithColor();
    void fillWithShape();
    void clear();
    void fade();

    ofColor getColorAtPosition(vec2 pos) const;

    void draw();

    // parameters
    ofParameterGroup parameters;
    ofParameter<float> dimmer;
    ofParameter<int> strobe;
    ofParameter<int> clearAmount;
    ofParameter<bool> useRandom;
    ofParameter<bool> useFade;
    ofParameter<bool> useShape;
    ofParameter<bool> clearOnStrobe;
    ofParameter<bool> useNoise;
    ofParameterGroup groupRandom;
    ofParameter<float> randomThresh;
    ofParameterGroup groupNoise;
    ofParameter<float> noiseFreq;
    ofParameter<float> noiseSpeed;
    ofParameterGroup groupShape;
    ofParameter<float> shapeWidth;
    ofParameter<float> shapeSpeed;
    ofParameter<bool> noiseNormalize;
    ofParameterGroup groupTint;
    ofParameter<float> tintR;
    ofParameter<float> tintG;
    ofParameter<float> tintB;

    vec2 position;

private:

    ofPixels pixels;
    vec2 size;          // screen size in pixels
    vec2 resolution;    // cols x rows

    ofFbo shapeFBO;

    ofTexture texture;
};