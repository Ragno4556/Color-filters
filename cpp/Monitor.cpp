#include "Monitor.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "ColorController.h"

HDC Monitor::createHDC() const
{
    HDC hdc = CreateDC(L"DISPLAY", deviceName.c_str(), nullptr, nullptr);

    if (hdc == nullptr)
    {
        std::wcerr << L"createHDC: Failed for " << deviceName << L'\n';
        return nullptr;
    }

    return hdc;
}

bool Monitor::initialize(HMONITOR monitorHandle)
{
    filterSettings = {};

    MONITORINFOEX monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfo(monitorHandle, &monitorInfo))
    {
        std::cerr << "Monitor::initialize: GetMonitorInfo failed.\n";
        return false;
    }

    deviceName = monitorInfo.szDevice;

    HDC hdc = createHDC();

    if (hdc == nullptr)
    {
        return false;
    }

    if (!GetDeviceGammaRamp(hdc, originalGamma.data()))
    {
        std::wcerr << L"Monitor::initialize: Failed to read gamma ramp for " << deviceName << L'\n';

        DeleteDC(hdc);
        return false;
    }

    DeleteDC(hdc);
    return true;
}

bool Monitor::applyFilter()
{
    const RGB rgb = ColorController::calculateRGB(filterSettings.tint, filterSettings.intensity);

    if (rgb.red < 0.0f || rgb.red > 1.0f || rgb.green < 0.0f || rgb.green > 1.0f || rgb.blue < 0.0f || rgb.blue > 1.0f)
    {
        std::cerr << "Monitor::applyFilter: RGB values must be between 0 and 1\n";

        return false;
    }

    if (filterSettings.gamma <= 0.0f)
    {
        std::cerr << "Monitor::applyFilter: Gamma must be greater than zero\n";

        return false;
    }

    GammaRamp newGamma{};

    for (std::size_t channel = 0; channel < newGamma.size(); ++channel)
    {
        float colorMultiplier = 1.0f;

        switch (channel)
        {
        case 0:
            colorMultiplier = rgb.red;
            break;

        case 1:
            colorMultiplier = rgb.green;
            break;

        case 2:
            colorMultiplier = rgb.blue;
            break;
        }

        for (std::size_t value = 0; value < newGamma[channel].size(); ++value)
        {
            float normalized = static_cast<float>(originalGamma[channel][value]) / 65535.0f;

            normalized *= colorMultiplier;
            normalized = std::clamp(normalized, 0.0f, 1.0f);
            normalized = std::pow(normalized, filterSettings.gamma);

            newGamma[channel][value] = static_cast<WORD>(normalized * 65535.0f);
        }
    }

    HDC hdc = createHDC();

    if (hdc == nullptr)
    {
        return false;
    }

    const BOOL succeeded = SetDeviceGammaRamp(hdc, newGamma.data());

    DeleteDC(hdc);

    if (!succeeded)
    {
        std::wcerr << L"Monitor::applyFilter: Failed for " << deviceName << L'\n';

        return false;
    }

    return true;
}

bool Monitor::restoreGammaRamp()
{
    HDC hdc = createHDC();

    if (hdc == nullptr)
    {
        return false;
    }

    const BOOL succeeded = SetDeviceGammaRamp(hdc, originalGamma.data());

    DeleteDC(hdc);

    if (!succeeded)
    {
        std::wcerr << L"Monitor::restoreGammaRamp: Failed for " << deviceName << L'\n';

        return false;
    }

    return true;
}

FilterSettings &Monitor::getFilterSettings()
{
    return filterSettings;
}

const FilterSettings &Monitor::getFilterSettings() const
{
    return filterSettings;
}

const std::wstring &Monitor::getDeviceName() const
{
    return deviceName;
}
