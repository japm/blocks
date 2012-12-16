#include "workerlockless.h"

workerLockless::workerLockless() :  _task(nullptr), _stop(false)
{
}

workerLockless::workerLockless(string Name) : _task(nullptr), _stop(false), _name(Name)
{
}

workerLockless::workerLockless(const workerLockless &Other): workerLockless(Other._name)
{
}


workerLockless &workerLockless::operator =(workerLockless &&Other)
{
    if (this == &Other)
        return *this;

    _name  = std::move(Other._name);
}


void workerLockless::run()
{
    for(;!_stop;){
        if (_task == nullptr)
            break;

        auto localTask = std::move(_task);
        _task = nullptr;

        if (_stop)
            return;

        localTask();
    }
}


void workerLockless::start(shared_ptr<workerLockless>&& worker)
{
    shared_ptr<workerLockless> w =std::move(worker);

    //Encapsulet in a closure so the worker is not deleted until thread end
    function<void ()> guard = [w](){w->run();};
    //Run the thread
    _thr = std::thread(guard);
    //Dont wait for finalization
    _thr.detach();

}

void workerLockless::stop()
{
    _stop = true;
}

void workerLockless::Process(function<void ()>&& Task)
{
    _task = std::move(Task);
}

void workerLockless::WaitUntilFinish()
{
    _thr.join();
}
