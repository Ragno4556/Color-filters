#include "MonitorManager.h"

#include <Windows.h>

#include <iostream>
#include <utility>

namespace
{
BOOL CALLBACK MonitorEnumProc(
    HMONITOR monitorHandle,
    HDC,
    LPRECT,
    LPARAM data)
{
    auto* monitors =
        reinterpret_cast<std::vector<Monitor>*>(data);

    Monitor monitor;

    if (!monitor.init(monitorHandle))
    {
        std::cerr << "MonitorManager: Failed to initialize monitor\n";

        // Continue searching for other monitors.
        return TRUE;
    }

    monitors->push_back(std::move(monitor));

    return TRUE;
}
}

bool MonitorManager::initialize()
{
    monitors.clear();

    globalSettings = {};

    if (!EnumDisplayMonitors(
            nullptr,
            nullptr,
            MonitorEnumProc,
            reinterpret_cast<LPARAM>(&monitors)))
    {
        std::cerr
            << "MonitorManager::initialize: "
            << "EnumDisplayMonitors failed\n";

        return false;
    }

    if (monitors.empty())
    {
        std::cerr
            << "MonitorManager::initialize: "
            << "No monitors were initialized\n";

        return false;
    }

    std::cout
        << monitors.size()
        << " monitor(s) initialized\n";

    return true;
}

bool MonitorManager::restoreAllGammaRamps()
{
    bool allSucceeded = true;

    for (Monitor& monitor : monitors)
    {
        if (!monitor.restoreGammaRamp())
        {
            allSucceeded = false;
        }
    }

    return allSucceeded;
}

int MonitorManager::getMonitorCount() const
{
    return static_cast<int>(monitors.size());
}

Monitor& MonitorManager::getMonitor(int index)
{
    return monitors.at(index);
}

const Monitor& MonitorManager::getMonitor(int index) const
{
    return monitors.at(index);
}

std::vector<Monitor>& MonitorManager::getMonitorVector()
{
    return monitors;
}

const std::vector<Monitor>& MonitorManager::getMonitorVector() const
{
    return monitors;
}

FilterSettings& MonitorManager::getGlobalSettings()
{
    return globalSettings;
}

const FilterSettings& MonitorManager::getGlobalSettings() const
{
    return globalSettings;
}

bool MonitorManager::applyAll()
{
    bool allSucceeded = true;

    for (Monitor& monitor : monitors)
    {
        if (!monitor.applyFilter())
        {
            allSucceeded = false;
        }
    }

    return allSucceeded;
}

bool MonitorManager::applyAllGlobal()
{
    bool allSucceeded = true;

    for (Monitor& monitor : monitors)
    {
        if (!monitor.applyFilter(globalSettings))
        {
            allSucceeded = false;
        }
    }

    return allSucceeded;
}

