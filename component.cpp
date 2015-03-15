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

}


Vrok::Component::Component()
{

}

Vrok::Component::~Component()
{

}

