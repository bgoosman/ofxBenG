#include "monitor_manager.h"
#include "monitor.h"

using namespace ofxBenG;

monitor_manager::monitor_manager() {
}

void monitor_manager::update() {
    refreshList();
}

void monitor_manager::excludeMonitor(std::string monitorName) {
    excludedMonitors.push_back(monitorName);
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
        if (!isMonitorExcluded(name)) {
            if (std::find_if(monitors.begin(), monitors.end(), [glfwMonitor, name](ofxBenG::monitor *monitor) {
                return monitor->getName() == name;
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

bool monitor_manager::isMonitorExcluded(std::string monitorName) {
    for (auto s : excludedMonitors) {
        if (s == monitorName) {
            return true;
        }
    }
    return false;
}