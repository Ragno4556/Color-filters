#pragma once

#include <Windows.h>
#include "FilterSettings.h"


#include <array>
#include <string>

#include "RGB.h"

class Monitor
{
public:

    using GammaRamp = std::array<std::array<WORD,256>,3>;

    bool init(HMONITOR monitorHandle);

    // Filter control
    bool applyFilter();
    bool applyFilter(const FilterSettings& settings);
    bool restoreGammaRamp();

    // Settings
    FilterSettings& getFilterSettings();
    const FilterSettings& getFilterSettings() const;
    void setFilterSettings(const FilterSettings& settings);

    // Calculated values
    const RGB& getRGB() const;

    // Gamma ramps
    const GammaRamp& getCurrentGamma() const;
    const GammaRamp& getOriginalGamma() const;

    // Device information
    const std::wstring& getDeviceName() const;
    const std::wstring& getFriendlyName() const;
private:

    // Windows monitor data
    std::wstring deviceName;

    GammaRamp originalGamma{};
    GammaRamp currentGamma{};

    // Filter state
    FilterSettings filterSettings{};
    RGB currentRGB{};

    HDC createHDC() const;


};