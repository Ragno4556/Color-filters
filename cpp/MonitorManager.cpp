#include "MonitorManager.h"

#include <Windows.h>

#include <exception>
#include <iostream>
#include <utility>

namespace
{
BOOL CALLBACK addMonitor(HMONITOR monitorHandle, HDC, LPRECT, LPARAM data)
{
    try
    {
        auto *monitors = reinterpret_cast<std::vector<Monitor> *>(data);
        Monitor monitor;

        if (!monitor.initialize(monitorHandle))
        {
            std::cerr << "MonitorManager: Could not initialize a monitor.\n";
            return TRUE;
        }

        monitors->push_back(std::move(monitor));
        return TRUE;
    }
    catch (const std::exception &error)
    {
        std::cerr << "MonitorManager: Monitor enumeration failed: " << error.what() << '\n';
        return FALSE;
    }
}
}

bool MonitorManager::initialize()
{
    monitors.clear();

    if (!EnumDisplayMonitors(nullptr, nullptr, addMonitor, reinterpret_cast<LPARAM>(&monitors)))
    {
        std::cerr << "MonitorManager::initialize: Monitor scan failed.\n";
        return false;
    }

    if (monitors.empty())
    {
        std::cerr << "MonitorManager::initialize: No monitors found.\n";
        return false;
    }

    return true;
}

bool MonitorManager::restoreAllGammaRamps()
{
    bool succeeded = true;

    for (Monitor &monitor : monitors)
    {
        if (!monitor.restoreGammaRamp())
        {
            succeeded = false;
        }
    }

    return succeeded;
}

Monitor &MonitorManager::getMonitor(int index)
{
    return monitors.at(index);
}

const Monitor &MonitorManager::getMonitor(int index) const
{
    return monitors.at(index);
}

std::vector<Monitor> &MonitorManager::getMonitorVector()
{
    return monitors;
}

const std::vector<Monitor> &MonitorManager::getMonitorVector() const
{
    return monitors;
}
