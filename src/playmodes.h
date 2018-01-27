//
//  playmodes.h
//  JumpRope
//
//  Created by MacBook Pro on 1/20/18.
//

#ifndef playmodes_h
#define playmodes_h

#include "AVHeaders/VideoHeader.h"
#include "buffers/VideoBuffer.h"
#include "filters/VideoRate.h"
#include "frames/VideoFrame.h"
#include "grabbers/VideoGrabber.h"
#include "renderers/VideoRenderer.h"
#include "renderers/BasicVideoRenderer.h"
#include "utils/VideoFormat.h"
#include "blackmagic.h"

namespace ofxBenG {
    
using namespace ofxPm;

class PlayModes {
public:
    PlayModes() {}
    ~PlayModes() {
        for (int i = 0; i < bufferCount; i++) {
            auto& buffer = buffers[i];
            buffer.stop();
        }
    }
    void setup(const std::string& deviceName, float requestedWidth, float requestedHeight) {
        fps = 60;
        delay = 0;
        speed = 1.0;
        currentBuffer = 0;
        
        if (deviceName.compare("blackmagic") == 0) {
            selectBlackMagic();
        } else {
            selectVideoDeviceOrDefault(deviceName, requestedWidth, requestedHeight);
        }

        for (int i = 0; i < bufferCount; i++) {
            bool const allocateOnSetup = true;
            auto& buffer = buffers[i];
            buffer.setup(rate, bufferSize, allocateOnSetup);
            
            auto& header = headers[i];
            header.setup(buffer);
            header.setDelayMs(0);
            
            auto& renderer = renderers[i];
            renderer.setup(headers[i]);
        }
    }
    void selectBlackMagic() {
        if (blackMagic.setup()) {
            ofVec2f dimensions = blackMagic.getDimensions();
            videoWidth = dimensions[0];
            videoHeight = dimensions[1];
            rate.setup(blackMagic, blackMagic.getFps());
            std::cout << "capturing at resolution (" << videoWidth << ", " << videoHeight << ")" << std::endl;
        } else {
            std::cout << "Failed to initialize BlackMagic" << std::endl;
        }
    }
    void selectVideoDeviceOrDefault(const std::string& deviceName, float requestedWidth, float requestedHeight) {
        std::vector<ofVideoDevice> grabbers = grabber.listDevices();
        for (ofVideoDevice& device : grabbers) {
            if (device.deviceName.find(deviceName) != std::string::npos) {
                grabber.setDeviceID(device.id);
                std::cout << "found " << deviceName << std::endl;
                break;
            }
        }
        
        grabber.initGrabber(requestedWidth, requestedHeight);
        videoWidth = grabber.getWidth();
        videoHeight = grabber.getHeight();
        rate.setup(grabber, fps);
        std::cout << "capturing at resolution (" << videoWidth << ", " << videoHeight << ")" << std::endl;
    }
    void update() {
        grabber.update();
        blackMagic.update();
    }
    float getVideoWidth() {
        return videoWidth;
    }
    float getVideoHeight() {
        return videoHeight;
    }
    void setCurrentBuffer(int index) {
        currentBuffer = index;
    }
    void setDelay(float _delay) {
        delay = ofMap(_delay, 1.0, 0.0, 0, bufferSize);
        getCurrentHeader().setDelayFrames(delay);
    }
    void setDelayPercent(float percent) {
        for (auto header : headers) {
            header.setDelayPct(percent);
        }
    }
    void setFps(int _fps) {
        rate.setFps(_fps);
        getCurrentHeader().setFps(_fps);
    }
    void toggleRecording() {
        toggleRecording(currentBuffer);
    }
    void toggleRecording(int index) {
        if (isRecording(index)) {
            resumeRecording(index);
        } else {
            pauseRecording(index);
        }
    }
    bool isRecording(int index) {
        return !buffers[index].isStopped();
    }
    void pauseRecording(int index) {
        buffers[index].stop();
    }
    void resumeRecording(int index) {
        buffers[index].resume();
    }
    void setForwardRatio(float value) {
        forwardRatio = value;
    }
    void setBackwardRatio(float value) {
        backwardRatio = value;
    }
    ofxPm::VideoBuffer& getCurrentBuffer() {
        return buffers[currentBuffer];
    }
    ofxPm::VideoHeader& getCurrentHeader() {
        return headers[currentBuffer];
    }
    ofTexture& getBufferTexture(int index) {
        return renderers[index].getNextTexture();
    }
    float getBufferDuration(int index) {
        auto& buffer = buffers[index];
        return (float)bufferSize / (float)buffer.getRealFPS();
    }
    int getBufferCount() {
        return bufferCount;
    }
    void setDelayPercent(int index, float percent) {
        headers[index].setDelayPct(percent);
    }
    int getBoomerangsPerMeasure() {
        return boomerangsPerMeasure;
    }
    
    std::map<ofxPm::VideoFormat, std::vector<ofPtr<ofxPm::VideoFrame::Obj>>>* pool;
    ofxPm::VideoGrabber grabber;
    static int const bufferCount = 3;
    ofxPm::VideoBuffer buffers[bufferCount];
    ofxPm::VideoHeader headers[bufferCount];
    ofxPm::BasicVideoRenderer renderers[bufferCount];
    ofxPm::VideoRate rate;
    ofxBenG::BlackMagicVideoSource blackMagic;
    
    float videoWidth, videoHeight;
    float beatsPerMinute;
    float forwardRatio;
    float backwardRatio;
    float speed;
    int currentBuffer;
    int delay;
    int fps;
    static int const bufferSize = 180;
    int boomerangsPerMeasure = 2;
};

}; /* ofxBenG */
#endif /* playmodes_h */
