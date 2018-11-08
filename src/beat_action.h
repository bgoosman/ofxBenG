#ifndef beat_action_h
#define beat_action_h

#include <algorithm>
#include <vector>
#include <queue>
#include "ofxPlaymodes.h"
#include "audio.h"
#include "beat_action.h"
#include "ease.h"
#include "maxim.h"
#include "utilities.h"

namespace ofxBenG {

    class beat_action;

    class beat_action_comparator {
    public:
        bool operator()(beat_action *first, beat_action *second);
    };

    class beat_action {
    public:
        beat_action();
        virtual ~beat_action();
        virtual std::string getLabel() = 0;
        virtual void startThisAction() = 0;
        virtual void updateThisAction() = 0;
        virtual bool isThisActionDone();
        virtual void schedule(float beatsFromNow, beat_action *action);
        virtual void schedule(float beatsFromNow, std::function<void()> action);
        virtual void start(float currentBeat);
        virtual void update(float currentBeat);
        virtual float getTriggerBeat();
        virtual void setTriggerBeat(float value);
        virtual bool isDone();

    protected:
        std::deque<ofxBenG::beat_action *> runningActions;
        std::priority_queue<ofxBenG::beat_action *, std::vector<ofxBenG::beat_action *>, ofxBenG::beat_action_comparator> scheduledActions;
        float currentBeat;

    private:
        virtual void updateRunningActions();
        virtual void queueTriggeredActions();
        bool isScheduleDone();
        float triggerBeat;
    };

    class record_action : public beat_action {
    public:
        record_action(ofxPm::VideoBuffer *buffer) : buffer(buffer) {
        }

        virtual void startThisAction() {
            buffer->resume();
        }

        virtual std::string getLabel() {
            return "Record";
        }

    private:
        ofxPm::VideoBuffer *buffer;
    };

    class stop_recording_action : public beat_action {
    public:
        stop_recording_action(ofxPm::VideoBuffer *buffer) : buffer(buffer) {
        }

        virtual void startThisAction() {
            buffer->stop();
        }

        virtual std::string getLabel() {
            return "Stop recording";
        }

    private:
        ofxPm::VideoBuffer *buffer;
    };

    class resume_recording_action : public beat_action {
    public:
        resume_recording_action(ofxPm::VideoBuffer *buffer) : buffer(buffer) {
        }

        virtual ~resume_recording_action() {
        };

        virtual void startThisAction() {
            buffer->resume();
        }

        virtual std::string getLabel() {
            return "Resume Recording";
        }

    private:
        ofxPm::VideoBuffer *buffer;
    };

    class play_from_beginning_action : public beat_action {
    public:
        play_from_beginning_action(ofxPm::VideoHeader *header) : header(header) {
        }

        virtual ~play_from_beginning_action() {
        };

        virtual void startThisAction() {
            header->setLoopToStart();
            header->setPlaying(true);
        }

        virtual std::string getLabel() {
            return "Play From Beginning";
        }

    private:
        ofxPm::VideoHeader *header;
    };

    class pan_video : public beat_action {
    public:
        pan_video(ofxPm::VideoHeader *header, float originalLengthBeats,
                float targetLengthBeats, float bpm, float fps)
                : pan_video(header, originalLengthBeats, targetLengthBeats, bpm, fps, PLAY_FORWARDS) {
        }

        pan_video(ofxPm::VideoHeader *header, float originalLengthBeats,
                float targetLengthBeats, float bpm, float fps, bool playForwards)
                : header(header), originalLengthBeats(originalLengthBeats),
                  targetLengthBeats(targetLengthBeats), playForwards(playForwards), bpm(bpm), fps(fps) {
        }

        virtual void startThisAction() {
            delayFrames = originalLengthBeats * (60 / bpm) * fps;
            speed = originalLengthBeats / targetLengthBeats;
            startBeat = currentBeat;
            header->setDelayFrames(playForwards ? delayFrames : 0);
            std::cout << currentBeat << ": startPlayingBeat=" << startBeat
                      << ", originalLengthBeats=" << originalLengthBeats
                      << ", targetLengthBeats=" << targetLengthBeats
                      << std::endl;
        }

        virtual void updateThisAction() {
            float amount = (currentBeat - startBeat) / targetLengthBeats;
            float lerp = ofLerp(0, delayFrames, amount);
            float frames = (playForwards) ? delayFrames - lerp : lerp;
            header->setDelayFrames(frames);
        }

        virtual bool isThisActionDone() {
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
        ofxPm::VideoHeader *header;
    };

