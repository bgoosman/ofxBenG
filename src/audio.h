#ifndef ofxbengaudio_h
#define ofxbengaudio_h

#include <algorithm>
#include <functional>
#include <utility>
#include "ofxMaxim.h"

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
        audio() {}
        ~audio() {}

        void setSample(ofxMaxiSample *s) {
            sample = s;
        }

        void add(mix_t* mix) {
            std::cout << "adding mix" << std::endl;
            mixes.push_back(mix);
        }

        void remove(mix_t* mix) {
            std::cout << "removing mix" << std::endl;
            mixes.erase(std::remove(mixes.begin(), mixes.end(), mix), mixes.end());
        }

        float getMix() {
            float master = 0;
            for (auto& mix : mixes)
                master += (*mix)();
            if (sample != nullptr)
                master += sample->playOnce();
            return master;
        }

    private:
        ofxMaxiOsc oscillator;
        ofxMaxiSample* sample = nullptr;
        std::vector<mix_t*> mixes;
    };
};

#endif