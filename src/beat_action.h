//
//  beat_action.h
//  Stutter
//
//  Created by MacBook Pro on 2/4/18.
//

#ifndef beat_action_h
#define beat_action_h

#include "VideoBuffer.h"
#include "VideoHeader.h"
#include "ofxBenG.h"
#include "maxim.h"
#include "VideoBuffer.h"
#include "VideoHeader.h"
#include "VideoRate.h"
#include "ease.h"
#include "beat_action.h"
#include "playmodes.h"
#include "maxim.h"
#include "audio.h"
#include <algorithm>
#include <vector>

namespace ofxBenG {
    class beat_action {
    public:
        beat_action() : baseBeat(0) {}
        beat_action(float baseBeat) : baseBeat(baseBeat) {}
        virtual ~beat_action() {}
        virtual std::string getLabel() = 0;
        virtual void updateThisAction(float beat) = 0;
        virtual void startThisAction(float beat) = 0;
        virtual bool isThisActionDone(float beat) = 0;
        virtual void draw(float beat) = 0;
        virtual float getLengthBeats() = 0;

        virtual void start(float beat) {
            std::cout << ofToString(beat) << ": Starting " << this->getLabel() << std::endl;
            sortSchedule();
            startThisAction(beat);
        }

        virtual void update(float beat) {
            queueTriggeredActions(beat);
            updateRunningActions(beat);
            updateThisAction(beat);
        }

        virtual void schedule(beat_action* action, float beat) {
            float const scheduledBeat = this->baseBeat + beat;
            action->setTriggerBeat(scheduledBeat);
            std::cout << "Scheduling " << action->getLabel() << " action at beat " << scheduledBeat << std::endl;
            scheduledActions.push_back(action);
        }

        virtual void setTriggerBeat(float value) {
            this->triggerBeat = value;
        }

        virtual float getTriggerBeat() {
            return triggerBeat;
        }

        virtual bool isTriggered(float beat) {
            return beat >= triggerBeat;
        }

        virtual bool isDone(float beat) {
            return scheduleIsDone() && isThisActionDone(beat);
        }

    protected:
        float getLengthOfScheduleInBeats() {
            float length = 0;
            for (auto action : scheduledActions) {
                length += action->getLengthBeats();
            }
            return length;
        }

        float getLengthOfRunningActionsInBeats() {
            float length = 0;
            for (auto action : runningActions) {
                length += action->getLengthBeats();
            }
            return length;
        }

        float baseBeat = 0.0f;
        std::deque<ofxBenG::beat_action*> runningActions;
        std::vector<ofxBenG::beat_action*> scheduledActions;

    private:
        void sortSchedule() {
            std::sort(scheduledActions.begin(), scheduledActions.end(), [](beat_action* first, beat_action* second) {
                return first->getTriggerBeat() >= second->getTriggerBeat();
            });
        }

        bool scheduleIsDone() {
            return scheduledActions.size() == 0 && runningActions.size() == 0;
        }

        virtual void updateRunningActions(float beat) {
            for (auto it = runningActions.begin(); it != runningActions.end();) {
                beat_action* action = *it;
                if (action->isDone(beat)) {
                    std::cout << beat << " deleting action " << action->getLabel() << std::endl;
                    delete action;
                    action = nullptr;
                    it = this->runningActions.erase(it);
                } else {
                    action->update(beat);
                    it++;
                }
            }
        }

        virtual void queueTriggeredActions(float beat) {
            beat_action* nextAction;
            while (scheduledActions.size() > 0) {
                nextAction = scheduledActions.back();
                if (nextAction->isTriggered(beat)) {
                    scheduledActions.pop_back();
                    runningActions.push_back(nextAction);
                    nextAction->start(beat);
                } else {
                    break;
                }
            }
        }

        float triggerBeat;
    };

    class record_action : public beat_action {
    public:
        record_action(ofxPm::VideoBuffer* buffer) : buffer(buffer) {}
        virtual ~record_action() {};

