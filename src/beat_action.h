//
//  beat_action.h
//  Stutter
//
//  Created by MacBook Pro on 2/4/18.
//

#ifndef beat_action_h
#define beat_action_h

#include "ofxBenG.h"
#include "VideoBuffer.h"
#include "VideoHeader.h"
#include "VideoRate.h"
#include "maxim.h"
#include "ease.h"
#include "beat_action.h"
#include "playmodes.h"
#include "audio.h"
#include <algorithm>
#include <vector>
#include <queue>

namespace ofxBenG {

    class beat_action;
    class beat_action_comparator {
    public:
        bool operator() (beat_action* first, beat_action* second);
    };

    class beat_action {
    public:
        beat_action() {}
        virtual ~beat_action() {}
        virtual std::string getLabel() = 0;
        virtual void setupThisAction(float currentBeat) = 0;
        virtual void updateThisAction(float currentBeat) = 0;
        virtual void startThisAction(float currentBeat) = 0;
        virtual bool isThisActionDone(float currentBeat) = 0;
        virtual float getTriggerBeat();
        virtual void setup(float currentBeat);
        virtual void start(float currentBeat);
        virtual void update(float currentBeat);
        virtual void schedule(beat_action* action, float currentBeat, float beatsFromNow);
        virtual void setTriggerBeat(float value);
        virtual bool isTriggered(float currentBeat);
        virtual bool isDone(float currentBeat);

    protected:
        std::deque<ofxBenG::beat_action*> runningActions;
        std::priority_queue<ofxBenG::beat_action*, std::vector<ofxBenG::beat_action*>, ofxBenG::beat_action_comparator> scheduledActions;

    private:
        virtual void updateRunningActions(float currentBeat);
        virtual void queueTriggeredActions(float currentBeat);
        bool scheduleIsDone();

        float triggerBeat;
    };


    class record_action : public beat_action {
    public:
        record_action(ofxPm::VideoBuffer* buffer) : buffer(buffer) {}
        virtual ~record_action() {};

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float currentBeat) {
            buffer->resume();
        }

        virtual void updateThisAction(float currentBeat) {}

        virtual bool isThisActionDone(float currentBeat) {
            return true;
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

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float beat) {
            buffer->stop();
        }

        virtual void updateThisAction(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual std::string getLabel() {
            return "Stop recording";
        }

    private:
        ofxPm::VideoBuffer* buffer;
    };

    class resume_recording_action : public beat_action {
    public:
        resume_recording_action(ofxPm::VideoBuffer* buffer) : buffer(buffer) {}
        virtual ~resume_recording_action() {};

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float beat) {
            buffer->resume();
        }

        virtual void updateThisAction(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
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

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float beat) {
            header->setLoopToStart();
            header->setPlaying(true);
        }

        virtual void updateThisAction(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual std::string getLabel() {
            return "Play From Beginning";
        }

    private:
        ofxPm::VideoHeader* header;
    };

    class pan_video : public beat_action {
    public:
        pan_video(ofxPm::VideoHeader* header, ofxBenG::playmodes* playModes, float originalLengthBeats,
                  float targetLengthBeats, float bpm, float fps)
                : pan_video(header, playModes, originalLengthBeats, targetLengthBeats, bpm, fps, true) {}

        pan_video(ofxPm::VideoHeader* header, ofxBenG::playmodes* playModes, float originalLengthBeats,
                  float targetLengthBeats, float bpm, float fps, bool playForwards)
                : header(header), playModes(playModes), originalLengthBeats(originalLengthBeats),
                  targetLengthBeats(targetLengthBeats), playForwards(playForwards), bpm(bpm), fps(fps) {}

        virtual void setupThisAction(float currentBeat) {
            delayFrames = originalLengthBeats * (60 / bpm) * fps;
            speed = originalLengthBeats / targetLengthBeats;
        }

        virtual void startThisAction(float currentBeat) {
            startBeat = currentBeat;
            header->setDelayFrames(playForwards ? delayFrames : 0);
            std::cout << currentBeat << ": startBeat=" << startBeat
                      << ", originalLengthBeats=" << originalLengthBeats
                      << ", targetLengthBeats=" << targetLengthBeats
                      << std::endl;
        }