    class flicker : public beat_action, public window_view {
    public:
        flicker(ofxBenG::video_stream *stream, float blackoutLengthBeats, float videoLengthBeats, ofxBenG::audio *audio);
        ~flicker();
        virtual void draw(ofPoint windowSize);
        virtual void startThisAction();
        virtual void updateThisAction();
        virtual bool isThisActionDone();
        virtual std::string getLabel();

    private:
        ofxBenG::audio *audio;
        ofxMaxiSample *sample;
        ofxBenG::video_stream *stream;
        ofxPm::VideoBuffer *buffer;
        ofxPm::VideoHeader *header;
        ofxPm::BasicVideoRenderer *renderer;
        float blackoutLengthBeats;
        float videoLengthBeats;
        float recordingFps;
        bool isPlaying;
        bool isBlackout;
    };

    class reverse_audio : public beat_action {
    public:
        reverse_audio(ofxBenG::audio *audio, ofxMaxiSample *forwardSample, ofxMaxiSample *backwardSample, float lengthBeats, float bpm)
                : audio(audio),
                  forwardSample(forwardSample),
                  backwardSample(backwardSample),
                  lengthBeats(lengthBeats),
                  bpm(bpm) {
        }

        ~reverse_audio() {
            delete reverse;
        }

        virtual void startThisAction() {
            float lengthSeconds = ofxBenG::utilities::beatsToSeconds(lengthBeats, bpm);
            reverse = new maxim_reverse(audio, forwardSample, backwardSample, lengthSeconds);
        }

        virtual void updateThisAction() {
            reverse->update();
        }

        virtual bool isThisActionDone() {
            return reverse->isDone();
        }

        virtual std::string getLabel() {
            return "Reverse Audio";
        }

    private:
        ofxBenG::maxim_reverse *reverse;
        ofxMaxiSample *forwardSample;
        ofxMaxiSample *backwardSample;
        float bpm;
        float lengthBeats;
        ofxBenG::audio *audio;
    };

    class stutter_audio : public beat_action {
    public:
        stutter_audio(ofxMaxiSample *sample, float lengthBeats, float bpm)
                : sample(sample), lengthBeats(lengthBeats), bpm(bpm) {
        }

        ~stutter_audio() {
            delete stutter;
        }

        virtual void startThisAction() {
            float lengthSeconds = lengthBeats * (1 / (bpm / 60));
            stutter = new maxim_stutter(sample, lengthSeconds, 1);
        }

        virtual void updateThisAction() {
            stutter->update();
        }

        virtual bool isThisActionDone() {
            return stutter->isDone();
        }

        virtual std::string getLabel() {
            return "Pan Audio";
        }

    private:
        ofxBenG::maxim_stutter *stutter;
        ofxMaxiSample *sample;
        float bpm;
        float lengthBeats;
    };

    class play_tone : public beat_action {
    public:
        play_tone(ofxBenG::audio *audio, float durationBeats, float frequency)
                : audio(audio), durationBeats(durationBeats), frequency(frequency) {
            tone = new mix_t([this]() {
                return this->oscillator.sinewave(this->frequency) / 3;
            });
        }

        virtual ~play_tone() {
            audio->remove(tone);
        };

        virtual void startThisAction() {
            startBeat = currentBeat;
            audio->add(tone);
        }

        virtual bool isThisActionDone() {
            return currentBeat >= startBeat + durationBeats;
        }

        virtual std::string getLabel() {
            return ofToString("Play Tone")
                    + "{frequency: " + ofToString(frequency)
                    + ", startPlayingBeat: " + ofToString(startBeat)
                    + ", durationBeats: " + ofToString(durationBeats) + "}";
        }

    private:
        float durationBeats;
        float startBeat;
        float frequency;
        ofxBenG::mix_t *tone;
        ofxBenG::audio *audio;
        ofxMaxiOsc oscillator;
    };

    class generic_action : public beat_action {
    public:
        generic_action(std::function<void()> action) : beat_action(), action(action) {
        }

        virtual void startThisAction() {
            action();
        }

        virtual void updateThisAction() {

        }

        virtual std::string getLabel() {
            return "Generic Action";
        }

    private:
        std::function<void()> action;
    };

    class timeline : public beat_action {
    public:
        timeline() : beat_action() {
        };

        virtual void startThisAction() {

        }

        virtual void updateThisAction() {

        }

        virtual bool isThisActionDone() {
            return false;
        }

        virtual std::string getLabel() {
            return "Timeline";
        }
    };
};

#endif /* beat_action_h */
