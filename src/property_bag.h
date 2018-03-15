#ifndef propertybag_h
#define propertybag_h

#include "property.h"
#include <vector>

namespace ofxBenG {

class property_bag {
public:
    void add(property_base* property) {
        properties.push_back(property);
    }
    
    void loadFromXml() {
        loadFromXml("settings.xml");
    }

    void loadFromXml(const std::string& file) {
        settings.load(file);
        for (auto property : properties) {
            property->load(settings);
        }
    }
    
    void saveToXml() {
        saveToXml("settings.xml");
    }

    void saveToXml(const std::string& file) {
        for (auto property : properties) {
            property->save(settings);
        }
        settings.save(file);
    }

    void update() {
        for (auto property : properties) {
            property->clean();
        }
    }

    template<typename Functor>
    void apply(Functor f) {
        std::for_each(properties.begin(), properties.end(), f);
    }
private:
    std::vector<property_base*> properties;
    ofxXmlSettings settings;
};
    
}; /* ofxBenG */

#endif /* propertybag_h */
