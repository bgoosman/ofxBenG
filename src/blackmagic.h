//
//  blackmagic.h
//  JumpRope
//
//  Created by MacBook Pro on 1/20/18.
//

#ifndef blackmagic_h
#define blackmagic_h

#include "ofxBlackMagic.h"
#include "frames/VideoFrame.h" /* ofxPlayModes */
#include "pipeline/video/VideoSource.h" /* ofxPlayModes */

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

    class BlackMagicVideoSource : public ofxPm::VideoSource {
    public:
        BlackMagicVideoSource() {}
        
        ~BlackMagicVideoSource() {
            cam.close();
        }

        bool setup() {
            isSetup = true;
            fps = 59.94f;
            width = 1280;
            height = 720;
            bool result = cam.setup(width, height, fps);
            update();
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
        
        void newFrame(ofPixels& pixels) {
            frame = ofxPm::VideoFrame::newVideoFrame(pixels);
            frame.getTextureRef();
            newFrameEvent.notify(this, frame);
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
        
    private:
        float fps;
        float width;
        float height;
        ofxPm::VideoFrame frame;
        ofxBlackMagic cam;
        RateTimer timer;
        bool isSetup;
    };
    
}; /* ofxBenG */

#endif /* blackmagic_h */
