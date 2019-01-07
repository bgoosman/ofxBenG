#ifndef screen_h
#define screen_h

#include "ofMain.h"
#include "ofxPlaymodes.h"
#include "video_stream.h"
#include "window_view.h"
#include "header_view.h"

namespace ofxBenG {
    class monitor;

    class window {
    public:
        window(std::shared_ptr<ofAppBaseWindow> parentWindow);

        ~window();

        void exit(ofEventArgs &args);

        void close();

        void draw(ofEventArgs &args);

        void addView(ofxBenG::window_view *view);

        void removeView(ofxBenG::window_view *view);

        std::string getMonitorName();

        ofxBenG::monitor *getMonitor();

        void setMonitor(ofxBenG::monitor *monitor);

        void setStream(ofxBenG::video_stream *stream);

        void setBlackout(bool enabled);

        void setWindowPosition(int x, int y);

        void setWindowShape(int x, int y);

        void setFullscreen(bool enabled);

        ofxBenG::video_stream *getStream();

    private:
        ofxPm::VideoHeader *header;
        ofxPm::BasicVideoRenderer *renderer;
        ofxBenG::video_stream *stream;
        ofxBenG::monitor *monitor;
        shared_ptr<ofAppBaseWindow> myWindow;
        shared_ptr<ofAppBaseWindow> parentWindow;
        std::vector<ofxBenG::window_view *> views;
        bool isClosing = false;
        bool isBlackout = false;
    };
}

#endif
