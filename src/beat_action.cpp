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

flicker::flicker(ofxBenG::window *screen, float blackoutLengthBeats, float videoLengthBeats) :
        screen(screen), blackoutLengthBeats(blackoutLengthBeats), videoLengthBeats(videoLengthBeats) {
}

void flicker::startThisAction(float currentBeat) {
    schedule(new generic_action([this]() { this->screen->setBlackout(true); }), currentBeat, 0);
    schedule(new generic_action([this]() { this->screen->setBlackout(false); }), currentBeat, blackoutLengthBeats);
}

bool flicker::isThisActionDone(float currentBeat) {
    return true;
}

std::string flicker::getLabel() {
    return "Flicker";
}