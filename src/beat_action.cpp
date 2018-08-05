#include "beat_action.h"

using namespace ofxBenG;

bool beat_action_comparator::operator()(beat_action* first, beat_action* second) {
    return first->getTriggerBeat() > second->getTriggerBeat();
}

void beat_action::setup(float currentBeat) {
    setupThisAction(currentBeat);
    queueTriggeredActions(currentBeat);
}

void beat_action::start(float currentBeat) {
    std::cout << ofToString(currentBeat) << ": Starting " << this->getLabel() << std::endl;
    startThisAction(currentBeat);
}

void beat_action::update(float currentBeat) {
    queueTriggeredActions(currentBeat);
    updateRunningActions(currentBeat);
    updateThisAction(currentBeat);
}

void beat_action::schedule(beat_action* action, float currentBeat, float beatsFromNow) {
    float const scheduledBeat = currentBeat + beatsFromNow;
    std::cout << currentBeat << ": Scheduling " << action->getLabel() << " action at beat " << scheduledBeat << std::endl;
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
    return scheduleIsDone() && isThisActionDone(currentBeat);
}

bool beat_action::scheduleIsDone() {
    return scheduledActions.size() == 0 && runningActions.size() == 0;
}

void beat_action::updateRunningActions(float currentBeat) {
    for (auto it = runningActions.begin(); it != runningActions.end();) {
        beat_action* action = *it;
        if (action->isDone(currentBeat)) {
            std::cout << currentBeat << " deleting action " << action->getLabel() << std::endl;
            delete action;
            action = nullptr;
            it = this->runningActions.erase(it);
        } else if (action != nullptr) {
            action->update(currentBeat);
            it++;
        }
    }
}

void beat_action::queueTriggeredActions(float currentBeat) {
    beat_action* nextAction;
    while (scheduledActions.size() > 0) {
        nextAction = scheduledActions.top();
        if (nextAction->isTriggered(currentBeat)) {
            scheduledActions.pop();
            runningActions.push_back(nextAction);
            nextAction->setup(currentBeat);
            nextAction->start(currentBeat);
        } else {
            break;
        }
    }
}

float beat_action::getTriggerBeat() {
    return triggerBeat;
}
