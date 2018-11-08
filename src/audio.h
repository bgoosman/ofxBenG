#ifndef ofxbengaudio_h
#define ofxbengaudio_h

#include <algorithm>
#include <functional>
#include <utility>
#include "ofxMaxim.h"
#include "property.h"

namespace ofxBenG {
    typedef std::function<float()> MixFunction;
    struct mix_t {
        mix_t(MixFunction f) : f_ {std::move(f)} {}
        MixFunction f_;
        float operator()() {
            return f_();
        }
    };

    class audio {
    public:
        audio() {
            ofxMaxiSettings::setup(rate, channels, bufferSize);
        }
        audio(int rate, int channels, int bufferSize) {
            ofxMaxiSettings::setup(rate, channels, bufferSize);
        }
        ~audio() {}

        void playSample(ofxMaxiSample *sample) {
            sample->setPosition(0);
            samples.push_back(sample);
        }

        void setSample(ofxMaxiSample *s) {
            playSample(s);
        }

        void add(mix_t* mix) {
            mixes.push_back(mix);
        }

        void remove(mix_t* mix) {
            mixes.erase(std::remove(mixes.begin(), mixes.end(), mix), mixes.end());
        }

        float getMix() {
            float master = 0;
            for (auto& mix : mixes)
                master += (*mix)();

            std::vector<ofxMaxiSample*> finishedSamples;
            for (ofxMaxiSample* sample : samples) {
                master += sample->playOnce() * volume;
                if (sample->isDone()) {
                    finishedSamples.push_back(sample);
                }
            }
            for (ofxMaxiSample* sample : finishedSamples) {
                samples.erase(std::remove(samples.begin(), samples.end(), sample), samples.end());
            }

            return master;
        }

        float getVolume() {
            return volume;
        }

        float setVolume(float value) {
            volume = value;
        }

        ofxBenG::property<float>& getVolumeReference() {
            return volume;
        }

        int getSampleRate() {
            return rate;
        }

        int getBufferSize() {
            return bufferSize;
        }

        static const int channels = 2;
        static const int bufferSize = 512;
        static const int rate = 44100;

    private:
        std::vector<mix_t*> mixes;
        std::vector<ofxMaxiSample*> samples;
        ofxBenG::property<float> volume = {"volume", 0.75, 0.0, 1.0};
    };
};

#endif