#ifndef tracer_h
#define tracer_h

#include "ofxIntersection.h"

namespace ofxBenG {

class Tracer;

class TracerUpdateStrategy {
public:
    virtual void update(Tracer* t, float time) = 0;
};

class TracerDrawStrategy {
public:
    virtual void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) = 0;
};

class Particle {
public:
    Particle(ofVec3f location, ofVec3f velocity = ofVec3f::zero(), float mass = 0) {
        this->location = location;
        this->velocity = velocity;
        this->mass = mass;
    }
    
    void applyForce(ofVec3f force) {
        force /= mass;
        acceleration += force;
    }
    
    void update() {
        velocity += acceleration;
        location += velocity;
        acceleration *= 0;
    }
    
    void setLocation(ofVec3f location) {
        this->location = location;
    }
    
    ofVec3f location;
    ofVec3f velocity;
    ofVec3f acceleration;
    float mass;
};

class Tracer {
public:
    Tracer(ofVec3f startLocation, ofVec3f stageSize)
        : head(startLocation),
          stageSize(stageSize) {}
    
    TracerUpdateStrategy* getUpdateBehavior(int index) {
        return (index >= 0 && index < updateStrategies.size()) ? updateStrategies[index] : nullptr;
    }
    
    void addUpdateBehavior(TracerUpdateStrategy* strategy) {
        updateStrategies.push_back(strategy);
    }
    
    void addDrawBehavior(TracerDrawStrategy* strategy) {
        drawStrategies.push_back(strategy);
    }
    
    void update(float time) {
        for (TracerUpdateStrategy* s : updateStrategies) {
            s->update(this, time);
        }
    }
    
    void draw(float time, std::shared_ptr<ofBaseRenderer> renderer) {
        for (TracerDrawStrategy* s : drawStrategies) {
            s->draw(this, time, renderer);
        }
    }
    
    void mapDimension(int dimension, ofVec2f range) {
        head[dimension] = ofMap(head[dimension],
                                -0.5 * stageSize[dimension],
                                 0.5 * stageSize[dimension],
                                range[0],
                                range[1]);
    }
    
    Particle* getHead() {
        return (particles.size() > 0) ? particles[particles.size()-1] : nullptr;
    }
    
    Particle* getTail() {
        return (particles.size() > 0) ? particles[0] : nullptr;
    }
    
    ofVec3f head;
    ofVec3f stageSize;
    ofPath path;
    std::deque<Particle*> particles;
    std::vector<TracerUpdateStrategy*> updateStrategies;
    std::vector<TracerDrawStrategy*> drawStrategies;
};

template <class T>
class VaryPerlin : public TracerUpdateStrategy {
public:
    VaryPerlin(T* source, ofVec2f range) :
        source(source),
        range(range)
    {
    }
    
    void update(Tracer* tracer, float time) {
        float perlin = ofNoise(time * 0.001);
        *source = ofMap(perlin, 0, 1, range[0], range[1], true);
    }
    
    T* source;
    ofVec2f range;
};

class Multiplier : public TracerDrawStrategy {
public:
    Multiplier(property<int>& multiplierCount, property<float>& maxShift) : multiplierCount("multiplierCount", multiplierCount), maxShift("multiplierMaxShift", maxShift) {
        this->shifts = getRandomShifts();
        this->maxShift.addSubscriber([&]() {
            this->shifts = getRandomShifts();
        });
        this->multiplierCount.addSubscriber([&]() {
            this->shifts = getRandomShifts();
        });
    }
    
    ofVec3f getRandomShift() {
        float const maxShift = this->maxShift;
        ofVec3f randomShift;
        randomShift.x = ofRandom(-1 * maxShift, maxShift);
        randomShift.y = ofRandom(-1 * maxShift, maxShift);
        randomShift.z = ofRandom(-1 * maxShift, maxShift);
        return randomShift;
    }
    
    std::vector<ofVec3f> getRandomShifts() {
        std::vector<ofVec3f> shifts;
        for (int i = 0; i < multiplierCount; i++) {
            ofVec3f shift = getRandomShift();
            shifts.push_back(shift);
        }
        return shifts;
    }
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        multiplierCount.clean();
        maxShift.clean();
        if (t->particles.size() >= 2) {
            for (int i = 0; i < multiplierCount; i++) {
                if (i < shifts.size()) {
                    ofPushMatrix();
                    ofVec3f shift = shifts[i];
                    ofTranslate(shift);
                    renderer->draw(t->path);
                    ofPopMatrix();
                }
            }
        }
    }
    
    std::vector<ofVec3f> shifts;
    property<int> multiplierCount;
    property<float> maxShift;
};

