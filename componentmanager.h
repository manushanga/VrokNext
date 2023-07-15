/**
 * Copyright (C) Madura A.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02111-1307, USA.
 */

#pragma once

#include <map>
#include <mutex>
#include <sstream>

#include "component.h"

namespace vrok {
class ComponentManager {
public:
    class ConfigIO {
    public:
        virtual ~ConfigIO() { }
        virtual void ReadOpen() = 0;
        virtual void WriteOpen() = 0;
        virtual void Close() = 0;
        virtual bool ReadLine(std::vector<std::string> &line) = 0;
        virtual bool WriteLine(const std::vector<std::string> &line) = 0;
    };
    ComponentManager();
    void SetConfigIO(ConfigIO *configIO);
    static ComponentManager *GetSingleton();
    bool RegisterComponent(Component *component);
    bool RegisterProperty(Component *component, std::string propertyname, PropertyBase *property);
    Component *GetComponent(std::string component);
    void SetProperty(std::string component, std::string prop_name, std::string value);
    void SetProperty(Component *component, PropertyBase *property, std::string value);

    PropertyBase *GetProperty(std::string component, std::string prop_name);
    PropertyBase *GetProperty(Component *component, std::string prop_name);

    void Serialize();
    void Deserialize();

    ~ComponentManager();

private:
    std::mutex _lock_write;
    std::map<std::string, int> _used_names;
    std::map<std::string, Component *> _component_map;
    std::map<Component *, std::map<std::string, PropertyBase *>> _property_map;
    ConfigIO *_configIO;
};
}
