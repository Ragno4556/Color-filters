#pragma once

#include "MonitorManager.h"
#include "ini.h"

#include <QVector>
#include <filesystem>
#include <string>
#include <vector>

struct GlobalSettings
{
    bool exitToTaskbar = false;
    bool runAtStartup = false;
    std::string currentProfile;
    QVector<int> toggleFilterHotkey = {Qt::Key_Control, Qt::Key_K};
    QVector<int> peekHotkey = {Qt::Key_Control, Qt::Key_P};
};

class IniManager
{
  public:
    explicit IniManager(MonitorManager &monitorManager);

    bool initialize();

    bool createProfile(const std::string &name);
    bool duplicateProfile(const std::string &sourceName, const std::string &newName);
    bool deleteProfile(const std::string &name);
    bool importProfile(const std::filesystem::path &sourcePath);

    bool loadProfile(const std::string &name);
    bool saveProfile(const std::string &name) const;
    bool saveGlobalSettings() const;

    const std::vector<std::string> &profiles() const;
    const std::filesystem::path &profilesDirectory() const;

    GlobalSettings &globalSettings();
    const GlobalSettings &globalSettings() const;

  private:
    MonitorManager &monitorManager;
    std::vector<std::string> loadedProfiles;
    std::filesystem::path profileDirectory;
    std::filesystem::path globalSettingsFile;
    GlobalSettings settings;

    bool loadProfiles();
    bool loadGlobalSettings();
    bool isValidProfileName(const std::string &name) const;
    std::string makeUniqueProfileName(const std::string &baseName) const;
};
