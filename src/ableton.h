#ifndef ableton_h
#define ableton_h

#include <cmath>
#include "ofxAbletonLink.h"
#include "utilities.h"
#include "ofxAbletonLiveTrack.h"
#include "ofxAbletonLive.h"

namespace ofxBenG {

    class ableton {
    public:
        static ableton *getInstance() {
            static ableton instance;
            return &instance;
        }

        ableton(ableton const &) = delete;
        void operator=(ableton const &) = delete;

        void setup(double beatsPerMinute, double quantum) {
            link.setup(beatsPerMinute);
            link.setQuantum(quantum);
            live.setup();
        }

        void update() {
            live.update();
        }

        float getTempo() {
            return link.tempo();
        }

        void drawLink() {
            ofxAbletonLink::Status status = link.update();
            int quantum = (int) ceil(link.quantum());
            int nbeat;
            float dw;
            if (quantum < 1) {
                dw = (float) ofGetWidth();
                nbeat = 0;
            } else {
                dw = (float) ofGetWidth() / (float) quantum;
                nbeat = (int) floor(status.beat) % quantum;
            }

            int top = (int) (ofGetHeight() * 0.7);
            int bottom = (int) (ofGetHeight() * 1.0);
            int h = bottom - top + 1;
            for (int i = 0; i < quantum; i++) {
                ofFill();
                ofSetColor((i <= nbeat) ? 255 : 128);
                ofDrawRectangle(i * dw, top, dw, h);
                ofNoFill();
                ofSetColor(0);
                ofDrawRectangle(i * dw, top, dw, h);
            }

            ofSetColor(0);
            ofDrawBitmapString("Tempo: " + ofToString(link.tempo()) + " Beats: " + ofToString(status.beat) + " Phase: " + ofToString(status.phase), 20, 20);
            ofDrawBitmapString("Number of peers: " + ofToString(link.numPeers()), 20, 40);
        }

        void setBeatsPerMinute(double beatsPerMinute) {
            link.setTempo(beatsPerMinute);
        }

        void setTempo(double tempo) {
            link.setTempo(tempo);
        }

        void setClockToZero() {
            setStartBeat(getBeat());
        }

        void setStartBeat(float startBeat) {
            this->startBeat = startBeat;
        }

        float getBeat() {
            ofxAbletonLink::Status status = link.update();
            return status.beat - startBeat;
        }

        float getNextWholeBeat() {
            return ceil(getBeat());
        }

        int getRoundedBeat() {
            ofxAbletonLink::Status status = link.update();
            int beat = ((int) floor(status.beat) % (int) ceil(link.quantum())) + 1;
            return beat;
        }

        bool onBeat() {
            return ofxBenG::utilities::closeToInteger(getBeat());
        }

        void stopAll() {
            live.stop();
        }

        ofxAbletonLiveTrack *getTrack(std::string trackName) {
            return live.getTrack(trackName);
        }

        ofxAbletonLiveClip *getClip(std::string trackName, std::string clipName) {
            auto track = getTrack(trackName);
            if (track != NULL) {
                return track->getClip(clipName);
            } else {
                return NULL;
            }
        }

        bool isClipPresent(std::string trackName, std::string clipName) {
            ofxAbletonLiveTrack *track = live.getTrack(trackName);
            if (track != NULL) {
                ofxAbletonLiveClip *clip = track->getClip(clipName);
                if (clip != NULL) {
                    return true;
                }
            }
            return false;
        }

        void playClip(std::string trackName, std::string clipName) {
            ofxAbletonLiveTrack *track = live.getTrack(trackName);
            if (track != NULL) {
                ofxAbletonLiveClip *clip = track->getClip(clipName);
                if (clip != NULL) {
                    clip->play();
                }
            }
        }

        void stopClip(std::string trackName, std::string clipName) {
            ofxAbletonLiveTrack *track = live.getTrack(trackName);
            if (track != NULL) {
                ofxAbletonLiveClip *clip = track->getClip(clipName);
                if (clip != NULL) {
                    clip->stop();
                }
            }
        }

        std::size_t getNumPeers() {
            return link.numPeers();
        }

    private:
        ableton() {
        }

        ofxAbletonLink link;
        ofxAbletonLive live;
        float startBeat = 0.0;
    };

    static ableton *ableton() {
        return ableton::getInstance();
    }
} // ofxBenG

#endif /* ableton_h */
