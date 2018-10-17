#ifndef VIDEO_STREAM_H
#define VIDEO_STREAM_H

#include "ofxPlaymodes.h"

namespace ofxBenG {
    class screen;
    class monitor;
    class video_stream {
    public:
        video_stream(std::string deviceName, ofxPm::VideoGrabber *grabber, int defaultBufferSize);

        ~video_stream();

        void update();

        void draw();

        ofxBenG::screen *makeScreen(std::shared_ptr<ofAppBaseWindow> parentWindow, ofxBenG::monitor *monitor);

        ofxBenG::screen *getScreen();

        int addBuffer();

        void recordInto(int i);

        ofxPm::VideoHeader *makeHeader(int i);

        std::string getDeviceName();

        ofVec2f getSize();

    private:
        ofxPm::VideoGrabber *grabber;
        ofxPm::VideoBuffer *currentlyRecording;
        std::string deviceName;
        std::vector<ofxPm::VideoBuffer *> buffers;
        int defaultBufferSize;
        ofxBenG::screen *screen;
    };
}

#endif
