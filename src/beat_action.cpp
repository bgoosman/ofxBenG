#include "beat_action.h"

using namespace ofxBenG;

bool beat_action_comparator::operator()(beat_action *first, beat_action *second) {
    return first->getTriggerBeat() > second->getTriggerBeat();
}

void beat_action::update(float currentBeat) {
    this->currentBeat = currentBeat;
    queueTriggeredActions(currentBeat);
    updateRunningActions(currentBeat);
    updateThisAction(currentBeat);
}

void beat_action::start(float currentBeat) {
    std::cout << ofToString(currentBeat) << ": Starting " << this->getLabel() << std::endl;
    queueTriggeredActions(currentBeat);
    startThisAction(currentBeat);
}

void beat_action::updateThisAction(float currentBeat) {

}

bool beat_action::isThisActionDone(float currentBeat) {
    return true;
}

void beat_action::schedule(beat_action *action, float currentBeat, float beatsFromNow) {
    float const scheduledBeat = currentBeat + beatsFromNow;
    std::cout << currentBeat << ": Scheduling " << action->getLabel() << " action at beat " << scheduledBeat
              << std::endl;
    action->setTriggerBeat(scheduledBeat);
    scheduledActions.push(action);
}

void beat_action::schedule(float currentBeat, float beatsFromNow, std::function<void()> action) {
    schedule(new generic_action(action), currentBeat, beatsFromNow);
}

void beat_action::setTriggerBeat(float value) {
    this->triggerBeat = value;
}

bool beat_action::isTriggered(float currentBeat) {
    return currentBeat >= triggerBeat;
}

bool beat_action::isDone(float currentBeat) {
    return isScheduleDone() && isThisActionDone(currentBeat);
}

bool beat_action::isScheduleDone() {
    return scheduledActions.size() == 0 && runningActions.size() == 0;
}

void beat_action::updateRunningActions(float currentBeat) {
    for (auto it = runningActions.begin(); it != runningActions.end();) {
        beat_action *action = *it;
        if (action->isDone(currentBeat)) {
            std::cout << currentBeat << " deleting action " << action->getLabel() << std::endl;
            it = this->runningActions.erase(it);
            delete action;
        } else if (action != nullptr) {
            action->update(currentBeat);
            it++;
        }
    }
}

void beat_action::queueTriggeredActions(float currentBeat) {
    beat_action *nextAction;
    while (scheduledActions.size() > 0) {
        nextAction = scheduledActions.top();
        if (nextAction->isTriggered(currentBeat)) {
            scheduledActions.pop();
            runningActions.push_back(nextAction);
            nextAction->start(currentBeat);
        } else {
            break;
        }
    }
}

float beat_action::getTriggerBeat() {
    return triggerBeat;
}

flicker::flicker(ofxBenG::video_stream *stream, float blackoutLengthBeats, float videoLengthBeats) :
        stream(stream),
        blackoutLengthBeats(blackoutLengthBeats),
        videoLengthBeats(videoLengthBeats),
        isPlaying(false),
        isBlackout(false) {
    int const size = ofxBenG::utilities::beatsToSeconds(videoLengthBeats, 60) * 30;
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

void flicker::startThisAction(float currentBeat) {
    stream->getWindow()->addView(this);
    buffer->resume();

    float b = videoLengthBeats;
    schedule(currentBeat, b, [this]() {
        buffer->stop();
        isBlackout = true;
    });

    b += blackoutLengthBeats;
    startBeat = currentBeat + b;
    schedule(currentBeat, b, [this]() {
        isBlackout = false;
        isPlaying = true;
        header->setup(*buffer);
    });

    b += videoLengthBeats;
    schedule(currentBeat, b, [this]() {
        isPlaying = false;
        stream->getWindow()->removeView(this);
    });
}

void flicker::updateThisAction(float currentBeat) {
    if (isPlaying) {
        float amount = (currentBeat - startBeat) / videoLengthBeats;
        float lerp = ofLerp(0, buffer->size(), amount);
        float frames = buffer->size() - lerp;
        header->setDelayFrames(frames);
    }
}

bool flicker::isThisActionDone(float currentBeat) {
    return true;
}

std::string flicker::getLabel() {
    return "Flicker";
}