class VibratingMultiplier : public TracerDrawStrategy {
public:
    VibratingMultiplier(Multiplier* super, property<float>& entropy) : super(super), entropy("entropy", entropy) { }
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        entropy.clean();
        if (entropy > 0.1) {
            super->shifts = super->getRandomShifts();
        }
        
        super->draw(t, time, renderer);
    }
    
private:
    Multiplier* super;
    property<float> entropy;
};

class DrawPizza : public TracerDrawStrategy {
public:
    DrawPizza(ofImage& pizza) : pizza(pizza) {}
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        if (t->particles.size() > 0) {
            for (Particle* particle : t->particles) {
                pizza.draw(particle->location.x, particle->location.y);
            }
        }
    }
    
private:
    ofImage& pizza;
};

class PerlinBrightness : public TracerDrawStrategy {
public:
    PerlinBrightness(ofVec3f timeShift, ofVec3f velocity)
    : timeShift(timeShift), velocity(velocity) {}
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        ofColor color = t->path.getStrokeColor();
        float noise = ofNoise(time * velocity[0] + timeShift[0]);
        color.setBrightness(ofMap(noise, 0, 1, 0, ofColor::limit(), true));
        t->path.setStrokeColor(color);
    }
    
private:
    ofVec3f timeShift;
    ofVec3f velocity;
};

class StrokeWidthMappedToValue : public TracerDrawStrategy {
public:
    StrokeWidthMappedToValue(float maxStrokeWidth, float* value, ofVec2f valueRange)
    : maxStrokeWidth(maxStrokeWidth), value(value), valueRange(valueRange) {}
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        float strokeWidth = ofMap(*value, valueRange[0], valueRange[1], 0, maxStrokeWidth);
        t->path.setStrokeWidth(strokeWidth);
    }
    
private:
    float maxStrokeWidth;
    float* value;
    ofVec2f valueRange;
};

class StrokeWidth : public TracerDrawStrategy {
public:
    StrokeWidth(property<float>& strokeWidth) : strokeWidth("StrokeWidth", strokeWidth) {}
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        strokeWidth.clean();
        t->path.setStrokeWidth(strokeWidth);
    }
    
    property<float> strokeWidth;
};

class FilledPath : public TracerDrawStrategy {
public:
    FilledPath(bool filled, ofColor color) :
        filled(filled),
        color(color) {}
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        t->path.setFilled(filled);
        t->path.setFillColor(color);
    }
    
private:
    bool filled;
    ofColor color;
};

class StrokeColor : public TracerDrawStrategy {
public:
    StrokeColor(ofColor strokeColor) : strokeColor(strokeColor) {}
    
    virtual void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
//        ofSetColor(strokeColor);
        t->path.setStrokeColor(strokeColor);
    }
    
    virtual ofColor getColor() {
        return strokeColor;
    }
    
    virtual void setColor(ofColor& color) {
        strokeColor = color;
    }
    
private:
    ofColor strokeColor;
};

class Saturation : public TracerDrawStrategy {
public:
    Saturation(StrokeColor* stroke, property<int>& saturation) : stroke(stroke), saturation("saturation", saturation) {}
    
    virtual void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        saturation.clean();
        ofColor hsbColor = stroke->getColor();
        float h, s, b;
        hsbColor.getHsb(h, s, b);
        hsbColor.setHsb(h, saturation, b);
        stroke->setColor(hsbColor);
    }
    
private:
    StrokeColor* stroke;
    property<int> saturation;
};

class Brightness : public TracerDrawStrategy {
public:
    Brightness(StrokeColor* stroke, property<int>& brightness) : stroke(stroke), brightness("brightness", brightness) {}
    
    virtual void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        brightness.clean();
        ofColor hsbColor = stroke->getColor();
        float h, s, b;
        hsbColor.getHsb(h, s, b);
        hsbColor.setHsb(h, s, brightness);
        stroke->setColor(hsbColor);
    }
    
private:
    StrokeColor* stroke;
    property<int> brightness;
};

class Hue : public TracerDrawStrategy {
public:
    Hue(StrokeColor* stroke, property<int>& hue) : stroke(stroke), hue("hue", hue) {}
    
    virtual void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        hue.clean();
        ofColor hsbColor = stroke->getColor();
        float h, s, b;
        hsbColor.getHsb(h, s, b);
        hsbColor.setHsb(hue, s, b);
        stroke->setColor(hsbColor);
    }
    
private:
    StrokeColor* stroke;
    property<int> hue;
};

class InvertHue : public TracerDrawStrategy {
public:
    InvertHue(StrokeColor* stroke) : stroke(stroke) {}
    
