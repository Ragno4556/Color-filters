#include "INIManager.h"

#include <QDir>
#include <QStandardPaths>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <utility>

namespace
{
std::string monitorSectionName(const Monitor &monitor)
{
    const std::string deviceName = QString::fromStdWString(monitor.getDeviceName()).toUtf8().toStdString();

    if (deviceName.size() > 4)
    {
        return "Monitor" + deviceName.substr(4);
    }

    return "Monitor" + deviceName;
}

bool parseBool(const std::string &text, bool &value)
{
    std::istringstream stream(text);
    stream >> std::boolalpha >> value;
    stream >> std::ws;
    return !stream.fail() && stream.eof();
}

bool parseInt(const std::string &text, int &value)
{
    try
    {
        std::size_t parsedCharacters = 0;
        value = std::stoi(text, &parsedCharacters);
        return parsedCharacters == text.size();
    }
    catch (const std::exception &)
    {
        return false;
    }
}

bool parseFloat(const std::string &text, float &value)
{
    try
    {
        std::size_t parsedCharacters = 0;
        value = std::stof(text, &parsedCharacters);
        return parsedCharacters == text.size();
    }
    catch (const std::exception &)
    {
        return false;
    }
}

bool readProfileSettings(const std::filesystem::path &path, const std::vector<Monitor> &monitors, std::vector<FilterSettings> &settings)
{
    mINI::INIFile file(path);
    mINI::INIStructure ini;

    if (!file.read(ini))
    {
        return false;
    }

    settings.clear();
    settings.reserve(monitors.size());

    for (const Monitor &monitor : monitors)
    {
        const std::string section = monitorSectionName(monitor);
        FilterSettings filterSettings;

        const bool valid = parseFloat(ini.get(section).get("tint"), filterSettings.tint) &&
                           parseFloat(ini.get(section).get("intensity"), filterSettings.intensity) &&
                           parseFloat(ini.get(section).get("gamma"), filterSettings.gamma) &&
                           parseBool(ini.get(section).get("filterToggle"), filterSettings.enabled) && filterSettings.tint >= 0.0f &&
                           filterSettings.tint < 360.0f && filterSettings.intensity >= 0.0f && filterSettings.intensity <= 1.0f && filterSettings.gamma > 0.0f;

        if (!valid)
        {
            return false;
        }

        settings.push_back(filterSettings);
    }

    return true;
}

bool loadHotkey(const mINI::INIStructure &ini, const std::string &prefix, QVector<int> &hotkey)
{
    QVector<int> loadedKeys;

    for (int index = 0; index < 32; ++index)
    {
        const std::string value = ini.get("Settings").get(prefix + std::to_string(index));

        if (value.empty())
        {
            break;
        }

        int key = 0;
        if (!parseInt(value, key))
        {
            return false;
        }

        if (!loadedKeys.contains(key))
        {
            loadedKeys.append(key);
        }
    }

    if (!loadedKeys.empty())
    {
        hotkey = std::move(loadedKeys);
    }

    return true;
}
}

IniManager::IniManager(MonitorManager &monitorManager) : monitorManager(monitorManager) {}

bool IniManager::initialize()
{
    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString profilesPath = appData + "/Profiles";
    const QString globalPath = appData + "/GlobalSettings";

    if (!QDir().mkpath(profilesPath) || !QDir().mkpath(globalPath))
    {
        std::cerr << "IniManager::initialize: Could not create settings folders.\n";
        return false;
    }

    profileDirectory = std::filesystem::path(profilesPath.toStdWString());
    globalSettingsFile = std::filesystem::path(globalPath.toStdWString()) / "GlobalSettings.ini";

    std::error_code fileError;
    const bool globalSettingsExist = std::filesystem::exists(globalSettingsFile, fileError);

    if (fileError)
    {
        std::cerr << "IniManager::initialize: Could not inspect global settings.\n";
        return false;
    }

    if (globalSettingsExist)
    {
        if (!loadGlobalSettings())
        {
            return false;
        }
    }

    if (!loadProfiles())
    {
        return false;
    }

    if (loadedProfiles.empty())
    {
        return createProfile("defaultIni");
    }

    const bool currentProfileExists = std::find(loadedProfiles.cbegin(), loadedProfiles.cend(), settings.currentProfile) != loadedProfiles.cend();

    if (!currentProfileExists)
    {
        settings.currentProfile = loadedProfiles.front();
    }

    return saveGlobalSettings();
}

