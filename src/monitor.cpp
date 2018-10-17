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
    if (glfwMonitor != NULL) {
        myScreen = screen;
        int monitorX, monitorY;
        glfwGetMonitorPos(glfwMonitor, &monitorX, &monitorY);
        myScreen->setWindowPosition(monitorX, monitorY);
        const GLFWvidmode *mode = glfwGetVideoMode(glfwMonitor);
        myScreen->setWindowShape(mode->width, mode->height);
        myScreen->setFullscreen(true);
    }
}

ofxBenG::screen *monitor::getScreen() {
    return myScreen;
}

void monitor::setRemoved() {
    glfwMonitor = NULL;
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
