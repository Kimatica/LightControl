//
//  Grid.cpp
//  devicesController
//
//  Created by Nestor Rubio Garcia on 29/05/2018.
//

#include "Grid.h"

using namespace glm;

Grid::Grid() {
    parameters.setName("grid");
    parameters.add(dimmer.set("dimmer", 1, 0, 1));
    parameters.add(clearAmount.set("clear_amount", 255, 0, 255));
    parameters.add(useFade.set("useFade", false));
    parameters.add(useRandom.set("useRandom", false));
    parameters.add(useShape.set("useShape", false));
    parameters.add(useNoise.set("useNoise", false));
    parameters.add(strobe.set("strobe", 1, 1, 30));
    parameters.add(clearOnStrobe.set("clear_on_strobe", false));

    {
        groupRandom.setName("random");
        groupRandom.add(randomThresh.set("threshold", 1, 0, 1));
    }
    parameters.add(groupRandom);

    {
        groupNoise.setName("noise");
        groupNoise.add(noiseFreq.set("freq", 1, 0, 50));
        groupNoise.add(noiseSpeed.set("speed", 1, 0, 5));
    }
    parameters.add(groupNoise);

    {
        groupShape.setName("shape");
        groupShape.add(shapeWidth.set("width", 1, 0, 50));
        groupShape.add(shapeSpeed.set("speed", 15, 5, 200));
    }
    parameters.add(groupShape);

    {
        groupTint.setName("tint");
        groupTint.add(tintR.set("r", 1, 0, 1));
        groupTint.add(tintG.set("g", 1, 0, 1));
        groupTint.add(tintB.set("b", 1, 0, 1));
    }
    parameters.add(groupTint);

    position.x = 0;
    position.y = 0;
}


void Grid::setup(vec2 size, vec2 resolution) {
    this->size = size;
    this->resolution = resolution;

    pixels.allocate(resolution.x, resolution.y, OF_PIXELS_RGB);
    shapeFBO.allocate(resolution.x, resolution.y, GL_RGB);

    clear(); // start black

    // drawing
    texture.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);

    shapeWidth.setMax(resolution.x);
}

void Grid::clear() {
    for (auto line : pixels.getLines()) {
        for (auto pixel : line.getPixels()) {
            pixel[0] = 0;
            pixel[1] = 0;
            pixel[2] = 0;
        }
    }
}

void Grid::fade() {
    float clearAmountNorm = 1.f - float(clearAmount) / clearAmount.getMax();
    for (auto line : pixels.getLines()) {
        for (auto pixel : line.getPixels()) {
            int r, g, b;
            if (useFade) {
                r = pixel[0] * clearAmountNorm;
                g = pixel[1] * clearAmountNorm;
                b = pixel[2] * clearAmountNorm;
            }
            else {
                r = pixel[0] - clearAmount;
                g = pixel[1] - clearAmount;
                b = pixel[2] - clearAmount;
            }
            // clamp
            if (r < 0) r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;

            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
        }
    }
}

void Grid::setFrom(ofPixels& pixelsSrc, int _x, int _y, int _w, int _h) {
    return;

    int srcX = _x > -1 ? _x : 0;
    int srcY = _y > -1 ? _y : 0;
    //    int w = _w > -1 ? _w : pixels.getWidth();
    //    int h = _h > -1 ? _h : pixels.getHeight();
    int srcRows = pixelsSrc.getHeight();
    int srcCols = pixelsSrc.getWidth();
    int dstRows = pixels.getHeight();
    int dstCols = pixels.getWidth();
    for (int srcRow = srcY, dstRow = 0; dstRow < dstRows; srcRow++, dstRow++) {
        auto rowSrc = pixelsSrc.getLine(srcRow);
        auto rowDst = pixels.getLine(dstRow);
        for (int srcCol = srcX, dstCol = 0; dstCol < dstCols; srcCol++, dstCol++) {
            auto src = rowSrc.getPixel(srcCol);
            auto dst = rowDst.getPixel(dstCol);
            dst[0] = src[0]; // r
            dst[1] = src[1]; // g
            dst[2] = src[2]; // b
        }
    }
}


void Grid::fillWithRandom() {
    if (ofGetFrameNum() % strobe == 0) {
        if (clearOnStrobe) {
            clear();
        }
        for (auto line : pixels.getLines()) {
            for (auto pixel : line.getPixels()) {
                if (ofRandomuf() < randomThresh) {
                    pixel[0] = (tintR * dimmer) * 255.f;
                    pixel[1] = (tintG * dimmer) * 255.f;
                    pixel[2] = (tintB * dimmer) * 255.f;
                }
            }
        }
    }
}


void Grid::fillWithNoise() {
    if (ofGetFrameNum() % strobe == 0) {
        if (clearOnStrobe) {
            clear();
        }

        float speed = ofGetElapsedTimef() * noiseSpeed;

        int numRows = pixels.getHeight();
        int numCols = pixels.getWidth();
        for (int row = 0; row < numRows; row++) {
            auto line = pixels.getLine(row);
            for (int col = 0; col < numCols; col++) {
                float noiseValue = glm::perlin(glm::vec3(col / noiseFreq, row / noiseFreq, speed));
                if (noiseNormalize) {
                    noiseValue = 0.5f + noiseValue * 0.5f;
                }
                else {
                    noiseValue = (noiseValue < 0.f) ? 0.f : noiseValue;
                }
                auto pixel = line.getPixel(col);
                pixel[0] = (noiseValue * tintR * dimmer) * 255.f;
                pixel[1] = (noiseValue * tintG * dimmer) * 255.f;
                pixel[2] = (noiseValue * tintB * dimmer) * 255.f;
            }
        }
    }
}


void Grid::fillWithColor() {
    if (ofGetFrameNum() % strobe == 0) {
        for (auto line : pixels.getLines()) {
            for (auto pixel : line.getPixels()) {
                pixel[0] = (tintR * dimmer) * 255.f;
                pixel[1] = (tintG * dimmer) * 255.f;
                pixel[2] = (tintB * dimmer) * 255.f;
            }
        }
    }
}

void Grid::fillWithShape() {

    auto speed = ofGetElapsedTimef() * shapeSpeed - shapeWidth / 2;

    if (speed > resolution.x - shapeWidth / 2)
        ofResetElapsedTimeCounter();

    if (ofGetFrameNum() % strobe == 0) {

        if (clearOnStrobe) {
            clear();
        }

        shapeFBO.begin();
        {
            ofClear(0);
            ofSetColor((tintR * dimmer) * 255.f, (tintG * dimmer) * 255.f, (tintB * dimmer) * 255.f);
            ofDrawRectangle(speed, 0, shapeWidth, resolution.y);
        }
        shapeFBO.end();

        shapeFBO.readToPixels(pixels);
    }   
}


ofColor Grid::getColorAtPosition(vec2 pos) const {
    vec2 localPos(pos - this->position);
    vec2 cell(localPos / size * resolution);
    //if(cell.x > 0 && cell.x < resolution.x-1 && cell.y > 0 && cell.y < resolution.y-1) {
    int index = (size_t(cell.x) + size_t(cell.y) * size_t(resolution.x)) * 3; // 3 = pixelStride

    return pixels.getColor(index);
    //}
    //return pixels.getColor(cell.x, cell.y);
}


void Grid::draw() {
    texture.loadData(pixels);
    texture.draw(position, size.x, size.y);
}