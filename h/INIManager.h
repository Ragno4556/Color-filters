#pragma once

#include "MonitorManager.h"
#include "ini.h"
#include "windows.h"

#include <filesystem>
#include <vector>
#include <string>

//IniData struct stores the ini's name and the .ini's ini structure.
struct IniData{
    std::string iniFilename;
    mINI::INIStructure iniStructure;
};

//IniManager handles the loading, saving, creation, and deletion of ini files.
class IniManager {
private:
    //Reference to the MonitorManager created by mainwindow.cpp. References all monitor data.
    MonitorManager& monitorManager;

    //IniData vector that contains the data of every .ini in the directory. (AppData/Roaming/ColorFilters/Profiles)
    std::vector<IniData> loadedInis;

    //Path where .ini files are stored by default (AppData/Roaming/ColorFilters/Profiles)
    std::filesystem::path profilesDirectory;

    //Helpers
    std::string makeUniqueName(const std::string& fileName);

public:
    //Initialization functions
    //Needed constructor to pass in MonitorManager reference from mainwindow.cpp
    explicit IniManager(MonitorManager& manager) : monitorManager(manager) {}
    bool initialize();

    //Ini modification functions
    void createNewIni(const std::string& fileName);
    void duplicateIni(const std::string& sourceName, const std::string& fileName);
    void deleteIni(const std::string& fileName);


    //Saving and loading functions
    void saveSettingsToIni(const FilterSettings& filterSettings, const std::string& fileName);
    FilterSettings loadSettingsFromIni(const std::string& fileName) const;

    //Getters
    std::vector<IniData>& getLoadedInis();
    const std::vector<IniData>& getLoadedInis() const;
};

