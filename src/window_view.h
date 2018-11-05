//
// Created by MacBook Pro on 11/4/18.
//

#ifndef PLUGANDPLAYCAM_WINDOW_VIEW_H
#define PLUGANDPLAYCAM_WINDOW_VIEW_H

#include <ofPoint.h>

namespace ofxBenG {
    class window_view {
    public:
        virtual void draw(ofPoint windowSize) = 0;
    };
}

#endif //PLUGANDPLAYCAM_WINDOW_VIEW_H
