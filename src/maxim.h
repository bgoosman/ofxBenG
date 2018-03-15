#ifndef MAXIM_H
#define MAXIM_H

#include "ofxMaxim.h"
#include "audio.h"

namespace ofxBenG {
    class maxim_stutter {
    public:
        maxim_stutter(ofxMaxiSample* sample, float triggerSeconds, float lengthSeconds, int times)
        : sample(sample), times(times) {
            double const sampleLengthSeconds = sample->getLengthSeconds();
            double const lengthPercent = lengthSeconds / sampleLengthSeconds;
            endPercent = MIN(1, triggerSeconds / sampleLengthSeconds);
            startPercent = MAX(0, endPercent - lengthPercent);
            std::cout << "new stutter @seconds=" << triggerSeconds << " {times=" << times << ", start=" << startPercent << ", end=" << endPercent << "}" << std::endl;
        }

        maxim_stutter(ofxMaxiSample* sample, float lengthSeconds, int times)
        : maxim_stutter(sample, sample->getPositionSeconds(), lengthSeconds, times) {}

        static maxim_stutter* make_random(ofxMaxiSample* sample) {
            float const currentSeconds = sample->getPositionSeconds();
            std::cout << "stutter, sampleCurrentSeconds=" << currentSeconds << std::endl;
            float const triggerSeconds = currentSeconds + ofRandom(minimumSecondsBetweenStutters, maximumSecondsBetweenStutters);
            float const lengthSeconds = ofRandom(minimumLengthSeconds, maximumLengthSeconds);
            int const times = int(ofRandom(minTimes, maxTimes));
            return new maxim_stutter(sample, triggerSeconds, lengthSeconds, times);
        }

        bool isDone() {
            return times <= 0;
        }

        void update() {
            if (sample->getPosition() >= endPercent) {
                std::cout << "stutter, sampleCurrentSeconds=" << sample->getPositionSeconds() << ", times=" << times << ", position=" << sample->getPosition() << std::endl;
                sample->setPosition(startPercent);
                times--;
            }
        }

    private:
        ofxMaxiSample* sample;
        float startPercent;
        float endPercent;
        int times;
        static float constexpr minimumSecondsBetweenStutters = 8;
        static float constexpr maximumSecondsBetweenStutters = 16;
        static float constexpr minimumLengthSeconds = 1.0 / 8.0;
        static float constexpr maximumLengthSeconds = 1.0 / 1.5;
        static int constexpr minTimes = 3;
        static int constexpr maxTimes = 8;
    };

    class maxim_reverse {
    public:
        maxim_reverse(ofxBenG::audio* audio, ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample, float lengthSeconds)
                : audio(audio), forwardSample(forwardSample), backwardSample(backwardSample), lengthSeconds(lengthSeconds) {
            backwardSample->setPosition(1 - forwardSample->getPosition());
            forwardSample->setPositionSeconds(forwardSample->getPositionSeconds() - lengthSeconds);
            targetSeconds = backwardSample->getPositionSeconds() + lengthSeconds;
            audio->setSample(backwardSample);
            std::cout << "reverse, seconds=" << lengthSeconds << std::endl;
        }

        ~maxim_reverse() {
            std::cout << "reverse, set current sample = forwardSample" << std::endl;
            audio->setSample(forwardSample);
        }

        static maxim_reverse* make_random(ofxBenG::audio* audio, ofxMaxiSample* forwardSample, ofxMaxiSample* backwardSample) {
            float lengthSeconds = ofRandom(5);
            return new maxim_reverse(audio, forwardSample, backwardSample, lengthSeconds);
        }

        bool isDone() {
            return backwardSample->getPositionSeconds() >= targetSeconds;
        }

        void update() {}

    private:
        ofxMaxiSample* forwardSample;
        ofxMaxiSample* backwardSample;
        float lengthSeconds;
        float targetSeconds;
        ofxBenG::audio* audio;
    };
};

#endif