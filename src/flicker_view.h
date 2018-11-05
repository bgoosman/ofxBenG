//
// Created by MacBook Pro on 11/4/18.
//

#ifndef PLUGANDPLAYCAM_FLICKER_VIEW_H
#define PLUGANDPLAYCAM_FLICKER_VIEW_H

#include "window_view.h"

namespace ofxBenG {
    class flicker_view : public window_view {
    public:
        flicker_view();
        ~flicker_view();
        void draw(ofPoint windowSize);
        void setBlackout(bool enabled);

    private:
        bool isBlackout = false;
    };
}

#endif //PLUGANDPLAYCAM_FLICKER_VIEW_H