        virtual void updateThisAction(float currentBeat) {
            float amount = (currentBeat - startBeat) / targetLengthBeats;
            float lerp = ofLerp(0, delayFrames, amount);
            float frames = (playForwards) ? delayFrames - lerp : lerp;
            header->setDelayFrames(frames);
        }

        virtual bool isThisActionDone(float currentBeat) {
            return currentBeat >= startBeat + targetLengthBeats;
        }

        virtual std::string getLabel() {
            return "Pan Video";
        }

        static bool const PLAY_FORWARDS = true;
        static bool const PLAY_BACKWARDS = false;
    private:
        float startBeat;
        float originalLengthBeats;
        float targetLengthBeats;
        float delayFrames;
        float speed;
        float bpm;
        float fps;
        bool playForwards;
        ofxPm::VideoHeader* header;
        ofxBenG::playmodes* playModes;
    };

    class reverse_audio : public beat_action {
    public:
        reverse_audio(ofxBenG::audio* audio, ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample, float lengthBeats, float bpm)
                : audio(audio), forwardSample(forwardSample), backwardSample(backwardSample), lengthBeats(lengthBeats), bpm(bpm) {}

        ~reverse_audio() {
            delete reverse;
        }

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float beat) {
            float lengthSeconds = ofxBenG::utilities::beatsToSeconds(lengthBeats, bpm);
            reverse = new maxim_reverse(audio, forwardSample, backwardSample, lengthSeconds);
        }

        virtual void updateThisAction(float beat) {
            reverse->update();
        }

        virtual bool isThisActionDone(float beat) {
            return reverse->isDone();
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

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float beat) {
            float lengthSeconds = lengthBeats * (1 / (bpm / 60));
            stutter = new maxim_stutter(sample, lengthSeconds, 1);
        }

        virtual void updateThisAction(float beat) {
            stutter->update();
        }

        virtual bool isThisActionDone(float beat) {
            return stutter->isDone();
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

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float currentBeat) {
            startBeat = currentBeat;
            audio->add(tone);
        }

        virtual void updateThisAction(float currentBeat) {}

        virtual bool isThisActionDone(float currentBeat) {
            return currentBeat >= startBeat + durationBeats;
        }

