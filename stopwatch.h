/** C++11 StopWatch
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

#include <chrono>
#include <mutex>

using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::steady_clock;

class StopWatch
{
private:
    steady_clock::time_point _time,_end_time;
    float TimeDiff(steady_clock::time_point& tp1,
                   steady_clock::time_point& tp2)
    {
        return (duration_cast<duration<float>>(tp1 - tp2)).count();
    }
public:
    StopWatch()
    {
        _time = std::chrono::steady_clock::now();
    }
    void Reset()
    {
        _time = std::chrono::steady_clock::now();
    }
    float Stop()
    {
        _end_time=std::chrono::steady_clock::now();
        return TimeDiff(_end_time,_time);
    }
};

