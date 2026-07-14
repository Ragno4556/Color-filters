#define UNICODE
#define _UNICODE

#include "Monitor.h"

#include <iostream>
#include <algorithm>

HDC Monitor::createHDC()
{
    HDC hdc = CreateDC(L"DISPLAY", deviceName.c_str(), nullptr, nullptr);
    if (hdc == NULL)
    {
        std::cout << "Function createHDC failed. HDC = nullptr\n";
        return nullptr;
    }
    std::wcout << L"Function createHDC succeeded for " << deviceName << L"\n";
    return hdc;
}

bool Monitor::init(HMONITOR hmonitor)
{
    MONITORINFOEX monitorInfo{};
    monitorInfo.cbSize = sizeof(MONITORINFOEX);

    if (!GetMonitorInfo(hmonitor, &monitorInfo))
    {
        std::cout << "Function init failed to get monitor info\n";
        return false;
    }
    deviceName = monitorInfo.szDevice;
    std::wcout << L"Function init successfully obtained monitor info for " << deviceName << L"\n";

    HDC hdc = createHDC();

    if (hdc == nullptr)
    {
        std::cout << "init: HDC was nullptr\n";
        return false;
    }

    if (!GetDeviceGammaRamp(hdc, originalGamma))
    {
        std::wcout << L"init: gamma did not read correctly for " << deviceName << L"\n";
        DeleteDC(hdc);
        return false;
    }
    std::copy(
        &originalGamma[0][0],
        &originalGamma[0][0] + 3 * 256,
        &currentGamma[0][0]);
    DeleteDC(hdc);
    return true;
}

bool Monitor::setGamma(float red, float blue, float green)
{
    if (red > 1.0 || blue > 1.0 || green > 1.0)
    {
        std::cout << "setGamma: Float values cannot be greater than 1\n";
        return false;
    }
    HDC hdc = createHDC();
    if (hdc == nullptr)
    {
        std::cout << "setGamma: HDC was nullptr\n";
        return false;
    }

    WORD tempGamma[3][256];
    std::copy(&originalGamma[0][0], &originalGamma[0][0] + 3 * 256, &tempGamma[0][0]);

    for (int i = 0; i < 256; i++)
    {
        tempGamma[0][i] = tempGamma[0][i] * red;
        tempGamma[1][i] = tempGamma[1][i] * blue;
        tempGamma[2][i] = tempGamma[2][i] * green;
    }

    if (!SetDeviceGammaRamp(hdc, tempGamma))
    {
        std::wcout << L"setGamma: Failed to set gamma ramp for " << deviceName << L"\n";
        DeleteDC(hdc);
        return false;
    }
    std::wcout << L"Successfully set gamma ramp for " << deviceName << L"\n";
    std::copy(&tempGamma[0][0], &tempGamma[0][0] + 3 * 256, &currentGamma[0][0]);
    DeleteDC(hdc);
    return true;
}
bool Monitor::restoreGamma()
{
    HDC hdc = createHDC();
    if (hdc == nullptr)
    {
        std::cout << "restoreGamma: HDC is nullptr\n";
        return false;
    }

    WORD tempGamma[3][256];

    for (int i = 0; i < 256; i++)
    {
        tempGamma[0][i] = originalGamma[0][i];
        tempGamma[1][i] = originalGamma[1][i];
        tempGamma[2][i] = originalGamma[2][i];
    }

    if (!SetDeviceGammaRamp(hdc, tempGamma))
    {
        std::wcout << "restoreGamma: Failed to set gamma ramp for " << deviceName << "\n";
        DeleteDC(hdc);
        return false;
    }
    std::wcout << "restoreGamma: Successfully set gamma ramp for " << deviceName << "\n";
    std::copy(&tempGamma[0][0], &tempGamma[0][0] + 3 * 256, &currentGamma[0][0]);
    DeleteDC(hdc);
    return true;
}