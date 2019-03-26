//
// Created by MacBook Pro on 11/4/18.
//

#ifndef PLUGANDPLAYCAM_WINDOW_VIEW_H
#define PLUGANDPLAYCAM_WINDOW_VIEW_H

#include <ofPoint.h>
#include "ofGraphics.h"

namespace ofxBenG {
    class window_view {
    public:
        virtual void draw(ofPoint windowSize) = 0;
    };

    class single_color_view : public window_view {
    public:
        single_color_view(ofColor color): color(color) {

        }

        virtual void draw(ofPoint size) {
            ofPushStyle();
            ofSetColor(color);
            ofDrawRectangle(0, 0, size[0], size[1]);
            ofPopStyle();
        }

        void setColor(ofColor color) {
            this->color = color;
        }

    private:
        ofColor color;
    };

    typedef std::function<void(ofPoint)> genericDrawFunction;
    class generic_view : public window_view {
    public:
        generic_view(genericDrawFunction drawFunction) : drawFunction(drawFunction) {

        }

        void draw(ofPoint windowSize) {
            drawFunction(windowSize);
        }

    private:
        genericDrawFunction drawFunction;
    };
}

#endif //PLUGANDPLAYCAM_WINDOW_VIEW_H
