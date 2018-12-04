#include "monitor.h"
#include "monitor_manager.h"
#include "window.h"

using namespace ofxBenG;

monitor::monitor(GLFWmonitor *glfwMonitor, ofxBenG::monitor_manager* monitorManager, int id) : glfwMonitor(glfwMonitor), monitorManager(monitorManager), id(id) {
    name = glfwGetMonitorName(glfwMonitor);
    size = ofPoint(0, 0);
    myScreen = nullptr;
}

monitor::~monitor() {
    delete myScreen;
    myScreen = nullptr;
    glfwMonitor = NULL;
}

void monitor::update() {
}

int monitor::getId() {
    return id;
}

GLFWmonitor* monitor::getGlfwMonitor() {
    return glfwMonitor;
}

ofPoint monitor::getSize() {
    return size;
}

ofPoint monitor::getPosition() {
    int monitorX, monitorY;
    glfwGetMonitorPos(glfwMonitor, &monitorX, &monitorY);
    return ofPoint(monitorX, monitorY);
}

std::string monitor::toString() {
    return name;
}

std::string monitor::getName() {
    return name;
}
