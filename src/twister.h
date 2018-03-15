#ifndef twister_h
#define twister_h

#include "ofxMidiFighterTwister.h"
#include "property_bag.h"

namespace ofxBenG {
    typedef std::function<void(int, float)> encoderbinding_t;
    class encoder {
    public:
        encoder(int index, int min, int max, encoderbinding_t updateTwister) : index(index), min(min), max(max) {}
        
        void setValue(int value) {
            this->value = value;
            updateBindings(value);
        }
        
        float getValue() {
            return value;
        }
        
        void setScale(float scale) {
            this->value = (int)(scale * max);
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

        void bind(encoderbinding_t binding) {
            bindings.push_back(binding);
        }

        void bind(property_base* property) {
            std::cout << "Binding " << property->getName() << " to encoder " << index << ". Initial value " << property->getScale() << std::endl;
            value = property->getScale() * max;
            bindings.push_back([=](int index, float midi) {
                property->setScale(midi / max);
            });
        }
        
        void updateBindings(float value) {
            for (auto binding : bindings) {
                binding(index, value);
            }
        }

        void updateBindings() {
            updateBindings(value);
        }
        
    private:
        std::vector<encoderbinding_t> bindings;
        int value;
        int min;
        int max;
        int index;
    };

    class twister {
    public:
        twister() {
            for (int i = 0; i < encoderCount; i++) {
                encoders[i] = new ofxBenG::encoder(i, 0, 127, [&](int encoder, float value) {
                    this->setEncoderRingValue(encoder, value);
                });
            }

            midiFighterTwister.setup();
            ofAddListener(midiFighterTwister.eventEncoder, this, &twister::onEncoderUpdate);
            ofAddListener(midiFighterTwister.eventPushSwitch, this, &twister::onPushSwitchUpdate);
            ofAddListener(midiFighterTwister.eventSideButton, this, &twister::onSideButtonPressed);
        }

        ~twister() {
            for (int i = 0; i < encoderCount; i++) {
                delete encoders[i];
            }
        }

        void setEncoderRingValue(int encoder, float value) {
            midiFighterTwister.setEncoderRingValue(encoder, value);
        }

        void setEncoderColor(int encoder, float hue) {
            midiFighterTwister.setEncoderColor(encoder, hue);
        }

        ofxMidiFighterTwister* getDevice() {
            return &midiFighterTwister;
        }

        void bindToSingleEncoder(property_bag* propertyBag) {
            propertyBag->apply([this](property_base* p) {
                this->bindToEncoder(usedRow, usedCol, usedBank, p);
            });
        }

        void bindToMultipleEncoders(property_bag* propertyBag) {
            propertyBag->apply([this](property_base* p) {
                this->bindToNextEncoder(p);
            });
        }

        void bindToNextEncoder(property_base* property) {
            int encoderIndex = (usedBank * rowsPerBank * columnsPerRow) + (usedRow * columnsPerRow) + usedCol;
            encoders[encoderIndex]->bind(property);
            usedCol++;
            if (usedCol > columnsPerRow) {
                usedCol = 0;
                usedRow++;
                if (usedRow > rowsPerBank) {
                    usedRow = 0;
                    usedBank++;
                }
            }
        }

        void bindToEncoder(int bank, int row, int column, property_base* property) {
            int encoderIndex = (bank * rowsPerBank * columnsPerRow) + (row * columnsPerRow) + column;
            encoders[encoderIndex]->bind(property);
        }

        void onEncoderUpdate(ofxMidiFighterTwister::EncoderEventArgs& a) {
            std::cout << "onEncoderUpdate(" << a.ID << "): to MIDI " << a.value << std::endl;
            encoders[a.ID]->setValue(a.value);
        }

        void onPushSwitchUpdate(ofxMidiFighterTwister::PushSwitchEventArgs& a) {
            std::cout << "onPushSwitchUpdate(" << a.ID << "): value=" << a.value << std::endl;
        }

        void onSideButtonPressed(ofxMidiFighterTwister::SideButtonEventArgs & a) {
            std::cout << "onSideButtonPressed(" << a.buttonID << ")" << std::endl;
        }

        static int relativeMidi(int midi) {
            return (midi == midiIncrease) ? 1 : -1;
        }
        
        static float mapMidi(int midi, float targetMin, float targetMax) {
            return ofMap(midi, midiMin, midiMax, targetMin, targetMax);
        }

        static int const bankCount = 4;
        static int const rowsPerBank = 4;
        static int const columnsPerRow = 4;
        static int const midiDecrease = 63;
        static int const midiIncrease = 65;
        static int const midiMin = 0;
        static int const midiMax = 127;
        static int const encoderCount = 64;
    private:
        ofxBenG::encoder* encoders[encoderCount];
        ofxMidiFighterTwister midiFighterTwister;
        int usedRow = 0;
        int usedCol = 0;
        int usedBank = 0;
    };
} /* ofxBenG */

#endif /* twister_h */
