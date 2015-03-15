/** ThreadPool
 * (C) Madura. A
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

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <functional>

using namespace std;
class ThreadPool
{
private:
    int threadCount_;
    vector<thread> threads_;
    vector<function<void(void *)>> work_;

public:
    // the container type,
    // Dedicated - only work on a dedicated thread
    // Any - work on any idling thread
    enum SchedMode{Dedicated, Any};
    // Best - wake up as soon as possible
    // Lazy - schedule in to a free thread, if not run
    // for along time (>5 sec) schedule in to a new
    // thread
    enum WakeUpMode{Best, Lazy};
    ThreadPool(int threadCount);
    void ScheduleWork(function<void(void *)> func,
                      SchedMode schedMode,
                      WakeUpMode wakeUpMode);
};

#endif // THREADPOOL_H
