#ifndef monitor_manager_h
#define monitor_manager_h

#include "ofConstants.h"
#include "ofEvent.h"
#include "GL/glew.h"
#include <GLFW/glfw3.h>

namespace ofxBenG {

    class monitor;
    class monitor_manager {
    public:
        monitor_manager();

        void update();

        void excludeMonitor(std::string);

        ofEvent<ofxBenG::monitor> onMonitorAdded;
        ofEvent<ofxBenG::monitor> onMonitorRemoved;

    private:
        void refreshList();

        void addNew(GLFWmonitor **glfwMonitors, int count);

        void removeStale(GLFWmonitor **glfwMonitors, int count);

        bool isMonitorExcluded(std::string);

        std::vector<ofxBenG::monitor *> monitors;

        std::vector<std::string> excludedMonitors;
    };

}

#endif
