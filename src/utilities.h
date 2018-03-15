#ifndef ofxbengutilities_h
#define ofxbengutilities_h

namespace ofxBenG {
    class utilities {
    public:
        static void drawLabelValue(const std::string& label, float value, float y) {
            float const x = 15;
            std::string s = label + ofToString(value);
            ofDrawBitmapString(s, x, y);
        }

        static float beatsToSeconds(float beats, float bpm) {
            return beats * (1 / bpm) * 60;
        }

        static bool closeToInteger(float value) {
            double intPart, fractionalPart;
            fractionalPart = modf(value, &intPart);
            return value > 0.95 || value < 0.05;
        }
    };
}

#endif /* ofxbengutilities_h */
