//
// Created by MacBook Pro on 11/4/18.
//

#include "flicker_view.h"
#include "ofMain.h"

using namespace ofxBenG;

flicker_view::flicker_view() {

}

flicker_view::~flicker_view() {

}

void flicker_view::draw(ofPoint windowSize) {
    if (isBlackout) {
        ofPushStyle();
        ofSetColor(ofColor::black);
        ofDrawRectangle(0, 0, windowSize[0], windowSize[1]);
        ofPopStyle();
    }
}

void flicker_view::setBlackout(bool enabled) {
    isBlackout = enabled;
}
