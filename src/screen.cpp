#include "monitor.h"
#include "screen.h"

using namespace ofxBenG;

screen::screen(ofxBenG::video_stream *stream, std::shared_ptr<ofAppBaseWindow> parentWindow, ofxBenG::monitor *monitor)
        : parentWindow(parentWindow), stream(stream) {
    makeWindow(monitor);
    int newBuffer = stream->addBuffer();
    stream->recordInto(newBuffer);
    header = stream->makeHeader(newBuffer);
    renderer = new ofxPm::BasicVideoRenderer;
    renderer->setup(*header);
    this->monitor = monitor;
}

screen::~screen() {
    close();
}

void screen::exit(ofEventArgs &args) {
    close();
}

void screen::makeWindow(ofxBenG::monitor *monitor) {
    if (myWindow.get() != nullptr) {
        ofRemoveListener(myWindow->events().draw, this, &screen::draw);
        ofRemoveListener(myWindow->events().exit, this, &screen::exit);
        myWindow->setWindowShouldClose();
        myWindow.reset();
    }
    ofGLFWWindowSettings settings;
    settings.monitor = monitor->getId();
//    settings.windowMode = OF_FULLSCREEN;
    settings.shareContextWith = parentWindow;
    myWindow = ofCreateWindow(settings);
    ofAddListener(myWindow->events().draw, this, &screen::draw);
    ofAddListener(myWindow->events().exit, this, &screen::exit);
}

void screen::close() {
    if (!isClosing) {
        std::cout << "Screen of " << stream->getDeviceName() << " is closing" << std::endl;
        setFullscreen(false);
        monitor->setScreen(nullptr);
        monitor = nullptr;
        isClosing = true;
        delete renderer;
        delete header;
        myWindow->setWindowShouldClose();
    }
}

void screen::draw(ofEventArgs &args) {
    ofPoint size = myWindow->getWindowSize();
    renderer->draw(0, 0, size[0], size[1]);
}

void screen::setMonitor(ofxBenG::monitor *monitor) {
    std::cout << "Setting " << stream->getDeviceName() << " monitor to " << monitor->toString() << std::endl;
    this->monitor = monitor;
}

void screen::setWindowPosition(int x, int y) {
    myWindow->setWindowPosition(x, y);
}

void screen::setWindowShape(int w, int h) {
    myWindow->setWindowShape(w, h);
}

void screen::setFullscreen(bool enabled) {
    myWindow->setFullscreen(enabled);
}

video_stream *screen::getStream() {
    return this->stream;
}
