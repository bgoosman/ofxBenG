//
// Created by MacBook Pro on 11/3/18.
//

#ifndef PLUGANDPLAYCAM_WINDOW_MANAGER_H
#define PLUGANDPLAYCAM_WINDOW_MANAGER_H

#include "window.h"

namespace ofxBenG {
    class window_manager {
    public:
        ofxBenG::window *makeWindow(std::shared_ptr<ofAppBaseWindow> parentWindow);

        ofxBenG::window *getWindowWithNoStream();

        std::vector<ofxBenG::window *> getWindows();

    private:
        std::vector<ofxBenG::window *> windows;
    };
}

#endif //PLUGANDPLAYCAM_WINDOW_MANAGER_H