        virtual void startThisAction(float beat) {
            buffer->resume();
        }

        virtual void updateThisAction(float beat) {}

        virtual void draw(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual float getLengthBeats() {
            return 0.0;
        }

        virtual std::string getLabel() {
            return "Record";
        }

    private:
        ofxPm::VideoBuffer* buffer;
    };

    class stop_recording_action : public beat_action {
    public:
        stop_recording_action(ofxPm::VideoBuffer* buffer) : buffer(buffer) {}
        virtual ~stop_recording_action() {};

        virtual void startThisAction(float beat) {
            buffer->stop();
        }

        virtual void updateThisAction(float beat) {}

        virtual void draw(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual float getLengthBeats() {
            return 0.0;
        }

        virtual std::string getLabel() {
            return "Stop Recording";
        }

    private:
        ofxPm::VideoBuffer* buffer;
    };

    class resume_recording_action : public beat_action {
    public:
        resume_recording_action(ofxPm::VideoBuffer* buffer) : buffer(buffer) {}
        virtual ~resume_recording_action() {};

        virtual void startThisAction(float beat) {
            buffer->resume();
        }

        virtual void updateThisAction(float beat) {}

        virtual void draw(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual float getLengthBeats() {
            return 0.0;
        }

        virtual std::string getLabel() {
            return "Resume Recording";
        }

    private:
        ofxPm::VideoBuffer* buffer;
    };

    class play_from_beginning_action : public beat_action {
    public:
        play_from_beginning_action(ofxPm::VideoHeader* header) : header(header) {}
        virtual ~play_from_beginning_action() {};

        virtual void startThisAction(float beat) {
            header->setLoopToStart();
            header->setPlaying(true);
        }

        virtual void updateThisAction(float beat) {}

        virtual void draw(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual float getLengthBeats() {
            return 0.0;
        }

        virtual std::string getLabel() {
            return "Play From Beginning";
        }

    private:
        ofxPm::VideoHeader* header;
    };

    class pan_video : public beat_action {
    public:
        pan_video(ofxPm::VideoHeader* header, float originalLengthBeats, float targetLengthBeats, float bpm, float fps)
                : pan_video(header, originalLengthBeats, targetLengthBeats, bpm, fps, true) {}

        pan_video(ofxPm::VideoHeader* header, float originalLengthBeats, float targetLengthBeats, float bpm, float fps, bool playForwards)
                : header(header), originalLengthBeats(originalLengthBeats), targetLengthBeats(targetLengthBeats), playForwards(playForwards) {
            delayFrames = originalLengthBeats * (60 / bpm) * fps;
            speed = originalLengthBeats / targetLengthBeats;
        }

        virtual void startThisAction(float beat) {
            startBeat = beat;
            header->setDelayFrames(playForwards ? delayFrames : 0);
            std::cout << beat << ": startBeat=" << startBeat
                      << ", originalLengthBeats=" << originalLengthBeats
                      << ", targetLengthBeats=" << targetLengthBeats
                      << std::endl;
        }

        virtual void updateThisAction(float beat) {
            float amount = (beat - startBeat) / targetLengthBeats;
            float lerp = ofLerp(0, delayFrames, amount);
            float frames = (playForwards) ? delayFrames - lerp : lerp;
            header->setDelayFrames(frames);
        }

        virtual void draw(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return beat >= startBeat + targetLengthBeats;
        }

        virtual std::string getLabel() {
            return "Pan Video";
        }

        virtual float getLengthBeats() {
            return targetLengthBeats;
        }

        static bool const PLAY_FORWARDS = true;
        static bool const PLAY_BACKWARDS = false;
    private:
        float startBeat;
        float originalLengthBeats;
        float targetLengthBeats;
        float delayFrames;
        float speed;
        bool playForwards;
        ofxPm::VideoHeader* header;
    };