        virtual std::string getLabel() {
            return ofToString("Play Tone")
                    + "{frequency: " + ofToString(frequency)
                    + ", startBeat: " + ofToString(startBeat)
                    + ", durationBeats: " + ofToString(durationBeats) + "}";
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
        stutter(float currentBeat, float recordLengthBeats, float stutterLengthBeats, float bpm, int stutterTimes,
                ofxBenG::playmodes* playModes, ofxMaxiSample* sample, ofxBenG::audio* audio)
                : beat_action(), playModes(playModes), recordLengthBeats(recordLengthBeats), stutterTimes(stutterTimes),
                  stutterLengthBeats(stutterLengthBeats), bpm(bpm), sample(sample), audio(audio) {}

        virtual void setupThisAction(float currentBeat) {
            VideoBuffer* buffer = playModes->getBuffer(0);
            myHeader = new VideoHeader();
            myHeader->setup(*buffer);
            oldHeader = playModes->getPlayHeader();
            schedule(new record_action(buffer), currentBeat, 0);
            schedule(new stop_recording_action(buffer), currentBeat, recordLengthBeats);
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
            schedule(new play_tone(audio, firstToneDurationBeats, firstNote), currentBeat, nextTone);
            float nextStutter = recordLengthBeats;
            float const secondToneDurationBeats = 0.25;
            std::vector<float> const scale = {noteA4, noteASharp4, noteB4, noteC5, noteD5, noteDSharp5, noteE5,
                    noteF5, noteFSharp5, noteG5, noteGSharp5};
            for (int i = 0; i < stutterTimes; i++, nextStutter += stutterLengthBeats) {
                float const stutterNote = scale[stutterTimes - i];
                schedule(new play_tone(audio, secondToneDurationBeats, stutterNote), currentBeat, nextStutter);
                schedule(new pan_video(myHeader, playModes, recordLengthBeats, stutterLengthBeats, bpm, playModes->getFps(0)), currentBeat, nextStutter);
                schedule(new stutter_audio(sample, stutterLengthBeats, bpm), currentBeat, nextStutter);
            }
            schedule(new resume_recording_action(buffer), currentBeat, nextStutter);
        }

        static stutter* make_random(float currentBeat, float bpm, float recordLengthBeats, float stutterLengthBeats,
                ofxBenG::playmodes* playModes, ofxMaxiSample* sample, ofxBenG::audio* audio) {
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
            return new stutter(currentBeat, recordLengthBeats, stutterLengthBeats, bpm, stutterTimes, playModes, sample, audio);
        }

        static stutter* make_random(float currentBeat, float bpm, ofxBenG::playmodes* playModes, ofxMaxiSample* sample, ofxBenG::audio* audio) {
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
            return new stutter(currentBeat, recordLengthBeats, stutterLengthBeats, bpm, stutterTimes, playModes, sample, audio);
        }

        ~stutter() {
            playModes->setPlayHeader(oldHeader);
            delete myHeader;
        }

        virtual void startThisAction(float currentBeat) {
            playModes->setPlayHeader(myHeader);
        }

        virtual void updateThisAction(float currentBeat) {}

        virtual bool isThisActionDone(float currentBeat) {
            return true;
        }

        virtual std::string getLabel() {
            return "Stutter {recordLengthBeats: " + ofToString(recordLengthBeats)
                    + ", stutterLengthBeats: " + ofToString(stutterLengthBeats)
                    + ", stutterTimes: " + ofToString(stutterTimes) + "}";
        }

        bool isPlaying() {
            return myHeader->isPlaying();
        }

    private:
        ofxPm::VideoHeader* myHeader;
        ofxPm::VideoHeader* oldHeader;
        ofxBenG::playmodes* playModes;
        ofxBenG::audio* audio;
        ofxMaxiSample* sample;
        float recordLengthBeats, stutterLengthBeats, bpm;
        int stutterTimes;
    };

    class rewind : public beat_action {
    public:
        rewind(float currentBeat, float recordLengthBeats, float stutterLengthBeats, float bpm,
                ofxBenG::playmodes* playModes,
                ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample, ofxBenG::audio* audio)
                : beat_action(), playModes(playModes),
                  recordLengthBeats(recordLengthBeats), stutterLengthBeats(stutterLengthBeats), bpm(bpm),
                  forwardSample(forwardSample), backwardSample(backwardSample), audio(audio) {}

        ~rewind() {
            playModes->setPlayHeader(oldHeader);
            delete myHeader;
        }

        virtual void setupThisAction(float currentBeat) {
            VideoBuffer* buffer = playModes->getBuffer(0);
            myHeader = new VideoHeader();
            myHeader->setup(*buffer);
            oldHeader = playModes->getPlayHeader();
            schedule(new record_action(buffer), currentBeat, 0);
            schedule(new stop_recording_action(buffer), currentBeat, recordLengthBeats);
            schedule(new play_tone(audio, firstToneDurationBeats, noteA), currentBeat, 0);
            schedule(new play_tone(audio, secondToneDurationBeats, noteE5), currentBeat, recordLengthBeats - secondToneDurationBeats);
            schedule(new pan_video(myHeader, playModes, recordLengthBeats, stutterLengthBeats, bpm, playModes->getFps(0), pan_video::PLAY_BACKWARDS), currentBeat, recordLengthBeats);
            schedule(new reverse_audio(audio, forwardSample, backwardSample, stutterLengthBeats, bpm), currentBeat, recordLengthBeats);
            schedule(new resume_recording_action(buffer), currentBeat, recordLengthBeats + stutterLengthBeats);
        }

        static rewind* make_random(float currentBeat, float bpm, float recordLengthBeats, float rewindLengthBeats,
                ofxBenG::playmodes* playModes, ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample, ofxBenG::audio* audio) {
            return new rewind(currentBeat, recordLengthBeats, rewindLengthBeats, bpm, playModes, forwardSample, backwardSample, audio);
        }

