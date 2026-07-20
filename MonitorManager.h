#pragma once

#define UNICODE
#define _UNICODE

#include <vector>

#include "Monitor.h"

class MonitorManager
{
private:
    std::vector<Monitor> monitors;

public:
    bool initialize();
    bool setGammaAll(float red, float green, float blue);
    bool restoreAll();
    int getMonitorcount();
};