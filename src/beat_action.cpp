#include "beat_action.h"

using namespace ofxBenG;

bool beat_action_comparator::operator()(beat_action *first, beat_action *second) {
    return first->getTriggerBeat() > second->getTriggerBeat();
}

void beat_action::start(float currentBeat) {
    std::cout << ofToString(currentBeat) << ": Starting " << this->getLabel() << std::endl;
    queueTriggeredActions(currentBeat);
    startThisAction(currentBeat);
}

void beat_action::update(float currentBeat) {
    queueTriggeredActions(currentBeat);
    updateRunningActions(currentBeat);
    updateThisAction(currentBeat);
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
        stream(stream), blackoutLengthBeats(blackoutLengthBeats), videoLengthBeats(videoLengthBeats), isPlaying(false) {
    int const size = ofxBenG::utilities::beatsToSeconds(videoLengthBeats, 60) * 30;
    buffer = stream->makeBuffer(size);
    buffer->stop();
    header = new ofxPm::VideoHeader;
    headerView = new header_view(header);
    flickerView = new flicker_view();
}

flicker::~flicker() {
    delete flickerView;
    delete headerView;
    delete header;
    delete buffer;
}

void flicker::startThisAction(float currentBeat) {
    stream->getWindow()->addView(flickerView);

    float b = 0;
    schedule(new generic_action([this]() {
        buffer->resume();
    }), currentBeat, b);

    b += videoLengthBeats;
    schedule(new generic_action([this]() {
        buffer->stop();
        flickerView->setBlackout(true);
    }), currentBeat, b);

    b += blackoutLengthBeats;
    startBeat = currentBeat + b;
    schedule(new generic_action([this]() {
        flickerView->setBlackout(false);
        header->setup(*buffer);
        isPlaying = true;
        stream->getWindow()->addView(headerView);
    }), currentBeat, b);

    b += videoLengthBeats;
    schedule(new generic_action([this]() {
        stream->getWindow()->removeView(flickerView);
        stream->getWindow()->removeView(headerView);
        isPlaying = false;
    }), currentBeat, b);
}

void flicker::updateThisAction(float currentBeat) {
    if (isPlaying) {
        float amount = (currentBeat - startBeat) / videoLengthBeats;
        float lerp = ofLerp(0, buffer->size(), amount);
        float frames = buffer->size() - lerp;
        std::cout << frames << std::endl;
        header->setDelayFrames(frames);
    }
}

bool flicker::isThisActionDone(float currentBeat) {
    return true;
}

std::string flicker::getLabel() {
    return "Flicker";
}