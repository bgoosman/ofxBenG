#ifndef blackmagic_h
#define blackmagic_h

#include "ofxBlackMagic.h"
#include "ofxPlaymodes.h"
//#include "frames/VideoFrame.h" /* ofxPlayModes */
//#include "pipeline/video/VideoSource.h" /* ofxPlayModes */

namespace ofxBenG {
    class RateTimer {
    protected:
        float lastTick, averagePeriod, smoothing;
        bool secondTick;
    public:
        RateTimer() :
        smoothing(.9) {
            reset();
        }
        void reset() {
            lastTick = 0, averagePeriod = 0, secondTick = false;
        }
        void setSmoothing(float smoothing) {
            this->smoothing = smoothing;
        }
        float getFramerate() {
            return averagePeriod == 0 ? 0 : 1 / averagePeriod;
        }
        void tick() {
            float curTick = ofGetElapsedTimef();
            if(lastTick == 0) {
                secondTick = true;
            } else {
                float curDiff = curTick - lastTick;;
                if(secondTick) {
                    averagePeriod = curDiff;
                    secondTick = false;
                } else {
                    averagePeriod = ofLerp(curDiff, averagePeriod, smoothing);
                }
            }
            lastTick = curTick;
        }
    };

    class BlackMagicVideoSource : public ofxPm::VideoGrabber {
    public:
        BlackMagicVideoSource() {}
        
        ~BlackMagicVideoSource() {
            this->close();
        }

        bool setup(float fps, float width, float height) {
            bool result = false;
            if (!isSetup) {
                // iPhone: 59.94
                // BlackMagic Pocket Cinema: 24
                this->fps = fps;
                this->width = width;
                this->height = height;
                result = cam.setup(width, height, fps);
                if (result) {
                    update();
                    isSetup = true;
                }
            }
            return result;
        }
        
        ofxPm::VideoFrame getNextVideoFrame() {
            return frame;
        }
        
        void update() {
            if (isSetup) {
                cam.update();
                newFrame(cam.getColorPixels());
            }
        }

        void close() {
            if (isSetup) {
                cam.close();
                isSetup = false;
            }
        }
        
        void newFrame(ofPixels& pixels) {
            if (pixels.isAllocated()) {
                frame = ofxPm::VideoFrame::newVideoFrame(pixels);
                frame.getTextureRef();
                newFrameEvent.notify(this, frame);
            }
        }
        
        float getFps() {
            return fps;
        }
        
        void setFps(float fps) {
            this->fps = fps;
        }
        
        ofVec2f getDimensions() {
            return ofVec2f(width, height);
        }

        bool isInitialized() {
            return isSetup;
        }
        
    private:
        float fps;
        float width;
        float height;
        ofxPm::VideoFrame frame;
        ofxBlackMagic cam;
        RateTimer timer;
        bool isSetup = false;
    };
}; /* ofxBenG */

#endif /* blackmagic_h */