    class reverse_audio : public beat_action {
    public:
        reverse_audio(ofxBenG::audio* audio, ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample, float lengthBeats, float bpm)
                : audio(audio), forwardSample(forwardSample), backwardSample(backwardSample), lengthBeats(lengthBeats), bpm(bpm) {}

        ~reverse_audio() {
            delete reverse;
        }

        virtual void startThisAction(float beat) {
            float lengthSeconds = ofxBenG::utilities::beatsToSeconds(lengthBeats, bpm);
            reverse = new maxim_reverse(audio, forwardSample, backwardSample, lengthSeconds);
        }

        virtual void updateThisAction(float beat) {
            reverse->update();
        }

        virtual void draw(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return reverse->isDone();
        }

        virtual float getLengthBeats() {
            return lengthBeats;
        }

        virtual std::string getLabel() {
            return "Reverse Audio";
        }

    private:
        ofxBenG::maxim_reverse* reverse;
        ofxMaxiSample* forwardSample;
        ofxMaxiSample* backwardSample;
        float bpm;
        float lengthBeats;
        ofxBenG::audio* audio;
    };

    class stutter_audio : public beat_action {
    public:
        stutter_audio(ofxMaxiSample* sample, float lengthBeats, float bpm)
                : sample(sample), lengthBeats(lengthBeats), bpm(bpm) {}

        ~stutter_audio() {
            delete stutter;
        }

        virtual void startThisAction(float beat) {
            float lengthSeconds = lengthBeats * (1 / (bpm / 60));
            stutter = new maxim_stutter(sample, lengthSeconds, 1);
        }

        virtual void updateThisAction(float beat) {
            stutter->update();
        }

        virtual void draw(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return stutter->isDone();
        }

        virtual float getLengthBeats() {
            return lengthBeats;
        }

        virtual std::string getLabel() {
            return "Pan Audio";
        }

    private:
        ofxBenG::maxim_stutter* stutter;
        ofxMaxiSample* sample;
        float bpm;
        float lengthBeats;
    };

    class play_tone : public beat_action {
    public:
        play_tone(ofxBenG::audio* audio, float durationBeats, float frequency)
                : audio(audio), durationBeats(durationBeats), frequency(frequency) {
            tone = new mix_t([this]() { return this->oscillator.sinewave(this->frequency) / 3; });
        }

        virtual ~play_tone() {
            audio->remove(tone);
        };

        virtual void startThisAction(float beat) {
            startBeat = beat;
            audio->add(tone);
        }

        virtual void updateThisAction(float beat) {}

        virtual void draw(float beat) {}

        virtual bool isThisActionDone(float currentBeat) {
            return currentBeat >= startBeat + durationBeats;
        }

        virtual float getLengthBeats() {
            return durationBeats;
        }

        virtual std::string getLabel() {
            return "Play Tone";
        }

    private:
        float durationBeats;
        float startBeat;
        float frequency;
        ofxBenG::mix_t* tone;
        ofxBenG::audio* audio;
        ofxMaxiOsc oscillator;
    };

    class stutter : public beat_action {
    public:
        stutter(float baseBeat, float recordLengthBeats, float stutterLengthBeats, float bpm,
                float recordingFps, int stutterTimes, ofxPm::VideoBuffer* buffer, ofxPm::VideoRate* rate, ofxPm::BasicVideoRenderer* renderer,
                ofxMaxiSample* sample, ofxBenG::audio* audio)
                : beat_action(baseBeat), renderer(renderer), oldHeader((VideoHeader*)renderer->getSource()) {
            header.setup(*buffer);

            schedule(new record_action(buffer), 0);
            schedule(new stop_recording_action(buffer), recordLengthBeats);

            float const firstToneDurationBeats = 0.2;
            float const noteA4 = 440,
                    noteASharp4 = 466.16,
                    noteB4 = 493.88,
                    noteC5 = 523.25,
                    noteD5 = 587.33,
                    noteDSharp5 = 622.25,
                    noteE5 = 659.25,
                    noteF5 = 698.46,
                    noteFSharp5 = 739.99,
                    noteG5 = 783.99,
                    noteGSharp5 = 830.61;
            float nextTone = 0;
            float const firstNote = noteA4;
            schedule(new play_tone(audio, firstToneDurationBeats, firstNote), nextTone);

            float nextStutter = recordLengthBeats;
            float const secondToneDurationBeats = 0.25;
            std::vector<float> const scale = {noteA4, noteASharp4, noteB4, noteC5, noteD5, noteDSharp5, noteE5,
                    noteF5, noteFSharp5, noteG5, noteGSharp5};
            for (int i = 0; i < stutterTimes; i++, nextStutter += stutterLengthBeats) {
                float const stutterNote = scale[stutterTimes - i];
                schedule(new play_tone(audio, secondToneDurationBeats, stutterNote), nextStutter);
                schedule(new pan_video(&header, recordLengthBeats, stutterLengthBeats, bpm, recordingFps), nextStutter);
                schedule(new stutter_audio(sample, stutterLengthBeats, bpm), nextStutter);
            }

            schedule(new resume_recording_action(buffer), nextStutter);
        }

