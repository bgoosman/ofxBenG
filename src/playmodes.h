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
    static const std::string c615;
    static const std::string ps3eye;
    static const std::string facetime;

    playmodes(const std::string& deviceName, float requestedWidth, float requestedHeight, float requestedFps, int defaultBufferSize)
        : defaultBufferSize(defaultBufferSize) {
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

        renderer.setup(*getHeader(0));
    }
    
    ~playmodes() {
        for (int i = 0; i < bufferCount; i++) {
            auto& buffer = buffers[i];
            buffer.stop();
        }
    }

    void draw(float x, float y, float width, float height) {
        renderer.draw(x, y, width, height);
    }

    bool isInitialized() {
        return isAllocated;
    }

    void drawLastBufferFrame(int i, float x, float y, float w, float h) {
        auto& buffer = buffers[i];
        if (isAllocated && buffer.size() > 0) {
            auto frame = buffer.getVideoFrame((int)(buffer.size() - 1));
            auto texture = frame.getTextureRef();
            texture.drawSubsection(x, y, w, h, 0, 0, videoWidth, videoHeight);
        }
    }

    void drawHeader(int column, float x, float y, float w, float h) {
        auto header = headers[column];
        if (isAllocated && header != nullptr && header->getBuffer()->size() > 0) {
            ofTexture &rendererTexture = getBufferTexture(column);
            rendererTexture.drawSubsection(x, y, w, h, 0, 0, videoWidth, videoHeight);
        }
    }

    void drawColumn(int column) {
        if (isAllocated && buffers[column].size() > 0) {
            auto bufferCount = getBufferCount();
            auto columnWidth = ofGetWidth() / bufferCount;
            auto columnHeight = ofGetHeight();
            auto videoWidth = getWidth();
            auto videoHeight = getHeight();
            auto x = 0.0;
            auto thisColumnWidth = (videoWidth < columnWidth) ? videoWidth : columnWidth;
            auto thisColumnHeight = (videoHeight < columnHeight) ? videoHeight : columnHeight;
            auto cropX = (thisColumnWidth < videoWidth) ? videoWidth / 3.0f : 0.0f;
            auto cropWidth = (thisColumnWidth < videoWidth) ? thisColumnWidth : videoWidth;
            auto y = (videoHeight < ofGetHeight()) ? ofGetHeight() / 2.0f - thisColumnHeight / 2.0f : 0.0f;
            ofTexture &rendererTexture = getBufferTexture(column);
            rendererTexture.drawSubsection(x, y, thisColumnWidth, thisColumnHeight, cropX, 0, thisColumnWidth, videoHeight);
        }
    }

    void drawColumns(int columns, bool reverseColumns) {
        if (isAllocated) {
            auto bufferCount = getBufferCount();
            auto columnWidth = ofGetWidth() / columns;
            auto columnHeight = ofGetHeight();
            auto videoWidth = getWidth();
            auto videoHeight = getHeight();
            auto x = 0.0;

            for (int i = 0; i < columns; i++, x += columnWidth) {
                int column = i;
                if (reverseColumns) {
                    column = columns - column - 1;
                }
                auto thisColumnWidth = (videoWidth < columnWidth) ? videoWidth : columnWidth;
                auto thisColumnHeight = (videoHeight < columnHeight) ? videoHeight : columnHeight;
                auto cropX = (thisColumnWidth < videoWidth) ? videoWidth / 3.0f : 0.0f;
                auto cropWidth = (thisColumnWidth < videoWidth) ? thisColumnWidth : videoWidth;
                auto y = (videoHeight < ofGetHeight()) ? ofGetHeight() / 2.0f - thisColumnHeight / 2.0f : 0.0f;
                ofTexture &rendererTexture = getBufferTexture(column);
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

    void swapVideoDevice(const std::string& deviceName) {
        selectVideoDeviceOrDefault(deviceName, videoWidth, videoHeight, fps);
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

            grabber.setDesiredFrameRate(requestedFps);
            grabber.setFps(requestedFps);
            grabber.initGrabber(requestedWidth, requestedHeight);
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
                buffer.setup(rate, defaultBufferSize, allocateOnSetup);
                buffer.setBufIndex(i);
                buffer.stop();

                headers[i] = new ofxPm::VideoHeader(buffer);
                headers[i]->setDelayMs(0);

                auto &renderer = renderers[i];
                renderer.setup(*headers[i]);
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

    float getFps(int i) {
        return buffers[i].getFps();
    }

    VideoHeader* getPlayHeader() {
        return (VideoHeader*) renderer.getSource();
    }

    void setPlayHeader(VideoHeader* header) {
        renderer.setup(*header);
    }

    VideoHeader* getHeader(int i) {
        return headers[i];
    }

    void setHeader(int i, VideoHeader* header) {
        delete headers[i];
        headers[i] = header;
        renderers[i].setup(*header);
    }

    VideoBuffer* getBuffer(int i) {
        return &buffers[i];
    }

    VideoRate* getRate() {
        return &rate;
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
        delay = ofMap(_delay, 1.0, 0.0, 0, defaultBufferSize);
        getCurrentHeader()->setDelayFrames(delay);
    }
    
    void setDelayPercent(float percent) {
        for (auto header : headers) {
            header->setDelayPct(percent);
        }
    }
    
    void setFps(int _fps) {
        rate.setFps(_fps);
        getCurrentHeader()->setFps(_fps);
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
    
    ofxPm::VideoHeader* getCurrentHeader() {
        return headers[currentBuffer];
    }

    ofxPm::BasicVideoRenderer* getRenderer() {
        return &renderer;
    }
    
    ofTexture& getBufferTexture(int index) {
        return renderers[index].getNextTexture();
    }
    
    float getBufferDuration(int index) {
        auto& buffer = buffers[index];
        return (float)defaultBufferSize / (float)buffer.getRealFPS();
    }
    
    int getBufferCount() {
        return bufferCount;
    }
    
    void setDelayPercent(int index, float percent) {
        headers[index]->setDelayPct(percent);
    }

    std::map<ofxPm::VideoFormat, std::vector<ofPtr<ofxPm::VideoFrame::Obj>>>* pool;
    ofxPm::VideoGrabber grabber;
    static int const bufferCount = 3;
    ofxPm::VideoBuffer buffers[bufferCount];
    ofxPm::VideoHeader* headers[bufferCount];
    ofxPm::BasicVideoRenderer renderers[bufferCount];
    ofxPm::VideoRate rate;
    ofxBenG::BlackMagicVideoSource* blackMagic;
    ofxPm::BasicVideoRenderer renderer;

    float videoWidth, videoHeight;
    float beatsPerMinute;
    float forwardRatio;
    float backwardRatio;
    float speed;
    int currentBuffer;
    int delay;
    int fps;
    int defaultBufferSize;

private:
    bool isAllocated = false;
    std::vector<ofVideoDevice> grabbers;
};

}; /* ofxBenG */
#endif /* playmodes_h */
