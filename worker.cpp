#include "worker.h"

worker::worker() :  worker("")
{
}

worker::worker(string Name) : _mutex(), _cvarReady(), _task(nullptr), _stop(false), _name(Name)
{
}

worker::worker(const worker &Other): worker(Other._name)
{
}


worker &worker::operator =(worker &&Other)
{
    if (this == &Other)
        return *this;

    _name  = std::move(Other._name);
}


void worker::run()
{
    for(;!_stop;){
        runOnce();
    }
}


void worker::runOnce()
{
    function<void ()> localTask;
    {
        unique_lock<mutex> lock(_mutex);
        if (_task == nullptr)
            _cvarReady.wait(lock);

        localTask = std::move(_task);
        _task = nullptr;
    }

    if (_stop)
        return;

    localTask();
}


void worker::start(shared_ptr<worker> w)
{
    //Encapsulet in a closure so the worker is not deleted until thread end
    function<void ()> guard = [w](){w->run();};
    //Run the thread
    _thr = std::thread(guard);

}


void worker::stop()
{
    _stop = true;
}



void worker::Process(function<void ()>&& Task)
{
    unique_lock<mutex> lock(_mutex);
    _task = std::move(Task);
    _cvarReady.notify_one();

}

void worker::WaitUntilFinish()
{
    _thr.join();
}


