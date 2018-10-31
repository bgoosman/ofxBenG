#include "monitor.h"
#include "monitor_manager.h"
#include "screen.h"

using namespace ofxBenG;

monitor::monitor(GLFWmonitor *glfwMonitor, ofxBenG::monitor_manager* monitorManager, int id) : glfwMonitor(glfwMonitor), monitorManager(monitorManager), id(id) {
    name = glfwGetMonitorName(glfwMonitor);
    targetUpdateTimeMs = ofGetElapsedTimeMillis() + updateDelayMs;
    size = ofPoint(0, 0);
    position = ofPoint(0, 0);
    myScreen = nullptr;
}

monitor::~monitor() {
    delete myScreen;
    myScreen = nullptr;
    glfwMonitor = NULL;
}

void monitor::update() {
    int now = ofGetElapsedTimeMillis();
    if (now - targetUpdateTimeMs > updateDelayMs) {
        targetUpdateTimeMs = now + updateDelayMs;
        if (glfwMonitor != NULL) {
            int x, y;
            glfwGetMonitorPos(glfwMonitor, &x, &y);
            position = ofPoint(x, y);
        }
        if (glfwMonitor != NULL) {
//            const GLFWvidmode *mode = glfwGetVideoMode(glfwMonitor);
//            if (mode != NULL) {
//                size = ofPoint(mode->width, mode->height);
//            }
        }
    }
}

int monitor::getId() {
    return id;
}

void monitor::setScreen(ofxBenG::screen* screen) {
    myScreen = screen;
    if (myScreen != nullptr) {
        moveToOrigin(myScreen);
        maximize(myScreen);
    }
}

void monitor::maximize(ofxBenG::screen *screen) {
    const GLFWvidmode *mode = glfwGetVideoMode(this->glfwMonitor);
    this->myScreen->setWindowShape(mode->width, mode->height);
    this->myScreen->setFullscreen(true);
}

void monitor::moveToOrigin(ofxBenG::screen *screen) {
    int monitorX, monitorY;
    glfwGetMonitorPos(glfwMonitor, &monitorX, &monitorY);
    myScreen->setWindowPosition(monitorX, monitorY);
}

ofxBenG::screen *monitor::getScreen() {
    return myScreen;
}

void monitor::setRemoved() {
    glfwMonitor = NULL;
}

void monitor::detachScreen() {
    myScreen = nullptr;
}

GLFWmonitor* monitor::getGlfwMonitor() {
    return glfwMonitor;
}

ofPoint monitor::getSize() {
    return size;
}

ofPoint monitor::getPosition() {
    return position;
}

std::string monitor::toString() {
    return name;
}