        static rewind* make_random(float currentBeat, float bpm, ofxBenG::playmodes* playModes,
                ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample, ofxBenG::audio* audio) {
            float const recordLengthBeats = 6;
            float const rewindLengthBeats = ofRandom(3, 6);
            return new rewind(currentBeat, recordLengthBeats, rewindLengthBeats, bpm, playModes, forwardSample, backwardSample, audio);
        }

        virtual void startThisAction(float beat) {
            playModes->setPlayHeader(myHeader);
        }

        virtual void updateThisAction(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual std::string getLabel() {
            return "Reverse";
        }

        bool isPlaying() {
            return myHeader->isPlaying();
        }

    private:
        ofxPm::VideoHeader* myHeader;
        ofxPm::VideoHeader* oldHeader;
        ofxBenG::playmodes* playModes;
        ofxBenG::audio* audio;
        ofxMaxiSample* forwardSample;
        ofxMaxiSample* backwardSample;
        float recordLengthBeats, stutterLengthBeats, bpm;
        float const firstToneDurationBeats = 0.25;
        float const secondToneDurationBeats = 0.25;
        float const noteA = 440, noteE5 = 659.25;
    };

    class generic_action : public beat_action {
    public:
        generic_action(std::function<void()> action) : beat_action(), action(action) {}
        virtual ~generic_action() {}

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float currentBeat) {
            action();
        }

        virtual void updateThisAction(float currentBeat) {}

        virtual bool isThisActionDone(float currentBeat) {
            return true;
        }

        virtual std::string getLabel() {
            return "Generic Action";
        }
    private:
        std::function<void()> action;
    };

    class set_integer_action : public beat_action {
    public:
        set_integer_action(float currentBeat, int* store, int value) : beat_action(), store(store), value(value) {}
        virtual ~set_integer_action() {}

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float currentBeat) {
            *store = value;
        }

        virtual void updateThisAction(float currentBeat) {}

        virtual bool isThisActionDone(float currentBeat) {
            return true;
        }

