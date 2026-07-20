#pragma once

#include <vector>

#include "Monitor.h"
#include "FilterSettings.h"

class MonitorManager {
 private:
  std::vector<Monitor> monitors;
  FilterSettings globalSettings;

 public:
  bool initialize();
  bool restoreAllGammaRamps();

  int getMonitorCount() const;

  Monitor& getMonitor(int index);
  const Monitor& getMonitor(int index) const;

  std::vector<Monitor>& getMonitorVector();
  const std::vector<Monitor>& getMonitorVector() const;


  FilterSettings& getGlobalSettings();
  const FilterSettings& getGlobalSettings() const;

  bool applyAll();
  bool applyAllGlobal();
};