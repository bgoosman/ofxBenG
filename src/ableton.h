#ifndef ableton_h
#define ableton_h

#include "ofxAbletonLink.h"

namespace ofxBenG {

class ableton {
public:
	void setupLink(double beatsPerMinute) {
		link.setup(beatsPerMinute);
	}
    
    float getTempo() {
        return link.tempo();
    }

	void drawLink() {
	    ofxAbletonLink::Status status = link.update();
	    int quantum = (int)ceil(link.quantum());
	    int nbeat;
	    float dw;
	    if (quantum < 1) {
	        dw = (float)ofGetWidth();
	        nbeat = 0;
	    } else {
	        dw = (float)ofGetWidth() / (float)quantum;
	        nbeat = (int)floor(status.beat) % quantum;
	    }

	    int top = (int)(ofGetHeight() * 0.7);
	    int bottom = (int)(ofGetHeight() * 1.0);
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
    
    int getBeat() {
        ofxAbletonLink::Status status = link.update();
        int beat = ((int)floor(status.beat) % (int)ceil(link.quantum())) + 1;
        return beat;
    }

private:
    ofxAbletonLink link;
};

} // ofxBenG

#endif /* ableton_h */
