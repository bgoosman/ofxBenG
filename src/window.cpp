#include "monitor.h"
#include "window.h"

using namespace ofxBenG;

window::window(std::shared_ptr<ofAppBaseWindow> parentWindow)
        : parentWindow(parentWindow) {
    renderer = nullptr;
    header = nullptr;
    monitor = nullptr;
    stream = nullptr;
}

window::~window() {
    close();
}

void window::exit(ofEventArgs &args) {
    close();
}

void window::close() {
    if (!isClosing) {
        isClosing = true;
        if (stream != nullptr)
            stream->setWindow(nullptr);
        stream = nullptr;
        if (renderer != nullptr)
            delete renderer;
        if (header != nullptr)
            delete header;
        myWindow->setWindowShouldClose();
    }
}

void window::draw(ofEventArgs &args) {
    ofPoint windowSize = myWindow->getWindowSize();
    for (auto view : views) {
        view->draw(windowSize);
    }
}

void window::addView(ofxBenG::window_view *view) {
    views.push_back(view);
}

void window::removeView(ofxBenG::window_view *view) {
    views.erase(std::remove(views.begin(), views.end(), view), views.end());
}

void window::setStream(ofxBenG::video_stream* stream) {
    if (stream != nullptr) {
        this->stream = stream;
        std::cout << "Setting " << this->stream->getDeviceName() << " to window of " << this->getMonitorName() << std::endl;
        int newBuffer = this->stream->addBuffer();
        this->stream->recordInto(newBuffer);
        header = stream->makeHeader(newBuffer);
        this->addView(new header_view(header));
        this->stream->setWindow(this);
    }
}

void window::setBlackout(bool enabled) {
    isBlackout = enabled;
}

std::string window::getMonitorName() {
    return monitor->getName();
}

ofxBenG::monitor *window::getMonitor() {
    return monitor;
}

void window::setMonitor(ofxBenG::monitor *monitor) {
    if (this->monitor != nullptr) {
        ofRemoveListener(myWindow->events().draw, this, &window::draw);
        ofRemoveListener(myWindow->events().exit, this, &window::exit);
        this->myWindow->setWindowShouldClose();
        this->myWindow.reset();
    }

    this->monitor = monitor;
    ofGLFWWindowSettings settings;
    settings.monitor = monitor->getId();
    settings.shareContextWith = parentWindow; // If absent, white window
    this->myWindow = ofCreateWindow(settings);
    ofAddListener(myWindow->events().draw, this, &window::draw);
    ofAddListener(myWindow->events().exit, this, &window::exit);
    ofPoint p = monitor->getPosition();
    this->setWindowPosition(p[0], p[1]);
    this->setFullscreen(true);
}

void window::setWindowPosition(int x, int y) {
    myWindow->setWindowPosition(x, y);
}

void window::setWindowShape(int w, int h) {
    myWindow->setWindowShape(w, h);
}

void window::setFullscreen(bool enabled) {
    myWindow->setFullscreen(true);
}

video_stream *window::getStream() {
    return this->stream;
}
