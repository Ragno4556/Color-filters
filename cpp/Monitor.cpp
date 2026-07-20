#include "Monitor.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "ColorController.h"

HDC Monitor::createHDC() const
{
    HDC hdc = CreateDC(
        L"DISPLAY",
        deviceName.c_str(),
        nullptr,
        nullptr);

    if (hdc == nullptr)
    {
        std::wcerr << L"createHDC: Failed for " << deviceName << L'\n';
        return nullptr;
    }

    return hdc;
}

bool Monitor::init(HMONITOR monitorHandle)
{
    filterSettings = {};
    currentRGB = {1.0f, 1.0f, 1.0f};

    MONITORINFOEX monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);

    if (!GetMonitorInfo(monitorHandle, &monitorInfo))
    {
        std::cerr << "Monitor::init: GetMonitorInfo failed\n";
        return false;
    }

    deviceName = monitorInfo.szDevice;

    // Temporary friendly name until you retrieve the actual display name.

    HDC hdc = createHDC();

    if (hdc == nullptr)
    {
        return false;
    }

    if (!GetDeviceGammaRamp(hdc, originalGamma.data()))
    {
        std::wcerr
            << L"Monitor::init: Failed to read gamma ramp for "
            << deviceName << L'\n';

        DeleteDC(hdc);
        return false;
    }

    currentGamma = originalGamma;

    DeleteDC(hdc);

    std::wcout
        << L"Initialized monitor "
        << deviceName << L'\n';

    return true;
}

bool Monitor::applyFilter()
{
    return applyFilter(filterSettings);
}

bool Monitor::applyFilter(const FilterSettings &settings)
{
    currentRGB = ColorController::calculateRGB(
        settings.tint,
        settings.intensity);

    if (currentRGB.red < 0.0f || currentRGB.red > 1.0f ||
        currentRGB.green < 0.0f || currentRGB.green > 1.0f ||
        currentRGB.blue < 0.0f || currentRGB.blue > 1.0f)
    {
        std::cerr
            << "Monitor::applyFilter: RGB values must be between 0 and 1\n";

        return false;
    }

    if (settings.gamma <= 0.0f)
    {
        std::cerr
            << "Monitor::applyFilter: Gamma must be greater than zero\n";

        return false;
    }

    GammaRamp newGamma{};

    for (std::size_t channel = 0; channel < newGamma.size(); ++channel)
    {
        float colorMultiplier = 1.0f;

        switch (channel)
        {
        case 0:
            colorMultiplier = currentRGB.red;
            break;

        case 1:
            colorMultiplier = currentRGB.green;
            break;

        case 2:
            colorMultiplier = currentRGB.blue;
            break;
        }

        for (std::size_t value = 0;
             value < newGamma[channel].size();
             ++value)
        {
            float normalized =
                static_cast<float>(originalGamma[channel][value]) /
                65535.0f;

            normalized *= colorMultiplier;
            normalized = std::clamp(normalized, 0.0f, 1.0f);
            normalized = std::pow(normalized, settings.gamma);

            newGamma[channel][value] =
                static_cast<WORD>(normalized * 65535.0f);
        }
    }

    HDC hdc = createHDC();

    if (hdc == nullptr)
    {
        return false;
    }

    const BOOL succeeded =
        SetDeviceGammaRamp(hdc, newGamma.data());

    DeleteDC(hdc);

    if (!succeeded)
    {
        std::wcerr
            << L"Monitor::applyFilter: Failed for "
            << deviceName << L'\n';

        return false;
    }

    currentGamma = newGamma;

    return true;
}

bool Monitor::restoreGammaRamp()
{
    HDC hdc = createHDC();

    if (hdc == nullptr)
    {
        return false;
    }

    const BOOL succeeded =
        SetDeviceGammaRamp(hdc, originalGamma.data());

    DeleteDC(hdc);

    if (!succeeded)
    {
        std::wcerr
            << L"Monitor::restoreGammaRamp: Failed for "
            << deviceName << L'\n';

        return false;
    }

    currentGamma = originalGamma;
    currentRGB = {1.0f, 1.0f, 1.0f};

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

void Monitor::setFilterSettings(const FilterSettings &settings)
{
    filterSettings = settings;
}

const RGB &Monitor::getRGB() const
{
    return currentRGB;
}

const Monitor::GammaRamp &Monitor::getCurrentGamma() const
{
    return currentGamma;
}

const Monitor::GammaRamp &Monitor::getOriginalGamma() const
{
    return originalGamma;
}

const std::wstring &Monitor::getDeviceName() const
{
    return deviceName;
}
