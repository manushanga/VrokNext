#include "componentmanager.h"

bool Vrok::ComponentManager::RegisterComponent(Component *component)
{
    stringstream name;
    name<<component->ComponentName();

    name<<":";
    auto it=_used_names.find(component->ComponentName());
    if (it != _used_names.end())
    {
        it->second = it->second +1;
        name<<it->second;
    } else
    {
        _used_names.insert(pair<string, int>(component->ComponentName(),0));
        name<<"0";

    }
    _component_map.insert(pair<string, Component*>(name.str(),component));
    return true;
}
bool Vrok::ComponentManager::RegisterProperty(Component *component,
                               string propertyname,
                               PropertyBase *property)
{
    auto it=_property_map.find(component);
    if (it != _property_map.end())
    {
        it->second.insert(pair<string,PropertyBase*>(propertyname,property));
    } else
    {
        map<string,PropertyBase*> pmap;
        pmap.insert(pair<string,PropertyBase*>(propertyname,property));
        _property_map.insert(pair<Component *, map<string, PropertyBase*> >(
                                 component, pmap ));
    }
    return true;
}

Vrok::Component *Vrok::ComponentManager::GetComponent(string component)
{
    auto it=_component_map.find(component);
    if (it != _component_map.end())
    {
        return it->second;
    } else
    {
        return nullptr;
    }
}

void Vrok::ComponentManager::SetProperty(Vrok::Component *component, PropertyBase *property, void *data)
{

    property->Set(data);

    component->PropertyChanged(property);

}

Vrok::PropertyBase *Vrok::ComponentManager::GetProperty(Vrok::Component *component, string prop_name)
{
    auto it=_property_map.find(component);
    if (it != _property_map.end())
    {
        auto it1=it->second.find(prop_name);
        if (it1 != it->second.end())
        {
            PropertyBase *p=it1->second;
            return p;
        } else
        {
            return nullptr;
        }
    } else
    {
        return nullptr;
    }
}
Vrok::ComponentManager::ComponentManager()
{

}

Vrok::ComponentManager *Vrok::ComponentManager::GetSingleton()
{
    static ComponentManager cm;
    return &cm;
}

Vrok::ComponentManager::~ComponentManager()
{

}

