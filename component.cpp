#include "component.h"
#include <stdio.h>

namespace vrok {

template <>
vrok::Property<int>::Property() : _type(PropertyType::INT) { }
template <>
vrok::Property<float>::Property() : _type(PropertyType::FLT) { }
template <>
vrok::Property<double>::Property() : _type(PropertyType::DBL) { }

void PropertyBase::SetName(std::string name) {
    _name = name;
}

std::string PropertyBase::GetName() {
    return _name;
}

void PropertyBase::SetPropertyInfo(const PropertyInfo &info) {
    _info = info;
}

const PropertyInfo &PropertyBase::GetPropertyInfo() {
    return _info;
}

uint32_t PropertyBase::Size() {
    switch (GetType()) {
    case PropertyType::INT:
        return sizeof(int);
    case PropertyType::DBL:
        return sizeof(double);
    case PropertyType::FLT:
        return sizeof(float);
    }
    return 0;
}

}

vrok::Component::Component() { }

vrok::Component::~Component() { }
