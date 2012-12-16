/*
 *Copyright (C) 2012  Juan Pascual
 *
 *This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef WORKERLOCKLESS_H
#define WORKERLOCKLESS_H

#include <iostream>
#include <memory>
#include <thread>
#include <functional>

using namespace std;


class workerLockless
{
public:
    workerLockless();
    workerLockless(string Name);
    workerLockless(const workerLockless& Other);
    workerLockless& operator=(workerLockless&& Other);

    void start(shared_ptr<workerLockless>&& worker);
    void stop();
    void Process(function<void ()>&& Task);
    void WaitUntilFinish();

private:

    void run();

    function<void ()>     _task;
    bool                  _stop;
    string                _name;
    thread                _thr;

};

#endif // WORKERLOCKLESS_H
