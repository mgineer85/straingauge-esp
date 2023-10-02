#pragma once

#include <Arduino.h>
#include <events.hpp>
using namespace esp32m;

class DataEvent : public Event
{
public:
    DataEvent(const char *type, String data) : Event(type), _data(data) {}
    String data() { return _data; }

private:
    String _data;
};