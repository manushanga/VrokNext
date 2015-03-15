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
namespace Vrok {

class Component
{
public:
    enum ComponentType{Decoder, Effect, Driver, Player, None};
    Component();
    virtual ComponentType ComponentType() { return None; }
    virtual Component *MakeSelf() = 0;
    virtual char *ComponentName() = 0;
    virtual char *Description() { return ""; }
    virtual char *Author() { return ""; }
    virtual char *License() { return ""; }
    virtual ~Component();
};

}


#endif // COMPONENT_H
