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
#include <sstream>

#include "component.h"

namespace Vrok {
class ComponentManager
{
public:
    class ComponentConfig
    {
    public:
        virtual bool Read(void **data, size_t *size)=0;
        virtual bool Write(void *data, size_t size)=0;
    };
    ComponentManager(ComponentConfig *comp_config = nullptr);
    static ComponentManager *GetSingleton();
    bool RegisterComponent(Component *component);
    bool RegisterProperty(Component *component,
                          std::string propertyname,
                          PropertyBase *property);
    Component *GetComponent(std::string component);
    void SetProperty(Component *component, PropertyBase *property, void *data);
    PropertyBase *GetProperty(Component *component, std::string prop_name);

    ~ComponentManager();
private:
    std::map<std::string, int> _used_names;
    std::map<std::string, Component *> _component_map;
    std::map<Component *, std::map<std::string, PropertyBase *> > _property_map;
    ComponentConfig *_component_config;

    bool Load();
    bool Save();

};
}

