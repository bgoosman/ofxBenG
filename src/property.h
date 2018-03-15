#ifndef property_h
#define property_h

#include "ofxXmlSettings.h"

namespace ofxBenG {

#define δ(x) (dynamic_cast<ofxBenG::property_base*>(&x))
#define CAST_PROPERTY(x) (dynamic_cast<ofxBenG::property_base*>(&x))

class property_base {
public:
    property_base() {}
    
    virtual void clean() = 0;
    virtual void save(ofxXmlSettings& settings) {};
    virtual void load(ofxXmlSettings& settings) {};
    virtual void setScale(float scale) {};
    virtual float getScale() {};
    virtual std::string getName() {}
};

template <typename T>
class property : public property_base {
public:
    typedef std::function<void()> subscription_t;
    
    property(const std::string& name, property<T>& other) : name(name) {
        cachedValue = other.get();
        dirtyValue = cachedValue;
        min = other.getMin();
        max = other.getMax();
        other.addSubscriber([&]() { set(map(other)); });
    }
    
    property(const std::string& name, const T& defaultValue, const T& min, const T& max)
        : name(name),
          cachedValue(defaultValue),
          min(min),
          max(max) {}
    
    property(const property<T>& other) {
        name = other.name;
        cachedValue = other.cachedValue;
        min = other.min;
        max = other.max;
    }
    
    property<T>& operator=(const property<T>& other) {
        name = other.name;
        cachedValue = other.cachedValue;
        min = other.min;
        max = other.max;
        return *this;
    }
    
    property() {}
    
    std::string getName() {
        return name;
    }
    
    T map(property<T>& other) {
        return map(other.get(), other.getMin(), other.getMax());
    }
    
    float map(float v, float min, float max) {
        return ofMap(v, min, max, getMin(), getMax());
    }
    
    int map(int v, int min, int max) {
        return (int)ofMap(v, min, max, getMin(), getMax());
    }
    
    ofVec3f map(ofVec3f v, ofVec3f min, ofVec3f max) {
        return v;
    }
    
    ofVec3f map(int i, float value, float min, float max) {
        auto v = dirtyValue;
        v[i] = ofMap(value, min, max, getMin()[i], getMax()[i], true);
        return v;
    }
    
    float lerp(float t, float min, float max) {
        return ofLerp(min, max, t);
    }
    
    int lerp(float t, int min, int max) {
        return (int)roundf(ofLerp((float)min, (float)max, t));
    }
    
    ofVec3f lerp(float t, ofVec3f min, ofVec3f max) {
        return t * max;
    }
    
    int mapTo(int min, int max) {
        return (int)ofMap(get(), getMin(), getMax(), min, max, true);
    }
    
    float mapTo(float min, float max) {
        return ofMap(get(), getMin(), getMax(), min, max, true);
    }
    
    ofVec3f mapTo(ofVec3f min, ofVec3f max) {
        return get();
    }
    
    float getScale() {
        return scale;
    }
    
    void setScale(float v) {
        scale = v;
        set(lerp(scale, getMin(), getMax()));
    }
    
    T getMin() const {
        return min;
    }
    
    void setMin(T value) {
        min = value;
    }
    
    T getMax() const {
        return max;
    }
    
    void setMax(T value) {
        max = value;
    }
    
    void addSubscriber(const subscription_t& s) {
        subscribers.push_back(s);
    }
    
    virtual void clean() {
        if (dirty) {
            cachedValue = dirtyValue;
            dirty = false;
            notifySubscribers();
        }
    }
    
    virtual void save(ofxXmlSettings& settings) {
        settings.setValue(tag + ":" + name, getScale());
    }
    
    virtual void load(ofxXmlSettings& settings) {
        std::string defaultValue = "missing";
        std::string s = settings.getValue(tag + ":" + name, defaultValue);
        if (s.compare(defaultValue) != 0) {
            float v = ofFromString<float>(s);
            setScale(v);
            clean();
        }
    }
    
    void notifySubscribers() {
        for (auto& subscriber : subscribers) {
            subscriber();
        }
    }
    
    virtual void set(const T& v) {
        std::lock_guard<std::mutex> guard(mutex);
        if (between(v, min, max)) {
            std::cout << "Setting " << getName() << " to " << v << std::endl;
            dirtyValue = v;
            dirty = true;
        }
    }
    
    bool between(const ofVec3f& v, const ofVec3f& min, const ofVec3f& max) {
        auto length = v.length();
        return min.length() <= length && length <= max.length();
    }
    
    bool between(float v, float min, float max) {
        return min <= v && v <= max;
    }
    
    bool between(int v, int min, int max) {
        return min <= v && v <= max;
    }
    
    const T& get() const {
        return cachedValue;
    }
    
    const T& operator()() const {
        return get();
    }
    
    operator const T&() const {
        return get();
    }
    
    float operator[](int const index) {
        return get()[index];
    }

    T operator=(const T& v) {
        set(v);
        return dirtyValue;
    }
    
    T operator+=(const T& v) {
        set(dirtyValue + v);
        return dirtyValue;
    }
    
    T operator-=(const T& v) {
        set(dirtyValue - v);
        return dirtyValue;
    }
    
    T operator++(int) {
        set(dirtyValue + 1);
        return dirtyValue;
    }
    
    T operator--(int) {
        set(dirtyValue - 1);
        return dirtyValue;
    }
private:
    T min;
    T max;
    T dirtyValue;
    T cachedValue;
    bool dirty = false;
    float scale;
    std::vector<subscription_t> subscribers;
    std::mutex mutex;
    std::string const tag = "property";
    std::string name;
};

template <typename T>
class jitter : public property_base {
public:
    jitter(property<T>& target) : target(target) {}
    
    virtual void clean() {
        target.clean();
        float oldScale = target.getScale();
        float newScale = oldScale + ofRandom(0.1);
        target.setScale(newScale);
    }
    
private:
    property<T>& target;
};

} // ofxBenG

#endif /* property_h */
