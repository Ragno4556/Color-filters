#pragma once

#include "Monitor.h"

#include <vector>

class MonitorManager
{
  public:
    bool initialize();
    bool restoreAllGammaRamps();

    Monitor &getMonitor(int index);
    const Monitor &getMonitor(int index) const;

    std::vector<Monitor> &getMonitorVector();
    const std::vector<Monitor> &getMonitorVector() const;

  private:
    std::vector<Monitor> monitors;
};
