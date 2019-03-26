#ifndef ETC_ELEMENT
#define ETC_ELEMENT

#include "ofxMidi.h"
#include "ofxOsc.h"

namespace ofxBenG {
    class etc_element_midi_proxy {
    public:
        void setup(std::string midiDeviceName);
        void setSubmaster(int faderNumber, int level);
    private:
        ofxMidiOut midiOut;
    };

    class etc_element_osc_proxy {
    public:
        void setup(std::string remoteIp, int remotePort);
        void setSubmaster(int faderNumber, float level);
        void setChannel(int faderNumber, int level);
    private:
        ofxOscSender oscSender;
    };
}

#endif
