#ifndef monitor_h
#define monitor_h

#include "ofMain.h"
#include "GLFW/glfw3.h"

namespace ofxBenG {

    class monitor_manager;
    class window;
    class monitor {
    public:
        monitor(GLFWmonitor *glfwMonitor, ofxBenG::monitor_manager *monitorManager, int id);
        ~monitor();

        void update();

        int getId();

        ofxBenG::window *getScreen();

        GLFWmonitor *getGlfwMonitor();

        ofPoint getSize();

        ofPoint getPosition();

        std::string toString();

    private:
        GLFWmonitor *glfwMonitor;
        ofxBenG::window *myScreen;
        ofxBenG::monitor_manager *monitorManager;
        std::string name;
        ofPoint size;
        int id;
    };

}

#endif
