//
// Created by MacBook Pro on 11/4/18.
//

#include "header_view.h"

using namespace ofxBenG;

header_view::header_view(ofxPm::VideoHeader *header) {
    renderer = new ofxPm::BasicVideoRenderer;
    renderer->setup(*header);
}

header_view::~header_view() {
    delete renderer;
}

void header_view::draw(ofPoint windowSize) {
    renderer->draw(0, 0, windowSize[0], windowSize[1]);
}

void header_view::draw(float x, float y, float w, float h) {
    renderer->draw(x, y, w, h);
}
