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

#ifndef DISPATCHER_H
#define DISPATCHER_H


#include "payload.h"
#include "worker.h"
#include "workerlockless.h"
#include <vector>
#include <list>
#include <memory>
#include <mutex>
#include <functional>

#define DEBUG 1
#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

using namespace std;


template<class T>
class Dispatcher
{
public:

    typedef shared_ptr<T>       HolderType;
    typedef shared_ptr<worker>  SharedWorker;
    typedef shared_ptr<workerLockless>  SharedWorkerLockless;

    Dispatcher(unsigned int ThreadsInPool, unsigned int MaxThreads, bool RunOnThisThread, unsigned int QueueSize, function<void (HolderType)> Fn);

    void Dispatch(HolderType spt);

    virtual void WorkerDone(SharedWorker w);
    virtual void WorkerNoPoolDone(SharedWorkerLockless w);

    void WaitUntilFinish();

    void Stop();


private:

    vector<SharedWorker>         _workers;
    vector<SharedWorker>         _freeWorkers;
    unsigned int                 _freeWorkersIdx;
    unsigned int                 _threadsInPool;
    unsigned int                 _maxThreads;
    unsigned int                 _actualThreads;
    bool                         _runOnThisThread;
    unsigned int                 _queueSize;
    unsigned int                 _waiting;
    mutex                        _mutex;
    condition_variable           _workerEnd;

    function<void (HolderType)>  _funct;
    vector<HolderType>           _buffer;
    unsigned int                 _first;
    unsigned int                 _last;
};



template<class T>
Dispatcher<T>::Dispatcher(unsigned int ThreadsInPool, unsigned int MaxThreads,
                          bool RunOnThisThread, unsigned int QueueSize,
                          function<void (HolderType)> Fn) :_workers(), _freeWorkers(), _mutex(), _funct(Fn),
    _buffer(), _maxThreads(MaxThreads), _actualThreads(ThreadsInPool), _workerEnd(), _runOnThisThread(RunOnThisThread),
    _queueSize(QueueSize == 1 ? 2 : QueueSize), _threadsInPool(ThreadsInPool)
{
    _waiting = 0;
    _funct = Fn;
    if (_queueSize)
        _buffer.reserve(_queueSize);
    _first = 0;
    _last  = 0;

    if (_threadsInPool){

        _workers.reserve(_threadsInPool);
        _freeWorkers.reserve(_threadsInPool);
        _freeWorkersIdx = _threadsInPool;

        for(unsigned int i=0;i<_threadsInPool;i++){
            auto wk = std::make_shared<worker>(worker(to_string(i)));
            _workers.push_back( std::move(wk));
        }

        //for(auto wk = _workers.begin(); wk != _workers.end(); wk++)
        //    _freeWorkers.push_back(*wk);

        for(unsigned int i=0;i<_threadsInPool;i++){
            _freeWorkers[i] = _workers[i];
        }

        for(auto wk = _workers.begin(); wk != _workers.end(); wk++)
            wk->get()->start(*wk);
    }
}

template<class T>
void Dispatcher<T>::Dispatch(HolderType spt)
{
    bool runInANewThread = false;
    SharedWorker w = nullptr;

    {
        unique_lock<mutex> ul(_mutex);

        while (true) {
            //If someone is ready, give it to him
            if (_freeWorkersIdx > 0){
                w = std::move(_freeWorkers[--_freeWorkersIdx]);
                break;
            //If i can dispatch a new thread do it
            }else if (_actualThreads < _maxThreads){
                _actualThreads++;
                runInANewThread = true;
                break;
            //If run in current thread we are done
            }else if (_runOnThisThread){
                break;
            }if (_queueSize){
                //Wait for a free slot in the queue
                if ((_last - _first) == 1 || (_last == 0 && _first == (_queueSize - 1))) {
                    _waiting++;
                    _workerEnd.wait(ul);
                    _waiting--;
                } else {
                    _buffer[_first++] = spt;
                    if (_first == _queueSize)
                        _first = 0;
                    break;
                }
            } else
                return;
        }
    }

    //Found a thread in pool
    if (w != nullptr){
        function<void ()> task = [w, this, spt](){this->_funct(spt); this->WorkerDone(w);};
        w->Process(std::move(task));
    //Run in a new thread
    } else if (runInANewThread){
        SharedWorkerLockless wNoPool = make_shared<workerLockless>();
        function<void ()> task = [wNoPool, this, spt](){this->_funct(spt); this->WorkerNoPoolDone(wNoPool);};
        wNoPool->Process(std::move(task)); //No need to coordinate, its not running yet
        wNoPool->start(std::move(wNoPool));
    //Run in this thread
    } else if (_runOnThisThread){
        _funct(spt);
    }
}

template<class T>
void Dispatcher<T>::WorkerDone(SharedWorker w)
{
    HolderType spt(nullptr);
    {
        unique_lock<mutex> ul(_mutex);

        //Something in the queue, then take it
        if (_last != _first){
            spt = std::move(_buffer[_last++]);
            if(_last == _queueSize)
                _last = 0;
            if (_waiting)
                _workerEnd.notify_one(); //A free slot is ready

        } else {
            //Report a free worker
            _freeWorkers[_freeWorkersIdx++] = std::move(w);
        }
    }
    //Found something in the queue to process
    if (spt != nullptr){
        function<void ()> task = [w, this, spt](){this->_funct(spt); this->WorkerDone(w);};
        w->Process(std::move(task));
    }
}

template<class T>
void Dispatcher<T>::WorkerNoPoolDone(SharedWorkerLockless w)
{
    HolderType spt(nullptr);
    {
        unique_lock<mutex> ul(_mutex);

        //Something in the queue, then take it
        if (_last != _first){
            spt = std::move(_buffer[_last++]);
            if(_last == _queueSize)
                _last = 0;
            _workerEnd.notify_one(); //A free slot is ready
        } else {
            //Report a new thread is creatable
            _actualThreads--;
        }
    }
    //Found something in the queue to process
    if (spt != nullptr)    {
        function<void ()> task = [w, this, spt](){this->_funct(spt); this->WorkerNoPoolDone(w);};
        w->Process(std::move(task));
    } else {
        //Nothing to process, stop me
        w->stop();
    }
}


template<class T>
void Dispatcher<T>::WaitUntilFinish()
{
    for(auto wk = _workers.begin(); wk != _workers.end(); wk++)
        wk->get()->WaitUntilFinish();
}

template<class T>
void Dispatcher<T>::Stop()
{
    for(auto wk = _workers.begin(); wk != _workers.end(); wk++){
        wk->get()->stop();
    }
}


#endif // DISPATCHER_H
