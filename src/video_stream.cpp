#include "video_stream.h"
#include "window.h"
#include "monitor.h"

using namespace ofxBenG;

video_stream::video_stream(std::string deviceName, ofxPm::VideoGrabber *grabber, int defaultBufferSize)
        : deviceName(deviceName),
          grabber(grabber),
          defaultBufferSize(defaultBufferSize),
          currentlyRecording(nullptr) {
    screen = nullptr;
}

video_stream::~video_stream() {
    std::cout << this->getDeviceName() << " is deleting" << std::endl;
    if (screen != nullptr)
        delete screen;
    for (auto buffer : buffers) {
        buffer->stop();
        buffer->clear();
        delete buffer;
    }
    grabber->close();
}

void video_stream::update() {
    if (grabber != nullptr)
        grabber->update();
}

void video_stream::draw() {
    if (currentlyRecording != nullptr) {
        ofxPm::VideoFrame frame = currentlyRecording->getNextVideoFrame();
        if (frame.isAllocated()) {
            ofTexture &ref = frame.getTextureRef();
            ref.draw(0, 0);
        }
    }
}

void video_stream::setScreen(ofxBenG::window *window) {
    this->screen = window;
}

ofxBenG::window *video_stream::getScreen() {
    return screen;
}

int video_stream::addBuffer() {
    auto buffer = new ofxPm::VideoBuffer();
    buffer->setup(*grabber, defaultBufferSize, false);
    buffers.push_back(buffer);
    return buffers.size() - 1;
}

void video_stream::recordInto(int i) {
    if (i > buffers.size() || i < 0)
        return;

    if (currentlyRecording != nullptr)
        currentlyRecording->stop();

    auto buffer = buffers[i];
    buffer->resume();
    currentlyRecording = buffer;
}

ofxPm::VideoHeader *video_stream::makeHeader(int i) {
    auto header = new ofxPm::VideoHeader;
    header->setup(*buffers[i]);
    return header;
}

std::string video_stream::getDeviceName() {
    return deviceName;
}

ofVec2f video_stream::getSize() {
    return ofVec2f(grabber->getWidth(), grabber->getHeight());
}