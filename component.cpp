#include "component.h"
#include <stdio.h>

namespace  Vrok {

template<>
Vrok::Property<int>::Property():
    _type(PropertyType::INT)
{}
template<>
Vrok::Property<float>::Property():
    _type(PropertyType::FLT)
{}
template<>
Vrok::Property<double>::Property():
    _type(PropertyType::DBL)
{}

void PropertyBase::SetName(std::string name)
{
    _name = name;
}

std::string PropertyBase::GetName()
{
    return _name;
}

void PropertyBase::SetPropertyInfo(const PropertyInfo &info)
{
    _info = info;
}

const PropertyInfo& PropertyBase::GetPropertyInfo()
{
    return _info;
}

}

Vrok::Component::Component()
{

}

Vrok::Component::~Component()
{

}

