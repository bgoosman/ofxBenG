#include "beat_action.h"

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

void beat_action::executeAction(beat_action *action) {
    runningActions.push_back(action);
    action->startThisAction();
}

void beat_action::start() {
//    std::cout << ofToString(currentBeat) << ": Starting " << this->getLabel() << std::endl;
    queueTriggeredActions();
    startThisAction();
}

bool beat_action::isThisActionDone() {
    return true;
}

void beat_action::clearScheduledActions() {
    scheduledActions = std::priority_queue<ofxBenG::beat_action *, std::vector<ofxBenG::beat_action *>, ofxBenG::beat_action_comparator>();
}

void beat_action::schedule(float baseBeat, float beatsFromBase, beat_action *action) {
    float const scheduledBeat = baseBeat + beatsFromBase;
//    std::cout << currentBeat << ": Scheduling " << action->getLabel() << " action at beat " << scheduledBeat
//              << std::endl;
    action->setTriggerBeat(scheduledBeat);
    scheduledActions.push(action);
}

void beat_action::schedule(float beatsFromNow, beat_action *action) {
    schedule(ofxBenG::ableton()->getBeat(), beatsFromNow, action);
}

void beat_action::schedule(float beatsFromNow, std::function<void()> action) {
    schedule(beatsFromNow, new generic_action(action));
}

void beat_action::scheduleOnNthBeatFromNow(int wholeBeatsFromNow, beat_action *action) {
    float beat = floor(ofxBenG::ableton()->getBeat() + wholeBeatsFromNow);
    schedule(beat, 0, action);
}

void beat_action::scheduleOnNthBeatFromNow(int wholeBeatsFromNow, std::function<void()> action) {
    float beat = floor(ofxBenG::ableton()->getBeat() + wholeBeatsFromNow);
    schedule(beat, 0, new generic_action(action));
}

void beat_action::scheduleNextWholeBeat(beat_action *action) {
    schedule(ofxBenG::ableton()->getNextWholeBeat(), 0, action);
}

void beat_action::scheduleNextWholeMeasure(beat_action *action) {
    float beat = ofxBenG::ableton()->getBeat();
    while (int(floor(beat)) % 4 != 0) beat += 1;
    schedule(floor(beat), 0, action);
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
//            ofxBenG::ableton()->log(" deleting action " + action->getLabel());
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
        if (ofxBenG::ableton()->getBeat() >= nextAction->getTriggerBeat()) {
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
        float videoLengthBeats,
        ofxBenG::etc_element_osc_proxy *lightBoard,
        float faderNumber,
        float lightLevelMin,
        float lightLevelMax,
        ofxBenG::flicker *lastFlicker) :
        stream(stream),
        blackoutLengthBeats(blackoutLengthBeats),
        videoLengthBeats(videoLengthBeats),
        isPlaying(false),
        isBlackout(false),
        lightBoard(lightBoard),
        faderNumber(faderNumber),
        lightLevelMin(lightLevelMin),
        lightLevelMax(lightLevelMax),
        lastFlicker(lastFlicker),
        isHoldingFrame(false),
        holdFrame(nullptr) {
    recordingFps = stream->getFps();
    int const size = ofxBenG::utilities::beatsToSeconds(videoLengthBeats, ofxBenG::ableton()->getTempo()) * recordingFps;
    buffer = stream->makeBuffer(size);
    header = new ofxPm::VideoHeader;
    renderer = new ofxPm::BasicVideoRenderer;
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
    }

    if (holdFrame != nullptr) {
        holdFrame->draw(0, 0, windowSize[0], windowSize[1]);
    } else if (renderer->isSetup()) {
        renderer->draw(0, 0, windowSize[0], windowSize[1]);
    }
}

