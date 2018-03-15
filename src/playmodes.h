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
#include "ofxPS3EyeGrabber.h"

namespace ofxBenG {

using namespace ofxPm;

    namespace VideoDevice {
        class Configuration {
        public:
            virtual float getFps() = 0;
            virtual float getWidth() = 0;
            virtual float getHeight() = 0;
        };

        class iPhone {
            float getFps() {
                return 59.94;
            }

            float getWidth() {
                return 1280;
            }

            float getHeight() {
                return 720;
            }
        };

        class c920 {
            float getFps() {
                return 30;
            }

            float getWidth() {
                return 1280;
            }

            float getHeight() {
                return 720;
            }
        };
    }

class playmodes {
public:
    static const std::string blackmagic;
    static const std::string c920;
    static const std::string ps3eye;

    playmodes(const std::string& deviceName, float requestedWidth, float requestedHeight, float requestedFps) {
        grabbers = grabber.listDevices();
        blackMagic = new BlackMagicVideoSource();

        fps = requestedFps;
        delay = 0;
        speed = 1.0;
        currentBuffer = 0;

        if (grabber.isInitialized()) {
            std::cout << "closing grabber" << std::endl;
            grabber.close();
        } else if (blackMagic->isInitialized()) {
            std::cout << "closing blackMagic" << std::endl;
            blackMagic->close();
        }

        if (deviceName.compare("blackmagic") == 0) {
            selectBlackMagic(requestedWidth, requestedHeight, requestedFps);
        } else if (deviceName.compare("ps3eye") == 0) {
            selectPs3Eye();
        } else {
            selectVideoDeviceOrDefault(deviceName, requestedWidth, requestedHeight, requestedFps);
        }

        videoDeviceCount++;
    }
    
    ~playmodes() {
        for (int i = 0; i < bufferCount; i++) {
            auto& buffer = buffers[i];
            buffer.stop();
        }
    }

    static int beatsToFrames(float beats, float bpm, float fps) {
        return beats * (1 / bpm) * 60.0 * fps;
    }

    bool isInitialized() {
        return isAllocated;
    }
    
    void drawColumns(int columns) {
        if (isAllocated) {
            auto bufferCount = getBufferCount();
            auto columnWidth = ofGetWidth() / columns;
            auto columnHeight = ofGetHeight();
            auto videoWidth = getWidth();
            auto videoHeight = getHeight();
            auto x = 0.0;

            for (int i = 0; i < columns; i++, x += columnWidth) {
                auto thisColumnWidth = (videoWidth < columnWidth) ? videoWidth : columnWidth;
                auto thisColumnHeight = (videoHeight < columnHeight) ? videoHeight : columnHeight;
                auto cropX = (thisColumnWidth < videoWidth) ? videoWidth / 3.0f : 0.0f;
                auto cropWidth = (thisColumnWidth < videoWidth) ? thisColumnWidth : videoWidth;
                auto y = (videoHeight < ofGetHeight()) ? ofGetHeight() / 2.0f - thisColumnHeight / 2.0f : 0.0f;
                ofTexture &rendererTexture = getBufferTexture(i);
                rendererTexture.drawSubsection(x, y, thisColumnWidth, thisColumnHeight, cropX, 0, thisColumnWidth, videoHeight);
            }
        }
    }

    void selectPs3Eye() {
        videoWidth = 640;
        videoHeight = 480;
        fps = 60;
        grabber.setGrabber(std::make_shared<ofxPS3EyeGrabber>());
        grabber.setup(videoWidth, videoHeight);
        grabber.initGrabber(videoWidth, videoHeight);
        grabber.update();
        rate.setup(grabber, fps);
        std::cout << "capturing at resolution (" << videoWidth << ", " << videoHeight << ")" << std::endl;
    }
    
    void selectBlackMagic(float requestedWidth, float requestedHeight, float requestedFps) {
        // BlackMagic Pocket Cinema: 24fps, 1920x1080
        // iPhone: 59.94fps, 1280x720
        if (!blackMagic->isInitialized()) {
            if (blackMagic->setup(requestedFps, requestedWidth, requestedHeight)) {
                ofVec2f dimensions = blackMagic->getDimensions();
                videoWidth = dimensions[0];
                videoHeight = dimensions[1];
                rate.setup(*blackMagic, blackMagic->getFps());
                std::cout << "capturing at resolution (" << videoWidth << ", " << videoHeight << ")" << std::endl;
            } else {
                std::cout << "failed to initialize blackmagic" << std::endl;
            }
        } else {
            rate.setup(*blackMagic, blackMagic->getFps());
            std::cout << "switched rate to blackmagic" << std::endl;
        }
    }
    
    void selectVideoDeviceOrDefault(const std::string& deviceName, float requestedWidth, float requestedHeight, float requestedFps) {
        if (!grabber.isInitialized()) {
            for (ofVideoDevice& device : grabbers) {
                if (device.deviceName.find(deviceName) != std::string::npos) {
                    grabber.setDeviceID(device.id);
                    std::cout << "found " << deviceName << std::endl;
                    break;
                }
            }

            grabber.initGrabber(requestedWidth, requestedHeight);
            grabber.setFps(requestedFps);
            videoWidth = grabber.getWidth();
            videoHeight = grabber.getHeight();
        }
        rate.setup(grabber, requestedFps);
        std::cout << "capturing at resolution (" << videoWidth << ", " << videoHeight << ")" << std::endl;
    }

    void setup() {
        if (!isAllocated && (grabber.isInitialized() || blackMagic->isInitialized())) {
            for (int i = 0; i < bufferCount; i++) {
                bool const allocateOnSetup = true;
                auto &buffer = buffers[i];
                buffer.setup(rate, bufferSize, allocateOnSetup);

                auto &header = headers[i];
                header.setup(buffer);
                header.setDelayMs(0);

                auto &renderer = renderers[i];
                renderer.setup(headers[i]);
            }

            isAllocated = true;
        }
    }
    
    void update() {
        if (isAllocated) {
            if (grabber.isInitialized()) {
                grabber.update();
            }
            if (blackMagic->isInitialized()) {
                blackMagic->update();
            }
        }
    }

    VideoHeader& getHeader(int i) {
        return headers[i];
    }

    VideoBuffer& getBuffer(int i) {
        return buffers[i];
    }

    VideoRate& getRate() {
        return rate;
    }

    float getWidth() {
        return videoWidth;
    }
    
    float getHeight() {
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

    std::map<ofxPm::VideoFormat, std::vector<ofPtr<ofxPm::VideoFrame::Obj>>>* pool;
    ofxPm::VideoGrabber grabber;
    static int const bufferCount = 1;
    ofxPm::VideoBuffer buffers[bufferCount];
    ofxPm::VideoHeader headers[bufferCount];
    ofxPm::BasicVideoRenderer renderers[bufferCount];
    ofxPm::VideoRate rate;
    ofxBenG::BlackMagicVideoSource* blackMagic;
    
    float videoWidth, videoHeight;
    float beatsPerMinute;
    float forwardRatio;
    float backwardRatio;
    float speed;
    int currentBuffer;
    int delay;
    int fps;
    int videoDeviceCount = 0;
    static int const bufferSize = 600;

private:
    bool isAllocated = false;
    std::vector<ofVideoDevice> grabbers;
};

}; /* ofxBenG */
#endif /* playmodes_h */
