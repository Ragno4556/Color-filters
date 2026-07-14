#pragma once

#define UNICODE
#define _UNICODE

#include <Windows.h>
#include <string>

class Monitor
{
private:
    std::wstring deviceName;
    WORD originalGamma[3][256];
    WORD currentGamma[3][256];
    HDC createHDC();

public:
    bool init(HMONITOR hmonitor);
    bool setGamma(float red, float green, float blue);
    bool restoreGamma();
};
