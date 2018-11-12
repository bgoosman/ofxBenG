#include "beat_action.h"
#include "ofApp.h"

using namespace ofxBenG;

bool beat_action_comparator::operator()(beat_action *first, beat_action *second) {
    return first->getTriggerBeat() > second->getTriggerBeat();
}

beat_action::beat_action() {
}

beat_action::~beat_action() {
}

void beat_action::update() {
    queueTriggeredActions();
    updateRunningActions();
    updateThisAction();
}

void beat_action::start() {
//    std::cout << ofToString(currentBeat) << ": Starting " << this->getLabel() << std::endl;
    queueTriggeredActions();
    startThisAction();
}

bool beat_action::isThisActionDone() {
    return true;
}

void beat_action::schedule(float baseBeat, float beatsFromBase, beat_action *action) {
    float const scheduledBeat = baseBeat + beatsFromBase;
//    std::cout << currentBeat << ": Scheduling " << action->getLabel() << " action at beat " << scheduledBeat
//              << std::endl;
    action->setTriggerBeat(scheduledBeat);
    scheduledActions.push(action);
}

void beat_action::schedule(float beatsFromNow, beat_action *action) {
    schedule(ofxBenG::ableton::getInstance()->getBeat(), beatsFromNow, action);
}

void beat_action::schedule(float beatsFromNow, std::function<void()> action) {
    schedule(beatsFromNow, new generic_action(action));
}

void beat_action::scheduleNextWholeBeat(beat_action *action) {
    schedule(ofxBenG::ableton::getInstance()->getNextWholeBeat(), 0, action);
}

void beat_action::setTriggerBeat(float value) {
    this->triggerBeat = value;
}

bool beat_action::isDone() {
    return isScheduleDone() && isThisActionDone();
}

bool beat_action::isScheduleDone() {
    return scheduledActions.size() == 0 && runningActions.size() == 0;
}

void beat_action::updateRunningActions() {
    for (auto it = runningActions.begin(); it != runningActions.end();) {
        beat_action *action = *it;
        action->update();
        if (action->isDone()) {
//            std::cout << ofxBenG::ableton::getInstance()->getBeat() << " deleting action " << action->getLabel() << std::endl;
            it = this->runningActions.erase(it);
            delete action;
        } else {
            it++;
        }
    }
}

void beat_action::queueTriggeredActions() {
    beat_action *nextAction;
    while (scheduledActions.size() > 0) {
        nextAction = scheduledActions.top();
        if (ofxBenG::ableton::getInstance()->getBeat() >= nextAction->getTriggerBeat()) {
            scheduledActions.pop();
            runningActions.push_back(nextAction);
            nextAction->start();
        } else {
            break;
        }
    }
}

float beat_action::getTriggerBeat() {
    return triggerBeat;
}

flicker::flicker(ofxBenG::video_stream *stream,
        float blackoutLengthBeats,
        float videoLengthBeats) :
        stream(stream),
        blackoutLengthBeats(blackoutLengthBeats),
        videoLengthBeats(videoLengthBeats),
        isPlaying(false),
        isBlackout(false) {
    recordingFps = stream->getFps();
    int const size = ofxBenG::utilities::beatsToSeconds(videoLengthBeats, ofxBenG::ableton::getInstance()->getTempo()) * recordingFps;
    buffer = stream->makeBuffer(size);
    header = new ofxPm::VideoHeader;
    renderer = new ofxPm::BasicVideoRenderer;
    renderer->setup(*header);
}

flicker::~flicker() {
    delete header;
    delete buffer;
}

void flicker::draw(ofPoint windowSize) {
    if (isBlackout) {
        ofPushStyle();
        ofSetColor(ofColor::black);
        ofDrawRectangle(0, 0, windowSize[0], windowSize[1]);
        ofPopStyle();
    } else if (isPlaying) {
        renderer->draw(0, 0, windowSize[0], windowSize[1]);
    }
}

void flicker::startThisAction() {
    std::cout << ofxBenG::ableton::getInstance()->getBeat() << ": Start recording flicker" << std::endl;
    stream->getWindow()->addView(this);
    buffer->resume();

    float acc = videoLengthBeats;
    schedule(acc, [this]() {
        isBlackout = true;
        buffer->stop();
    });

    acc += blackoutLengthBeats;
    schedule(acc, [this]() {
        std::cout << ofxBenG::ableton::getInstance()->getBeat() << ": Start playing flicker" << std::endl;
        isBlackout = false;
        isPlaying = true;
        header->setup(*buffer);
        header->setFps(recordingFps);
        header->setPlaying(true);
        header->setLoopToStart();
        header->setLoopMode(OF_LOOP_NONE);
    });

    acc += videoLengthBeats;
    schedule(acc, [this]() {
        isPlaying = false;
        isBlackout = true;
    });

    acc += blackoutLengthBeats;
    schedule(acc, [this]() {
        isBlackout = false;
        stream->getWindow()->removeView(this);
    });
}

void flicker::updateThisAction() {
}

bool flicker::isThisActionDone() {
    return true;
}

std::string flicker::getLabel() {
    return "Flicker";
}