#include "monitor_manager.h"
#include "monitor.h"

using namespace ofxBenG;

monitor_manager::monitor_manager() {
}

void monitor_manager::update() {
    refreshList();
}

ofxBenG::monitor *monitor_manager::getLastMonitor() {
    return (monitors.size() > 0) ? monitors.back() : nullptr;
}

ofxBenG::monitor *monitor_manager::getUnusedMonitor() {
    monitor* unused = nullptr;
    for (auto monitor : monitors) {
        if (monitor->getScreen() == nullptr) {
            unused = monitor;
            break;
        }
    }
    return unused;
}

void monitor_manager::refreshList() {
    int count;
    GLFWmonitor **glfwMonitors = glfwGetMonitors(&count);
    addNew(glfwMonitors, count);
    removeStale(glfwMonitors, count);
    for (auto monitor : monitors) {
        monitor->update();
    }
}

void monitor_manager::addNew(GLFWmonitor **glfwMonitors, int count) {
    for (int i = 0; i < count; i++) {
        GLFWmonitor *glfwMonitor = glfwMonitors[i];
        std::string name = glfwGetMonitorName(glfwMonitor);
        if (name != "Color LCD") {
            if (std::find_if(monitors.begin(), monitors.end(), [glfwMonitor](ofxBenG::monitor *monitor) {
                return monitor->getGlfwMonitor() == glfwMonitor;
            }) == monitors.end()) {
                auto monitor = new ofxBenG::monitor(glfwMonitor, this, i);
                monitors.push_back(monitor);
                ofNotifyEvent(onMonitorAdded, *monitor);
            }
        }
    }
}

void monitor_manager::removeStale(GLFWmonitor **glfwMonitors, int count) {
    for (auto it = monitors.begin(); it != monitors.end();) {
        if (std::find_if(glfwMonitors, glfwMonitors + count, [it](GLFWmonitor *glfwMonitor) {
            return glfwMonitor == (*it)->getGlfwMonitor();
        }) == glfwMonitors + count) {
            auto monitor = *it;
            ofNotifyEvent(onMonitorRemoved, *monitor);
            delete monitor;
            it = monitors.erase(it);
        } else {
            it++;
        }
    }
}