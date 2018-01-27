//
//  oscWrapper.h
//  JumpRope
//
//  Created by MacBook Pro on 12/29/17.
//

#ifndef oscWrapper_h
#define oscWrapper_h

#include "ofxOsc.h"

namespace ofxBenG {
    
class osc {
public:
    osc(const std::string& ipAddress, int port) {
        oscSender.setup(ipAddress, port);
    }
    
    void send(const std::string& channel, float value) {
        ofxOscMessage message;
        message.setAddress(channel);
        message.addFloatArg(value);
        oscSender.sendMessage(message);
    }
    
private:
    ofxOscSender oscSender;
};
    
}

#endif /* oscWrapper_h */
