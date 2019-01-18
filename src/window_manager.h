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

        ofxBenG::window *getWindowForMonitor(std::string monitor);

        std::vector<ofxBenG::window *> getWindows();

        bool getFullscreen();

        void setFullscreen(bool);

    private:
        std::vector<ofxBenG::window *> windows;

        bool isFullscreen = true;
    };
}

#endif //PLUGANDPLAYCAM_WINDOW_MANAGER_H