    virtual void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        ofColor hsbColor = stroke->getColor();
        float hue, saturation, brightness;
        hsbColor.getHsb(hue, saturation, brightness);
        hsbColor.setHsb(255 - hue, saturation, brightness);
        stroke->setColor(hsbColor);
    }
    
private:
    StrokeColor* stroke;
};

class RandomStrokeColor : public StrokeColor {
public:
    RandomStrokeColor() :
        StrokeColor(ofColor(ofRandom(255), ofRandom(255), ofRandom(255))) {}
    
    RandomStrokeColor(ofColor colors[], int size) :
        StrokeColor(colors[rand() % size]) {}
};

class DrawPath : public TracerDrawStrategy {
public:
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        if (t->particles.size() >= 2) {
            renderer->draw(t->path);
        }
    }
};

class SphereHead : public TracerDrawStrategy {
public:
    SphereHead(property<float>& strokeWidth) : strokeWidth("SphereHeadStrokeWidth", strokeWidth) {}
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        strokeWidth.clean();
        Particle* head = t->getHead();
        if (head != nullptr) {
            ofPushMatrix();
            ofTranslate(head->location);
            ofSpherePrimitive sphere;
            sphere.set(strokeWidth, 100);
            sphere.draw();
            ofPopMatrix();
        }
    }
private:
    property<float> strokeWidth;
};

class EllipseHead : public TracerDrawStrategy {
public:
    EllipseHead(property<float>& strokeWidth) : strokeWidth("EllipseHeadStrokeWidth", strokeWidth) {}
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        strokeWidth.clean();
        Particle* head = t->getHead();
        if (head != nullptr) {
            ofDrawEllipse(head->location.x, head->location.y, head->location.z, strokeWidth, strokeWidth);
        }
    }
private:
    property<float> strokeWidth;
};

class EllipseTail : public TracerDrawStrategy {
public:
    EllipseTail(property<float>& strokeWidth) : strokeWidth("EllipseTailStrokeWidth", strokeWidth) {}
    
    void draw(Tracer* t, float time, std::shared_ptr<ofBaseRenderer> renderer) {
        strokeWidth.clean();
        Particle* tail = t->getTail();
        if (tail != nullptr) {
            ofDrawEllipse(tail->location.x, tail->location.y, tail->location.z, strokeWidth, strokeWidth);
        }
    }
private:
    property<float> strokeWidth;
};

class HeadGrowth : public TracerUpdateStrategy {
public:
    void update(Tracer* t, float time) {
        t->particles.push_back(new Particle(t->head, ofVec3f::zero(), 0));
    }
};

class CurvedPath : public TracerUpdateStrategy {
public:
    CurvedPath() : updatesBeforeNextMoveTo(UPDATE_CYCLE) {}
    
    void update(Tracer* t, float time) {
        if (t->particles.size() >= 2 && updatesBeforeNextMoveTo-- > 0) {
            updatesBeforeNextMoveTo = UPDATE_CYCLE;
            t->path.clear();
            t->path.moveTo(t->particles[0]->location);
            for (Particle* particle : t->particles) {
                t->path.curveTo(particle->location);
            }
        }
    }
    
private:
    int const UPDATE_CYCLE = 5;
    int updatesBeforeNextMoveTo;
};

class MaximumLength : public TracerUpdateStrategy {
public:
    MaximumLength(property<int>& maxPoints): maxPoints("maxPoints", maxPoints) {}
    
    void update(Tracer* t, float time) {
        maxPoints.clean();
        while (t->particles.size() > maxPoints) {
            Particle* p = t->particles[0];
            t->particles.pop_front();
            delete p;
        }
    }

    property<int> maxPoints;
};

class CubicMovement : public TracerUpdateStrategy {
public:
    CubicMovement(ofVec3f velocity, ofVec3f stageSize, ofVec3f timeShift) : velocity(velocity), stageSize(stageSize), timeShift(timeShift) {
        range1 = {ofRandom(-10, 10), ofRandom(-10, 10)};
        range2 = {ofRandom(-10, 10), ofRandom(-10, 10)};
        range3 = {ofRandom(-10, 10), ofRandom(-10, 10)};
        range4 = {ofRandom(-10, 10), ofRandom(-10, 10)};
    }
    
    void update(Tracer* t, float time) {
        float x = (int)time % 100;
        float y = cubic(ofMap((int)time % 100, 0, 100, 0, 1));
        x = ofMap(x, 0, 100, 0, stageSize[0], true);
        y = ofMap(y, -10000, 10000, 0, stageSize[1], true);
        t->head = ofVec3f(x, y);
    }
    
