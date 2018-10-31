#ifndef monitor_h
#define monitor_h

#include "ofMain.h"
#include "GL/glew.h"

namespace ofxBenG {

    class monitor_manager;
    class screen;
    class monitor {
    public:
        monitor(GLFWmonitor *glfwMonitor, ofxBenG::monitor_manager *monitorManager, int id);
        ~monitor();

        void update();

        int getId();

        void setScreen(ofxBenG::screen *screen);

        ofxBenG::screen *getScreen();

        void maximize(ofxBenG::screen *screen);

        void moveToOrigin(ofxBenG::screen *screen);

        void setRemoved();

        void detachScreen();

        GLFWmonitor *getGlfwMonitor();

        ofPoint getSize();

        ofPoint getPosition();

        std::string toString();

    private:
        GLFWmonitor *glfwMonitor;
        ofxBenG::screen *myScreen;
        ofxBenG::monitor_manager *monitorManager;
        std::string name;
        ofPoint size;
        ofPoint position;
        int updateDelayMs = 0;
        int targetUpdateTimeMs;
        int id;
    };

}

#endif