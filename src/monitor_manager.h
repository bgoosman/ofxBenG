#ifndef monitor_manager_h
#define monitor_manager_h

#include <ofEvent.h>
#include <ofEventUtils.h>
#include "GL/glew.h"
#include <GLFW/glfw3.h>

namespace ofxBenG {

    class monitor;
    class monitor_manager {
    public:
        monitor_manager();

        void update();

        ofxBenG::monitor *getLastMonitor();

        ofxBenG::monitor *getUnusedMonitor();

        ofEvent<ofxBenG::monitor> onMonitorAdded;
        ofEvent<ofxBenG::monitor> onMonitorRemoved;

    private:
        void refreshList();

        void addNew(GLFWmonitor **glfwMonitors, int count);

        void removeStale(GLFWmonitor **glfwMonitors, int count);

        std::vector<ofxBenG::monitor *> monitors;
    };

}

#endif
