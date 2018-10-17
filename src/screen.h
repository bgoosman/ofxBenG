#ifndef screen_h
#define screen_h

#include "ofMain.h"
#include "ofxPlaymodes.h"
#include "video_stream.h"

namespace ofxBenG {
    class monitor;

    class screen {
    public:
        screen(ofxBenG::video_stream *stream, std::shared_ptr<ofAppBaseWindow> parentWindow, ofxBenG::monitor *monitor);

        ~screen();

        void exit(ofEventArgs &args);

        void close();

        void draw(ofEventArgs &args);

        void setMonitor(ofxBenG::monitor *monitor);

        void setWindowPosition(int x, int y);

        void setWindowShape(int x, int y);

        void setFullscreen(bool enabled);

        void makeWindow(ofxBenG::monitor *monitor);

        ofxBenG::video_stream* getStream();

    private:
        ofxPm::VideoHeader *header;
        ofxPm::BasicVideoRenderer *renderer;
        ofxBenG::video_stream *stream;
        ofxBenG::monitor *monitor;
        shared_ptr<ofAppBaseWindow> myWindow;
        shared_ptr<ofAppBaseWindow> parentWindow;
        bool isClosing = false;
    };
}

#endif
