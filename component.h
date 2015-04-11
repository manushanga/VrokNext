/** Component
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
#ifndef COMPONENT_H
#define COMPONENT_H
#include <atomic>
using namespace std;

namespace Vrok {

enum class ComponentType{Decoder, Effect, Driver, Player, None};
enum class PropertyType{INT,FLT,DBL};


class PropertyBase
{
public:
    virtual PropertyType GetType()=0;
    virtual void Get(void *ptr)=0;
    virtual void Set(void *ptr)=0;
};

template<typename T>
class Property : public PropertyBase
{
private:
    PropertyType _type;
    atomic<T> _data;
public:
    Property();

    PropertyType GetType()
    {
        return _type;
    }
    void Get(void *ptr)
    {
        *((T*)ptr)=_data.load(memory_order_relaxed);
    }
    T Get()
    {
        return _data.load(memory_order_relaxed);
    }
    void Set(T val)
    {
        _data.store(val, memory_order_relaxed);
    }
    void Set(void *ptr)
    {
        _data.store(*((T*)ptr), memory_order_relaxed);
    }

};



class Component
{
public:
    Component();
    virtual Vrok::ComponentType ComponentType() { return Vrok::ComponentType::None; }
    virtual Component *CreateSelf() = 0;
    virtual const char *ComponentName() = 0;
    virtual const char *Description() { return ""; }
    virtual const char *Author() { return ""; }
    virtual const char *License() { return ""; }
    virtual void PropertyChanged(PropertyBase *property) {}
    virtual ~Component();
};

}


#endif // COMPONENT_H
