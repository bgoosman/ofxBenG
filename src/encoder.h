#ifndef encoder_h
#define encoder_h

#include "ofxMidiFighterTwister.h"
#include "property.h"

namespace ofxBenG {

typedef std::function<void(float)> encoderbinding_t;

class encoder {
public:
    encoder(int index, int min, int max, ofxMidiFighterTwister* twister) : index(index), min(min), max(max), twister(twister) {}

    void setValue(int value) {
        this->value = value;
        twister->setEncoderRingValue(index, getScale());
        updateBindings();
    }
    
    float getValue() {
        return value;
    }
    
    void setScale(float scale) {
        this->value = (int)(scale * max);
        twister->setEncoderRingValue(index, scale);
        updateBindings(scale);
    }
    
    float getScale() {
        return ((float)(value - min) / (float)(max - min));
    }
    
    int getMin() {
        return min;
    }
    
    int getMax() {
        return max;
    }

    void bind(property_base* property) {
        std::cout << "Binding " << property->getName() << " to encoder " << index << ". Initial value " << property->getScale() << std::endl;
        value = property->getScale() * max;
        twister->setEncoderRingValue(index, property->getScale());
        twister->setEncoderColor(index, 0.25);
        bindings.push_back([=](float scale) {
            property->setScale(scale);
        });
    }
    
    void updateBindings(float value) {
        for (auto binding : bindings) {
            binding(value);
        }
    }
    
    void updateBindings() {
        for (auto binding : bindings) {
            float scale = ofxMidiFighterTwister::mapMidi(getValue(), 0, 1);
            binding(scale);
        }
    }
    
private:
    ofxMidiFighterTwister* twister;
    std::vector<encoderbinding_t> bindings;
    int value;
    int min;
    int max;
    int index;
};

} // ofxBenG

#endif /* encoder_h */