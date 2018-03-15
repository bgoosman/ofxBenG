#ifndef syphon_h
#define syphon_h

#include "ofxSyphonServer.h"

namespace ofxBenG {
    class syphon {
    public:
        syphon(int serverCount) {
            servers.resize(serverCount);
            for (int i = 0; i < serverCount; i++) {
                servers[i] = new ofxSyphonServer();
                servers[i]->setName(ofToString(i));
            }
        }
        ~syphon() {
            for (auto* server : servers) {
                delete server;
            }
        }

        void publishTexture(int i, ofTexture* texture) {
            servers[i]->publishTexture(texture);
        }

    private:
        std::vector<ofxSyphonServer*> servers;
    };
}

#endif