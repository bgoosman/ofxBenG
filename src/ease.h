#ifndef ease_h
#define ease_h

#include "ofxEasing.h"
#include "Poco/Timestamp.h"
#include <algorithm>

namespace ofxBenG {

typedef Poco::Timestamp::TimeDiff TimeDiff;

class ease {
public:
    ease(TimeDiff forwardDuration, float startValue, float targetValue, ofxeasing::function easingFunction) :
        startTime(-1),
        forwardDuration(forwardDuration),
        backwardDuration(-1),
        isGoingBackward(false),
        startValue(startValue),
        targetValue(targetValue),
        easingFunction(easingFunction) {}
    
    ease(TimeDiff forwardDuration, TimeDiff backwardDuration, float startValue, float targetValue, ofxeasing::function easingFunction) :
         startTime(-1),
         forwardDuration(forwardDuration),
         backwardDuration(backwardDuration),
         isGoingBackward(false),
         startValue(startValue),
         targetValue(targetValue),
         easingFunction(easingFunction) {}
    
    ease(const ease& other) :
        startTime(-1),
        forwardDuration(other.forwardDuration),
        backwardDuration(other.backwardDuration),
        isGoingBackward(false),
        startValue(other.startValue),
        targetValue(other.targetValue),
        easingFunction(other.easingFunction) {}
    
    ~ease() {}
    
    float update(TimeDiff currentTime) {
        if (startTime < 0) {
            startTime = currentTime;
        }
        
        auto currentStartTime = getStartTime();
        auto currentEndTime = getEndTime(currentStartTime);
        auto currentStartValue = isGoingBackward ? targetValue : startValue;
        auto currentTargetValue = isGoingBackward ? startValue : targetValue;
        if (currentTime < currentStartTime) {
            currentValue = currentStartValue;
        } else if (currentTime >= currentEndTime) {
            if (backwardDuration > 0 && !isGoingBackward) {
                isGoingBackward = true;
            } else {
                currentValue = currentTargetValue;
            }
        } else {
            currentValue = ofxeasing::map(currentTime, currentStartTime, currentEndTime, currentStartValue, currentTargetValue, easingFunction);
        }
        
        return currentValue;
    }

    TimeDiff getDuration() {
        return isGoingBackward ? backwardDuration : forwardDuration;
    }

    TimeDiff getStartTime() {
        return isGoingBackward ? startTime + forwardDuration : startTime;
    }

    TimeDiff getEndTime(float startTime) {
        return isGoingBackward ? startTime + backwardDuration : startTime + forwardDuration;
    }

    bool isPlaying() {
        return startValue < currentValue && currentValue < targetValue;
    }
    
    bool isDone(TimeDiff currentTime) {
        return currentTime > getEndTime(getStartTime());
    }
    
private:
    bool isGoingBackward;
    float currentValue;
    float startValue;
    float targetValue;
    TimeDiff startTime;
    TimeDiff forwardDuration;
    TimeDiff backwardDuration;
    ofxeasing::function easingFunction;
};

} // ofxBenG

#endif /* ease_h */
