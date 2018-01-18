#include "debug.h"
#include "componentmanager.h"

#include <cstdlib>
#include <string>

#define VERSION "1.0beta"

using namespace std;

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
    property->SetName(propertyname);

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

void Vrok::ComponentManager::SetProperty(Vrok::Component *component, PropertyBase *property, string value)
{
    if (property->GetType() == PropertyType::FLT)
    {
        float val = atof(value.c_str());
        property->Set(&val);
    } else if (property->GetType() == PropertyType::DBL)
    {
        double val = atof(value.c_str());
        property->Set(&val);
    } else if (property->GetType() == PropertyType::INT)
    {
        int val = atoi(value.c_str());
        property->Set(&val);
    }
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
        }
    }

    return nullptr;
}


Vrok::ComponentManager::ComponentManager()
{
    _configIO = nullptr;
}

Vrok::ComponentManager *Vrok::ComponentManager::GetSingleton()
{
    static ComponentManager cm;
    return &cm;
}

Vrok::ComponentManager::~ComponentManager()
{

}

Vrok::PropertyBase *Vrok::ComponentManager::GetProperty(std::string component,
                                                        std::string prop_name)
{

    Component* comp = GetComponent(component);

    if (comp)
    {
        return GetProperty(comp, prop_name);
    }

    return nullptr;
}

void Vrok::ComponentManager::SetProperty(std::string component,
                                         std::string prop_name,
                                         std::string value)
{

    Component* comp = GetComponent(component);
    if (comp == nullptr)
        return;

    PropertyBase* prop = GetProperty(comp, prop_name);
    if (prop == nullptr)
        return;

    if (prop->GetType() == PropertyType::FLT)
    {
        float val = atof(value.c_str());
        prop->Set(&val);
    } else if (prop->GetType() == PropertyType::DBL)
    {
        double val = atof(value.c_str());
        prop->Set(&val);
    } else if (prop->GetType() == PropertyType::INT)
    {
        int val = atoi(value.c_str());
        prop->Set(&val);
    }
    DBG(0, "Property set, comp=" << component
                                 << ", prop=" << prop_name
                                 << ", value=" << value);
    comp->PropertyChanged(prop);
}

void Vrok::ComponentManager::Deserialize()
{
    std::lock_guard<std::mutex> lg(_lock_write);

    _configIO->ReadOpen();

    std::vector<std::string> line;

    _configIO->ReadLine(line);
    if (line.size() != 1)
        throw std::runtime_error("invalid config");

    if (line[0] != VERSION)
    {
        _configIO->Close();
        return;
    }

    _configIO->ReadLine(line);
    if (line.size() != 1)
        throw std::runtime_error("invalid config");

    int components = atoi(line[0].c_str());

    for (int i=0;i<components;i++)
    {
        line.clear();
        _configIO->ReadLine(line);
        if (line.size() != 2)
            throw std::runtime_error("invalid config");
        std::string comp = line[0];
        int properties = atoi(line[1].c_str());
        for (int j=0;j<properties;j++)
        {
            line.clear();
            _configIO->ReadLine(line);
            if (line.size() != 2)
                throw std::runtime_error("invalid config");
            SetProperty(comp, line[0], line[1]);
        }
    }
    _configIO->Close();
}

void Vrok::ComponentManager::Serialize()
{
    std::lock_guard<std::mutex> lg(_lock_write);
    _configIO->WriteOpen();

    _configIO->WriteLine({VERSION});

    char temp_buffer[32] = {0};
    int size = (int) _component_map.size();
    std::vector<std::string> line;

    snprintf(temp_buffer, 31, "%d", size);
    line = { std::string(temp_buffer) };
    _configIO->WriteLine(line);

    for (auto& comp_pair : _component_map)
    {
        size = _property_map[comp_pair.second].size();
        snprintf(temp_buffer, 31, "%d", size);
        _configIO->WriteLine({ comp_pair.first, std::string(temp_buffer)});

        for (auto& prop_pair : _property_map[comp_pair.second])
        {
            switch (prop_pair.second->GetType())
            {
                case PropertyType ::INT:
                {
                    int val=0;
                    prop_pair.second->Get(&val);
                    snprintf(temp_buffer, 31, "%d", val);
                    break;
                }
                case PropertyType ::FLT:
                {
                    float val=0.f;
                    prop_pair.second->Get(&val);
                    snprintf(temp_buffer, 31, "%f", val);
                    break;
                }
                case PropertyType ::DBL:
                {
                    double val=0.0;
                    prop_pair.second->Get(&val);
                    snprintf(temp_buffer, 31, "%lf", val);
                    break;
                }
            }

            _configIO->WriteLine({prop_pair.first, std::string(temp_buffer)});

        }
    }
    _configIO->Close();
}

void Vrok::ComponentManager::SetConfigIO(Vrok::ComponentManager::ConfigIO *configIO)
{
    _configIO = configIO;
}

