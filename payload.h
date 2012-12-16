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

#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <string>
#include <utility>
#include <map>

using namespace std;
template <class T>

class Payload
{
public:

    Payload(const T&  other);
    Payload(T&& other);
    Payload(const Payload<T>& other);
    Payload(Payload<T>&& other);
    Payload<T>& operator=(Payload<T>&& other);

    const T& getPayload();
    map<string,string>& getMetadata();

private:
    map<string,string> _mdata;
    T                  _data;


};


template<class T>
Payload<T>::Payload(const T &data) : _data(data), _mdata()
{
}

template<class T>
Payload<T>::Payload(T&& data) : _data(std::move(data)), _mdata()
{
}


template<class T>
Payload<T>::Payload(const Payload &other)
{
    this->_data = other._data;
    this->_mdata = other._mdata;

}

template<class T>
Payload<T>::Payload(Payload&& other)
{
    *this = std::move(other);
}

template<class T>
Payload<T> &Payload<T>::operator =(Payload<T>&& other)
{
     if (this == &other)
         return *this;
     this->_data = std::move(other._data);
     this->_mdata = std::move(other._mdata);
     return *this;

}

template<class T>
const   T &Payload<T>::getPayload()
{
    return this->_data;
}

template<class T>
map<string,string> &Payload<T>::getMetadata()
{
    return _mdata;
}


#endif // PAYLOAD_H