bool IniManager::createProfile(const std::string &name)
{
    if (!isValidProfileName(name))
    {
        std::cerr << "IniManager::createProfile: Invalid profile name.\n";
        return false;
    }

    const std::string uniqueName = makeUniqueProfileName(name);
    const std::filesystem::path path = profileDirectory / (uniqueName + ".ini");

    mINI::INIFile file(path);
    mINI::INIStructure ini;

    for (const Monitor &monitor : monitorManager.getMonitorVector())
    {
        const std::string section = monitorSectionName(monitor);
        ini[section]["tint"] = "0";
        ini[section]["intensity"] = "0";
        ini[section]["gamma"] = "1";
        ini[section]["filterToggle"] = "true";
    }

    if (!file.generate(ini))
    {
        std::cerr << "IniManager::createProfile: Could not create profile.\n";
        return false;
    }

    loadedProfiles.push_back(uniqueName);
    settings.currentProfile = uniqueName;

    if (saveGlobalSettings())
    {
        return true;
    }

    loadedProfiles.pop_back();
    std::error_code error;
    std::filesystem::remove(path, error);
    return false;
}

bool IniManager::duplicateProfile(const std::string &sourceName, const std::string &newName)
{
    if (!isValidProfileName(newName))
    {
        std::cerr << "IniManager::duplicateProfile: Invalid profile name.\n";
        return false;
    }

    const std::filesystem::path sourcePath = profileDirectory / (sourceName + ".ini");

    std::error_code error;
    if (!std::filesystem::is_regular_file(sourcePath, error) || error)
    {
        std::cerr << "IniManager::duplicateProfile: Source profile is missing.\n";
        return false;
    }

    const std::string uniqueName = makeUniqueProfileName(newName);
    const std::filesystem::path destination = profileDirectory / (uniqueName + ".ini");

    std::filesystem::copy_file(sourcePath, destination, error);

    if (error)
    {
        std::cerr << "IniManager::duplicateProfile: " << error.message() << '\n';
        return false;
    }

    loadedProfiles.push_back(uniqueName);
    settings.currentProfile = uniqueName;

    if (!saveGlobalSettings())
    {
        loadedProfiles.pop_back();
        std::filesystem::remove(destination, error);
        return false;
    }

    return true;
}

bool IniManager::deleteProfile(const std::string &name)
{
    const auto profile = std::find(loadedProfiles.begin(), loadedProfiles.end(), name);

    if (profile == loadedProfiles.end())
    {
        return false;
    }

    std::error_code error;
    const std::filesystem::path path = profileDirectory / (name + ".ini");

    if (!std::filesystem::remove(path, error))
    {
        std::cerr << "IniManager::deleteProfile: Could not delete profile.\n";
        return false;
    }

    loadedProfiles.erase(profile);

    if (loadedProfiles.empty())
    {
        return createProfile("defaultIni");
    }

    if (settings.currentProfile == name)
    {
        settings.currentProfile = loadedProfiles.front();
    }

    return saveGlobalSettings();
}

bool IniManager::importProfile(const std::filesystem::path &sourcePath)
{
    std::string extension = sourcePath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char character) { return static_cast<char>(std::tolower(character)); });

    std::error_code error;
    if (extension != ".ini" || !std::filesystem::is_regular_file(sourcePath, error) || error)
    {
        std::cerr << "IniManager::importProfile: Select a valid INI file.\n";
        return false;
    }

    std::vector<FilterSettings> importedSettings;
    if (!readProfileSettings(sourcePath, monitorManager.getMonitorVector(), importedSettings))
    {
        std::cerr << "IniManager::importProfile: Profile data is invalid.\n";
        return false;
    }

    const std::string uniqueName = makeUniqueProfileName(sourcePath.stem().string());
    const std::filesystem::path destination = profileDirectory / (uniqueName + ".ini");

    std::filesystem::copy_file(sourcePath, destination, error);

    if (error)
    {
        std::cerr << "IniManager::importProfile: " << error.message() << '\n';
        return false;
    }

    loadedProfiles.push_back(uniqueName);
    settings.currentProfile = uniqueName;

    if (!saveGlobalSettings())
    {
        loadedProfiles.pop_back();
        std::filesystem::remove(destination, error);
        return false;
    }

    return true;
}

bool IniManager::loadProfile(const std::string &name)
{
    std::vector<FilterSettings> profileSettings;
    const std::filesystem::path path = profileDirectory / (name + ".ini");

    if (!readProfileSettings(path, monitorManager.getMonitorVector(), profileSettings))
    {
        std::cerr << "IniManager::loadProfile: Could not read " << name << ".ini\n";
        return false;
    }

    std::vector<Monitor> &monitors = monitorManager.getMonitorVector();

    for (std::size_t index = 0; index < monitors.size(); ++index)
    {
        monitors[index].getFilterSettings() = profileSettings[index];
    }

    return true;
}

