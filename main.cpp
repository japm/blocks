#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <functional>

#include "payload.h"
#include "worker.h"
#include "dispatcher.h"
#include <stdarg.h>
using namespace std;

std::string string_format(const std::string &fmt, ...) {
    int size=100;
    std::string str;
    va_list ap;
    while (1) {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {
            str.resize(n);
            return str;
        }
        if (n > -1)
            size=n+1;
        else
            size*=2;
    }
}


function<void (shared_ptr<string>)> f = [](shared_ptr<string> s){ std::cout << *s ; };

//Dispatcher<string> q(0, 30, false, 0, f);
//Dispatcher<string> q(4, 30, false, 0, f);
Dispatcher<string> q(4, 0, false, 4, f);
//Dispatcher<string> q(4, 0, true, 0, f);
//Dispatcher<string> q(4, 8, true, 10, f);
//Dispatcher<string> q(0, 7, true, 1, f);

void producer(){
    std::chrono::milliseconds dura(0);
    string s("load");
    //auto shs = std::make_shared<string>(s);
    for(int i = 0; i < 15000; i++){
        //auto shs = std::make_shared<string>(string_format("%s%d\n",s.c_str() ,a++));
        auto shs = std::make_shared<string>("load" + to_string(i) + "\n");
        q.Dispatch(shs);
        if ((i % 150) == 0)
            std::this_thread::sleep_for( dura );

    }
}

int main() {
    producer();
    //auto thrp = std::thread(producer);
    q.Stop(false);
    q.WaitUntilFinish();

}
