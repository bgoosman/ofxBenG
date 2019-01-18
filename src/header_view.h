//
// Created by MacBook Pro on 11/4/18.
//

#ifndef PLUGANDPLAYCAM_HEADER_VIEW_H
#define PLUGANDPLAYCAM_HEADER_VIEW_H

#include <ofPoint.h>
#include "BasicVideoRenderer.h"
#include "VideoHeader.h"
#include "window_view.h"

namespace ofxBenG {
    class header_view : public window_view {
    public:
        header_view(ofxPm::VideoHeader *header);
        ~header_view();
        void draw(ofPoint windowSize);
        void draw(float x, float y, float w, float h);

    private:
        ofxPm::BasicVideoRenderer *renderer;
    };
}

#endif //PLUGANDPLAYCAM_HEADER_VIEW_H
