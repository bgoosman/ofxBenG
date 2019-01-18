//
// Created by MacBook Pro on 11/3/18.
//

#include "window_manager.h"

using namespace ofxBenG;

ofxBenG::window *window_manager::makeWindow(std::shared_ptr<ofAppBaseWindow> parentWindow) {
    auto window = new ofxBenG::window(parentWindow, isFullscreen);
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

ofxBenG::window *window_manager::getWindowForMonitor(std::string monitor) {
    for (auto window : windows) {
        if (window->getMonitorName().find(monitor) != std::string::npos) {
            return window;
        }
    }
    return nullptr;
}

std::vector<ofxBenG::window*> window_manager::getWindows() {
    return windows;
}

bool window_manager::getFullscreen() {
    return isFullscreen;
}

void window_manager::setFullscreen(bool isFullscreen) {
    this->isFullscreen = isFullscreen;
}