        static stutter* make_random(float beat, float bpm,
                float fps, ofxPm::VideoBuffer* buffer, ofxPm::VideoRate* rate, ofxPm::BasicVideoRenderer* renderer,
                ofxMaxiSample* sample, ofxBenG::audio* audio) {
            float const stutterLengthBeats = ofRandom(0.1, 4);
            float const recordLengthBeats = ofRandom(stutterLengthBeats, 4);
            int minTimes, maxTimes;
            if (stutterLengthBeats < 1) {
                minTimes = 3;
                maxTimes = 8;
            } else if (1 <= stutterLengthBeats && stutterLengthBeats < 3) {
                minTimes = 1;
                maxTimes = 8;
            } else {
                minTimes = 1;
                maxTimes = 4;
            }
            int const stutterTimes = (int)ofRandom(minTimes, maxTimes);
            return new stutter(beat, recordLengthBeats, stutterLengthBeats, bpm,
                    fps, stutterTimes, buffer, rate, renderer,
                    sample, audio);
        }

        ~stutter() {
            renderer->setup(*oldHeader);
        }

        virtual void draw(float beat) {
            buffer.draw();
        }

        virtual void startThisAction(float beat) {
            renderer->setup(header);
        }

        virtual void updateThisAction(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual float getLengthBeats() {
            return getLengthOfScheduleInBeats();
        }

        virtual std::string getLabel() {
            return "Stutter";
        }

        bool isPlaying() {
            return header.isPlaying();
        }

        ofxPm::VideoHeader* getHeader() {
            return &header;
        }

    private:
        ofxPm::VideoBuffer buffer;
        ofxPm::VideoHeader header;
        ofxPm::VideoHeader* oldHeader;
        ofxPm::VideoRate* rate;
        ofxPm::BasicVideoRenderer* renderer;
    };

    class reverse : public beat_action {
    public:
        reverse(float baseBeat, float recordLengthBeats, float stutterLengthBeats, float bpm,
                float fps, ofxPm::VideoBuffer* buffer, ofxPm::VideoRate* rate, ofxPm::BasicVideoRenderer* renderer,
                ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample, ofxBenG::audio* audio)
                : beat_action(baseBeat), renderer(renderer), oldHeader((VideoHeader*)renderer->getSource()) {
            header.setup(*buffer);
            schedule(new record_action(buffer), 0);
            schedule(new stop_recording_action(buffer), recordLengthBeats);
            float const firstToneDurationBeats = 0.25;
            float const secondToneDurationBeats = 0.25;
            float const noteA = 440, noteE5 = 659.25;
            schedule(new play_tone(audio, firstToneDurationBeats, noteA), 0);
            schedule(new play_tone(audio, secondToneDurationBeats, noteE5), recordLengthBeats - secondToneDurationBeats);
            schedule(new pan_video(&header, recordLengthBeats, stutterLengthBeats, bpm, fps, pan_video::PLAY_BACKWARDS), recordLengthBeats);
            schedule(new reverse_audio(audio, forwardSample, backwardSample, stutterLengthBeats, bpm), recordLengthBeats);
            schedule(new resume_recording_action(buffer), recordLengthBeats + stutterLengthBeats);
        }