void flicker::startThisAction() {
    float acc = 0;

    isBlackout = true;
    stream->getWindow()->addView(this);

    // Play last recording forwards
    if (lastFlicker != nullptr) {
        std::cout << ofxBenG::ableton()->getBeat() << ": Start playing last recording forwards" << std::endl;
        auto lastHeader = lastFlicker->getHeader();
        renderer->setup(*lastHeader);
        executeAction(new pan_video(lastHeader, lastFlicker->getVideoLengthBeats(), videoLengthBeats, ofxBenG::ableton()->getTempo(), recordingFps, pan_video::PLAY_FORWARDS));
    }

    // Fade in the lights
    std::cout << ofxBenG::ableton()->getBeat() << ": Start fading in" << std::endl;
    executeAction(fade(lightLevelMin, lightLevelMax));
    acc += videoLengthBeats;

    // Start recording and play this buffer live
    schedule(acc, [this]() {
        std::cout << ofxBenG::ableton()->getBeat() << ": Start recording" << std::endl;
        isBlackout = false;
        buffer->resume();
        header->setup(*buffer);
        renderer->setup(*header);
    });
    acc += videoLengthBeats;

    // Stop recording, fade out the lights, and hold the video
    schedule(acc, [this]() {
        std::cout << ofxBenG::ableton()->getBeat() << ": Start fading out" << std::endl;
        holdFrame = renderer->getLastTexture();
        buffer->stop();
        executeAction(fade(lightLevelMax, lightLevelMin));
    });
    acc += videoLengthBeats;

    schedule(acc, [this]() {
        std::cout << ofxBenG::ableton()->getBeat() << ": Start playing this recording backwards" << std::endl;
        holdFrame = nullptr;
        executeAction(new pan_video(header, videoLengthBeats, videoLengthBeats, ofxBenG::ableton()->getTempo(), recordingFps, pan_video::PLAY_BACKWARDS));
    });
    acc += videoLengthBeats;

    schedule(acc, [this]() {
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

float flicker::getVideoLengthBeats() {
    return videoLengthBeats;
}

ofxPm::VideoHeader *flicker::getHeader() {
    return header;
}

ofxBenG::lerp_action *flicker::fade(float start, float end) {
    return new lerp_action(videoLengthBeats, [this, start, end](float value, float min, float max) {
        float const faderLevel = ofMap(value, min, max, start, end);
        lightBoard->setSubmaster(faderNumber, faderLevel);
    });
}

lfo_action::lfo_action(float frequency, float startY, bool isHolding)
        : frequency(frequency), startY(startY), isHolding(isHolding) {
    phase = asin(startY);
}

void lfo_action::startThisAction() {
    phase -= beatsToRadian(ofxBenG::ableton()->getBeat());
}

void lfo_action::updateThisAction() {
    float const beat = ofxBenG::ableton()->getBeat();
    float y = sin(beat * TWO_PI * frequency + phase);
    if (!isHolding)
        ofNotifyEvent(onLfoValue, y);
}

bool lfo_action::isThisActionDone() {
    return false;
}

std::string lfo_action::getLabel() {
    return "lfo_action { frequency:" + ofToString(frequency) + " Hz, phase:" + ofToString(phase) + " rad, startY: " + ofToString(startY) + " }";
}

float lfo_action::map(float lfoValue, float targetMin, float targetMax) {
    return ofMap(lfoValue, -1, 1, targetMin, targetMax);
}

void lfo_action::setFrequency(float value) {
    this->frequency = value;
}

float lfo_action::beatsToRadian(float beat) {
    float secondsPerBeat = 60.0f / ofxBenG::ableton()->getTempo();
    return beat * secondsPerBeat * frequency * TWO_PI;
}

void lfo_action::setHolding(bool value) {
    isHolding = value;
}

lerp_action::lerp_action(float durationBeats, floatFunction onValue)
        : durationBeats(durationBeats),
          onValue(onValue) {

}

void lerp_action::startThisAction() {
    startBeat = ofxBenG::ableton()->getBeat();
    endBeat = startBeat + durationBeats;
}

void lerp_action::updateThisAction() {
    float const currentBeat = ofxBenG::ableton()->getBeat();
    onValue(ofMap(currentBeat, startBeat, endBeat, 0, 1, true), myMin, myMax);
}

bool lerp_action::isThisActionDone() {
    return ofxBenG::ableton()->getBeat() > endBeat;
}

std::string lerp_action::getLabel() {
    return "lerp_action { startBeat:" + ofToString(startBeat) + ", endBeat:" + ofToString(endBeat) + " }";
}

float lerp_action::map(float value, float targetMin, float targetMax) {
    return ofMap(value, myMin, myMax, targetMin, targetMax, false);
}
