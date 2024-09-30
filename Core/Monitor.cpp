#include "Monitor.h"

void Monitor::start()
{
    if(!running)
    {
        running = true;
        counter.start();
    }else
    {
        throw std::logic_error("Monitor already started");
    }
}

void Monitor::stop()
{
    if(running)
    {
        counter.stop();
        running = false;
        calculator.calculateAndWrite(fileManager);
        devices.clear();
    }else
    {
        throw std::logic_error("Monitor has not started");
    }
}