        static reverse* make_random(float beat, float bpm,
                float fps, ofxPm::VideoBuffer* buffer, ofxPm::VideoRate* rate, ofxPm::BasicVideoRenderer* renderer,
                ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample, ofxBenG::audio* audio) {
            float const recordLengthBeats = 6;
            float const stutterLengthBeats = ofRandom(3, 6);
            return new reverse(beat, recordLengthBeats, stutterLengthBeats, bpm,
                    fps, buffer, rate, renderer,
                    forwardSample, backwardSample, audio);
        }

        ~reverse() {
            renderer->setup(*oldHeader);
        }

        virtual void draw(float beat) {
            buffer.draw();
        }

        virtual void startThisAction(float beat) {
            renderer->setup(header);
        }

        virtual void updateThisAction(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual float getLengthBeats() {
            return getLengthOfScheduleInBeats();
        }

        virtual std::string getLabel() {
            return "Reverse";
        }

        bool isPlaying() {
            return header.isPlaying();
        }

        ofxPm::VideoHeader* getHeader() {
            return &header;
        }

    private:
        ofxPm::VideoBuffer buffer;
        ofxPm::VideoHeader header;
        ofxPm::VideoHeader* oldHeader;
        ofxPm::VideoRate* rate;
        ofxPm::BasicVideoRenderer* renderer;
    };

    class effect_generator : beat_action {
    public:
        effect_generator(float beat, float bpm,
                float fps, ofxPm::VideoBuffer* buffer, ofxPm::VideoRate* rate, ofxPm::BasicVideoRenderer* renderer,
                ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample, ofxBenG::audio* audio)
                : beat_action(beat), bpm(bpm), fps(fps), buffer(buffer), rate(rate), renderer(renderer), forwardSample(forwardSample),
                  backwardSample(backwardSample), audio(audio) {
        };
        virtual ~effect_generator() {}

        virtual void startThisAction(float beat) {
            audio->setSample(forwardSample);
            scheduleRandomEffect(beat);
        }

        virtual void updateThisAction(float beat) {
            if (scheduledActions.size() == 0 && runningActions.size() == 0) {
                scheduleRandomEffect(beat);
            }
        }

        void scheduleRandomEffect(float beat) {
            float const delay = ofRandom(minBeatsBetweenEffects, maxBeatsBetweenEffects);
            float const futureBeat = beat + delay;
            schedule(makeRandomEffect(futureBeat), delay);
        }

        virtual beat_action* makeRandomEffect(float beat) {
            beat_action* action = nullptr;
            float r = ofRandom(1);
            if (r < 0.25) {
                stutter* s = stutter::make_random(beat, bpm, fps, buffer, rate, renderer, forwardSample, audio);
                action = s;
            } else {
                reverse* r = reverse::make_random(beat, bpm, fps, buffer, rate, renderer, forwardSample, backwardSample, audio);
                action = r;
            }
            return action;
        }

        virtual bool isThisActionDone(float beat) {
            return false;
        }

        virtual float getLengthBeats() {
            return getLengthOfScheduleInBeats() + getLengthOfRunningActionsInBeats();
        }

        virtual std::string getLabel() {
            return "Effect Generator";
        }

        virtual void draw(float beat) {}

    private:
        float minBeatsBetweenEffects = 5;
        float maxBeatsBetweenEffects = 10;
        float bpm, fps;
        ofxPm::VideoBuffer* buffer;
        ofxPm::VideoRate* rate;
        ofxPm::BasicVideoRenderer* renderer;
        ofxMaxiSample* forwardSample;
        ofxMaxiSample* backwardSample;
        ofxBenG::audio* audio;
    };
}

#endif /* beat_action_h */