    float cubic(float t) {
        float a1 = ofLerp(t, range1[0], range1[1]);
        float a2 = ofLerp(t, range2[0], range2[1]);
        float a3 = ofLerp(t, range3[0], range3[1]);
        float a4 = ofLerp(t, range4[0], range4[1]);
        float b1 = ofLerp(t, a1, a2);
        float b2 = ofLerp(t, a3, a4);
        float c = ofLerp(t, b1, b2);
        return c;
    }
    
private:
    ofVec3f velocity;
    ofVec3f stageSize;
    ofVec3f timeShift;
    ofVec2f range1;
    ofVec2f range2;
    ofVec2f range3;
    ofVec2f range4;
    float min, max;
};

class SetHeadToZeroEveryUpdate : public TracerUpdateStrategy {
public:
    SetHeadToZeroEveryUpdate() {}
    
    void update(Tracer* t, float time) {
        t->head = ofVec3f(0, 0, 0);
    }
};

class ProjectOntoBox : public TracerUpdateStrategy {
public:
    ProjectOntoBox(ofBoxPrimitive* box) : box(box) {}
    
    void update(Tracer *t, float time) {
        if (box->getSize().length() > 0) {
            ofVec3f closestIntersection = MAX_INTERSECTION;
            for (int i = 0; i < NUM_BOX_SIDES; i++) {
                auto side = box->getSideMesh(i);
                boxSide.set(side.getVertices()[0], side.getNormals()[0]);
                line.set(ofVec3f(0, 0, 0), SCALE * t->head + JITTER);
                intersection = is.LinePlaneIntersection(line, boxSide);
                if (intersection.isIntersection) {
                    if (intersection.pos.length() < closestIntersection.length()) {
                        closestIntersection = intersection.pos;
                    }
                }
            }
            t->head += closestIntersection;
        }
    }
    
private:
    ofBoxPrimitive* box;
    float const JITTER = 0.0001;
    int const SCALE = 100000;
    int const NUM_BOX_SIDES = 6;
    ofVec3f const MAX_INTERSECTION = {INT_MAX, INT_MAX, INT_MAX};
    ofxIntersection is;
    IntersectionData intersection;
    IsLine line;
    IsPlane boxSide;
};

class PerlinMovement : public TracerUpdateStrategy {
public:
    PerlinMovement(property<ofVec3f>& velocity, ofVec3f timeShift) : velocity("velocity", velocity), timeShift(timeShift) {
        rangeX = {"rangeX", ofVec2f(0, 1), ofVec2f(0, 1), ofVec2f(0, 1)};
        rangeY = {"rangeY", ofVec2f(0, 1), ofVec2f(0, 1), ofVec2f(0, 1)};
        rangeZ = {"rangeZ", ofVec2f(0, 1), ofVec2f(0, 1), ofVec2f(0, 1)};
    }
    
    PerlinMovement(property<ofVec3f>& velocity,
                   ofVec3f timeShift,
                   property<ofVec2f>& rangeX,
                   property<ofVec2f>& rangeY,
                   property<ofVec2f>& rangeZ) :
        velocity("velocity", velocity),
        rangeX("PerlinMovementRangeX", rangeX),
        rangeY("PerlinMovementRangeY", rangeY),
        rangeZ("PerlinMovementRangeZ", rangeZ),
        timeShift(timeShift) {}
    
    void update(Tracer* t, float time) {
        velocity.clean();
        rangeX.clean();
        rangeY.clean();
        rangeZ.clean();

        float x = ofNoise(time * velocity[0] + timeShift[0]);
        x = ofMap(x, 0, 1, rangeX[0], rangeX[1]);
        
        float y = ofNoise(time * velocity[1] + timeShift[1]);
        y = ofMap(y, 0, 1, rangeY[0], rangeY[1]);
        
        float z = ofNoise(time * velocity[2] + timeShift[2]);
        z = ofMap(z, 0, 1, rangeZ[0], rangeZ[1]);
        
        t->head += ofVec3f(x, y, z);
    }
    
private:
    property<ofVec3f> velocity;
    property<ofVec2f> rangeX;
    property<ofVec2f> rangeY;
    property<ofVec2f> rangeZ;
    ofVec3f timeShift;
};

class MapDimension : public TracerUpdateStrategy {
public:
    MapDimension(int dimension, property<ofVec2f>& range) : dimension(dimension), range("MapDimensionRange", range) {}
    
    void update(Tracer* t, float time) {
        range.clean();
        t->head[dimension] = ofMap(t->head[dimension], 0, 1, range[0], range[1]);
    }
    
private:
    property<ofVec2f> range;
    int dimension;
};

} // ofxBenG

#endif /* tracer_h */
