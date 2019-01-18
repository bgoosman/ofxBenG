#import "ofxMidi.h"

#ifndef ETC_ELEMENT
#define ETC_ELEMENT

namespace ofxBenG {
    class etc_element {
    public:
        void setup(std::string midiDeviceName);
        void setSubmaster(int faderNumber, int level);
    private:
        ofxMidiOut midiOut;
    };
}

#endif