bool IniManager::saveProfile(const std::string &name) const
{
    if (name.empty())
    {
        return false;
    }

    mINI::INIFile file(profileDirectory / (name + ".ini"));
    mINI::INIStructure ini;

    for (const Monitor &monitor : monitorManager.getMonitorVector())
    {
        const FilterSettings &filterSettings = monitor.getFilterSettings();
        const std::string section = monitorSectionName(monitor);

        ini[section]["tint"] = std::to_string(filterSettings.tint);
        ini[section]["intensity"] = std::to_string(filterSettings.intensity);
        ini[section]["gamma"] = std::to_string(filterSettings.gamma);
        ini[section]["filterToggle"] = filterSettings.enabled ? "true" : "false";
    }

    if (!file.write(ini))
    {
        std::cerr << "IniManager::saveProfile: Could not save " << name << ".ini\n";
        return false;
    }

    return true;
}

bool IniManager::saveGlobalSettings() const
{
    mINI::INIFile file(globalSettingsFile);
    mINI::INIStructure ini;

    ini["Settings"]["exitToTaskbar"] = settings.exitToTaskbar ? "true" : "false";
    ini["Settings"]["runAtStartup"] = settings.runAtStartup ? "true" : "false";
    ini["Settings"]["currentIni"] = settings.currentProfile;

    for (qsizetype index = 0; index < settings.toggleFilterHotkey.size(); ++index)
    {
        ini["Settings"]["toggleFilterHotkey" + std::to_string(index)] = std::to_string(settings.toggleFilterHotkey[index]);
    }

    for (qsizetype index = 0; index < settings.peekHotkey.size(); ++index)
    {
        ini["Settings"]["peekHotkey" + std::to_string(index)] = std::to_string(settings.peekHotkey[index]);
    }

    if (!file.write(ini))
    {
        std::cerr << "IniManager::saveGlobalSettings: Could not save settings.\n";
        return false;
    }

    return true;
}

const std::vector<std::string> &IniManager::profiles() const
{
    return loadedProfiles;
}

const std::filesystem::path &IniManager::profilesDirectory() const
{
    return profileDirectory;
}

GlobalSettings &IniManager::globalSettings()
{
    return settings;
}

const GlobalSettings &IniManager::globalSettings() const
{
    return settings;
}

bool IniManager::loadProfiles()
{
    loadedProfiles.clear();

    std::error_code error;
    std::filesystem::directory_iterator iterator(profileDirectory, error);
    const std::filesystem::directory_iterator end;

    if (error)
    {
        std::cerr << "IniManager::loadProfiles: Could not open profile folder.\n";
        return false;
    }

    while (iterator != end)
    {
        const std::filesystem::directory_entry &entry = *iterator;
        std::error_code entryError;

        if (entry.is_regular_file(entryError) && !entryError && entry.path().extension() == ".ini")
        {
            loadedProfiles.push_back(entry.path().stem().string());
        }

        iterator.increment(error);
        if (error)
        {
            std::cerr << "IniManager::loadProfiles: Could not continue reading the profile folder.\n";
            return false;
        }
    }

    std::sort(loadedProfiles.begin(), loadedProfiles.end());
    return true;
}

bool IniManager::loadGlobalSettings()
{
    mINI::INIFile file(globalSettingsFile);
    mINI::INIStructure ini;

    if (!file.read(ini))
    {
        std::cerr << "IniManager::loadGlobalSettings: Could not read settings.\n";
        return false;
    }

    bool exitToTaskbar = false;
    bool runAtStartup = false;

    if (!parseBool(ini.get("Settings").get("exitToTaskbar"), exitToTaskbar) || !parseBool(ini.get("Settings").get("runAtStartup"), runAtStartup))
    {
        std::cerr << "IniManager::loadGlobalSettings: Invalid settings; using defaults.\n";
        return true;
    }

    settings.exitToTaskbar = exitToTaskbar;
    settings.runAtStartup = runAtStartup;
    settings.currentProfile = ini.get("Settings").get("currentIni");

    if (!loadHotkey(ini, "toggleFilterHotkey", settings.toggleFilterHotkey) || !loadHotkey(ini, "peekHotkey", settings.peekHotkey))
    {
        std::cerr << "IniManager::loadGlobalSettings: Invalid hotkey settings; using defaults.\n";
    }

    return true;
}

bool IniManager::isValidProfileName(const std::string &name) const
{
    return !name.empty() && name.find_first_not_of(" \t\r\n") != std::string::npos && name.find_first_of("<>:\"/\\|?*") == std::string::npos;
}

std::string IniManager::makeUniqueProfileName(const std::string &baseName) const
{
    if (!std::filesystem::exists(profileDirectory / (baseName + ".ini")))
    {
        return baseName;
    }

    int suffix = 1;

    while (true)
    {
        const std::string candidate = baseName + " (" + std::to_string(suffix) + ")";

        if (!std::filesystem::exists(profileDirectory / (candidate + ".ini")))
        {
            return candidate;
        }

        ++suffix;
    }
}
