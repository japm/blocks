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
#ifndef WORKER_H
#define WORKER_H

#include <iostream>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>

using namespace std;

class worker
{
public:
    worker();
    worker(string Name);
    worker(const worker& Other);
    worker& operator=(worker&& Other);

    void start(shared_ptr<worker> w);
    void stop();
    void Process(function<void ()>&& Task);
    void WaitUntilFinish();

private:

    void run();
    void runOnce();

    function<void ()>     _task;
    mutex                 _mutex;
    condition_variable    _cvarReady;
    bool                  _stop;
    string                _name;
    thread                _thr;

};


#endif // WORKER_H
