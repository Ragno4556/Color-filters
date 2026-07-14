#define UNICODE
#define _UNICODE

#include "MonitorManager.h"

#include <windows.h>
#include <iostream>

static BOOL CALLBACK MonitorEnumProc(
    HMONITOR hmonitor,
    HDC hdcMonitor,
    LPRECT lprcMOnitor,
    LPARAM dwDATA)
{
    auto *monitors = reinterpret_cast<std::vector<Monitor> *>(dwDATA);

    Monitor monitor;

    if (monitor.init(hmonitor))
    {
        monitors->push_back(monitor);
    }
    else
    {
        std::wcout << L"Error initializing monitor\n";
        return FALSE;
    }

    return TRUE;
}

bool MonitorManager::initialize()
{
    if (!EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors)))
    {
        std::cout << "Failed to initilize monitor manager!";
        return false;
    }
    std::cout << monitors.size() << " monitors initialized!";
    return true;
}

bool MonitorManager::setGammaAll(float red, float green, float blue)
{
    for (int i = 0; i < monitors.size(); i++)
    {
        monitors[i].setGamma(red, green, blue);
    }
    return true;
}

bool MonitorManager::restoreAll()
{
    for (int i = 0; i < monitors.size(); i++)
    {
        monitors[i].restoreGamma();
    }
    return true;
}

int MonitorManager::getMonitorcount()
{
    return monitors.size();
}