        virtual std::string getLabel() {
            return "Set integer to " + ofToString(value);
        }
    private:
        int* store;
        int value;
    };

    class sample_sequence : public beat_action {
    public:
        sample_sequence() {};
        virtual ~sample_sequence();

        void append(ofxMaxiSample* sample, float seconds) {

        }

        virtual void setupThisAction(float currentBeat) {

        }

        virtual void startThisAction(float currentBeat) {

        }

        virtual void updateThisAction(float currentBeat) {

        }

        virtual bool isThisActionDone(float currentBeat) {

        }

        virtual std::string getLabel() {
            return "sample_sequence";
        }

    private:
        std::vector<ofxMaxiSample*> samples;
    };

    class telephone_switch : public beat_action {
    public:
        telephone_switch(ofEvent<void>& onTelephoneStarted, int* recordingIndex,
                         int wasRecordingIndex, int nowRecordingIndex, ofxBenG::playmodes* playModes,
                         float loopLengthSeconds, float bpm, ofxBenG::audio* audio,
                         ofxMaxiSample* beep, ofxMaxiSample* click)
                : beat_action(), playModes(playModes), onTelephoneStarted(onTelephoneStarted),
                  recordingIndex(recordingIndex), wasRecordingIndex(wasRecordingIndex),
                  nowRecordingIndex(nowRecordingIndex), loopLengthSeconds(loopLengthSeconds), bpm(bpm),
                  audio(audio), beep(beep), click(click) {};
        virtual ~telephone_switch() {}

        virtual void setupThisAction(float currentBeat) {
            *recordingIndex = nowRecordingIndex;
            auto recordedBuffer = playModes->getBuffer(wasRecordingIndex);
            auto nowRecordingBuffer = playModes->getBuffer(nowRecordingIndex);
            auto loopLengthBeats = ofxBenG::utilities::secondsToBeats(loopLengthSeconds, bpm);
            schedule(new stop_recording_action(recordedBuffer), currentBeat, 0);
            schedule(new generic_action([&]() {
                if (wasRecordingIndex % 2 == 0) {
                    playModes->swapVideoDevice(ofxBenG::playmodes::c615);
                } else {
                    playModes->swapVideoDevice(ofxBenG::playmodes::c920);
                }
            }), currentBeat, 0);
            schedule(new record_action(nowRecordingBuffer), currentBeat, 0);

            recordedBufferHeader = new VideoHeader();
            recordedBufferHeader->setup(*recordedBuffer);
            playModes->setHeader(wasRecordingIndex, recordedBufferHeader);
            schedule(new pan_video(recordedBufferHeader, playModes, loopLengthBeats, loopLengthBeats, bpm,
                    playModes->getFps(wasRecordingIndex), ofxBenG::pan_video::PLAY_FORWARDS),
                    currentBeat, 0);

            auto wasRecordingMinusOne = (wasRecordingIndex + playModes->getBufferCount() - 1) % playModes->getBufferCount();
            recordedMinusOneBufferHeader = new VideoHeader();
            recordedMinusOneBufferHeader->setup(*playModes->getBuffer(wasRecordingMinusOne));
            playModes->setHeader(wasRecordingMinusOne, recordedMinusOneBufferHeader);
            schedule(new pan_video(recordedMinusOneBufferHeader, playModes, loopLengthBeats, loopLengthBeats, bpm,
                    playModes->getFps(wasRecordingMinusOne), ofxBenG::pan_video::PLAY_FORWARDS),
                    currentBeat, 0);

            float paddingSeconds = 0.75;
            float secondsFromNow = loopLengthSeconds - paddingSeconds * 4;
            float beatsFromNow = ofxBenG::utilities::secondsToBeats(secondsFromNow, bpm);
            int beepCount = 3;
            for (int i = 0; i < beepCount; i++) {
                if (i == 0) {
                    schedule(new generic_action([&]() {
                        audio->playSample(beep);
                    }), currentBeat, beatsFromNow);
                }
                secondsFromNow += paddingSeconds;
                beatsFromNow = ofxBenG::utilities::secondsToBeats(secondsFromNow, bpm);
            }
            schedule(new generic_action([&]() {
                audio->playSample(click);
            }), currentBeat, beatsFromNow);
        }

        virtual void startThisAction(float beat) {
            ofNotifyEvent(onTelephoneStarted);
        }

        virtual void updateThisAction(float beat) {}

        virtual bool isThisActionDone(float beat) {
            return true;
        }

        virtual std::string getLabel() {
            return "telephone_switch";
        }

    private:
        ofxPm::VideoHeader* recordedBufferHeader;
        ofxPm::VideoHeader* recordedMinusOneBufferHeader;
        ofxBenG::playmodes* playModes;
        ofxMaxiSample* beep;
        ofxMaxiSample* click;
        ofxBenG::audio* audio;
        ofEvent<void>& onTelephoneStarted;
        int wasRecordingIndex, nowRecordingIndex;
        int* recordingIndex;
        float bpm;
        float loopLengthSeconds;
    };

    class telephone : public beat_action {
    public:
        telephone(float currentBeat, ofxBenG::property<float>& bpm, ofxBenG::property<float>& loopLengthSeconds,
                  ofxBenG::playmodes* playModes, ofxBenG::audio* audio, ofxMaxiSample* beep, ofxMaxiSample* click)
                : beat_action(), bpm(bpm), loopLengthSeconds(loopLengthSeconds), playModes(playModes), audio(audio),
                  beep(beep), click(click) {}
        virtual ~telephone() {}

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float currentBeat) {
            auto loopLengthBeats = ofxBenG::utilities::secondsToBeats(loopLengthSeconds, bpm);
            auto recordingBuffer = playModes->getBuffer(recordingIndex);
            schedule(new record_action(recordingBuffer), currentBeat, 0);
            schedule(new telephone_switch(onTelephoneStarted, &recordingIndex, recordingIndex, getNextRecordingIndex(), playModes, loopLengthSeconds, bpm, audio, beep, click), currentBeat, loopLengthBeats);
        }

        virtual void updateThisAction(float currentBeat) {
            if (scheduledActions.size() == 0 && runningActions.size() == 0) {
                schedule(new telephone_switch(onTelephoneStarted, &recordingIndex, recordingIndex, getNextRecordingIndex(), playModes, loopLengthSeconds, bpm, audio, beep, click), currentBeat, 0);
            }
        }

        virtual bool isThisActionDone(float currentBeat) {
            return false;
        }

        virtual std::string getLabel() {
            return "Telephone";
        }

        int getNextRecordingIndex() {
            return (recordingIndex + 1) % playModes->getBufferCount();
        }

        int getRecordingIndex() {
            return recordingIndex;
        }

        ofEvent<void> onTelephoneStarted;
    private:
        ofxBenG::property<float>& bpm;
        ofxBenG::property<float>& loopLengthSeconds;
        ofxBenG::playmodes* playModes;
        ofxBenG::audio* audio;
        int recordingIndex = 0;
        ofxMaxiSample* beep;
        ofxMaxiSample* click;
    };

    class effect_generator : public beat_action {
    public:
        effect_generator(float currentBeat, ofxBenG::property<float>& bpm, ofxBenG::property<float>& recordLengthBeats,
                ofxBenG::property<float>& rewindLengthBeats, ofxBenG::property<float>& stutterLengthBeats,
                ofxBenG::playmodes* playModes, ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample,
                ofxBenG::audio* audio)
                : beat_action(), bpm(bpm), recordLengthBeats(recordLengthBeats),
                  rewindLengthBeats(rewindLengthBeats), stutterLengthBeats(stutterLengthBeats),
                  playModes(playModes), forwardSample(forwardSample), backwardSample(backwardSample), audio(audio),
                  totalEffectsScheduled(0) {};
        virtual ~effect_generator() {}

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float currentBeat) {
            audio->setSample(forwardSample);
            scheduleRandomEffect(currentBeat);
        }

        virtual void updateThisAction(float currentBeat) {
            if (scheduledActions.size() == 0 && runningActions.size() == 0) {
                scheduleRandomEffect(currentBeat);
            }
        }

        virtual bool isThisActionDone(float currentBeat) {
            return false;
        }

        virtual std::string getLabel() {
            return "Effect Generator";
        }

        void scheduleRandomEffect(float currentBeat) {
            float const beatsFromNow = ofRandom(minBeatsBetweenEffects, maxBeatsBetweenEffects);
            float const futureBeat = currentBeat + beatsFromNow;
            schedule(makeRandomEffect(futureBeat), currentBeat, beatsFromNow);
            ofNotifyEvent(onEffectScheduled, ++totalEffectsScheduled);
        }

        beat_action* makeRandomEffect(float currentBeat) {
            beat_action* action = nullptr;
            float r = ofRandom(1);
            if (r < 0.25) {
                action = stutter::make_random(currentBeat, bpm, recordLengthBeats, stutterLengthBeats, playModes, forwardSample, audio);
            } else {
                action = rewind::make_random(currentBeat, bpm, recordLengthBeats, rewindLengthBeats, playModes, forwardSample, backwardSample, audio);
            }
            return action;
        }

        ofEvent<int> onEffectScheduled;
        int totalEffectsScheduled;
    private:
        float minBeatsBetweenEffects = 2.999;
        float maxBeatsBetweenEffects = 3.001;
        ofxBenG::property<float>& bpm;
        ofxBenG::property<float>& recordLengthBeats;
        ofxBenG::property<float>& rewindLengthBeats;
        ofxBenG::property<float>& stutterLengthBeats;
        ofxMaxiSample* forwardSample;
        ofxMaxiSample* backwardSample;
        ofxBenG::playmodes* playModes;
        ofxBenG::audio* audio;
    };

    class timeline : public beat_action {
    public:
        timeline() : beat_action() {};
        virtual ~timeline() {}

        virtual void setupThisAction(float currentBeat) {}

        virtual void startThisAction(float currentBeat) {}

        virtual void updateThisAction(float currentBeat) {}

        virtual bool isThisActionDone(float currentBeat) {
            return false;
        }

        virtual std::string getLabel() {
            return "Timeline";
        }
    };
};

#endif /* beat_action_h */
