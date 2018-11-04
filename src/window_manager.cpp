//
// Created by MacBook Pro on 11/3/18.
//

#include "window_manager.h"

using namespace ofxBenG;

ofxBenG::window *window_manager::makeWindow(std::shared_ptr<ofAppBaseWindow> parentWindow) {
    auto window = new ofxBenG::window(parentWindow);
    windows.push_back(window);
    return window;
}

ofxBenG::window *window_manager::getWindowWithNoStream() {
    for (auto window : windows) {
        if (window->getStream() == nullptr) {
            return window;
        }
    }
    return nullptr;
}