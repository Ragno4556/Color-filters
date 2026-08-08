#pragma once

#include "FilterSettings.h"

#include <Windows.h>

#include <array>
#include <string>

class Monitor
{
  public:
    using GammaRamp = std::array<std::array<WORD, 256>, 3>;

    bool initialize(HMONITOR monitorHandle);

    bool applyFilter();
    bool restoreGammaRamp();

    FilterSettings &getFilterSettings();
    const FilterSettings &getFilterSettings() const;

    const std::wstring &getDeviceName() const;

  private:
    std::wstring deviceName;
    GammaRamp originalGamma{};
    FilterSettings filterSettings{};

    HDC createHDC() const;
};
