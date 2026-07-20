#include "INIManager.h"

#include <QStandardPaths>
#include <QDir>
#include <QInputDialog>

#include <vector>
#include <string>
#include <cstdio>
#include <iostream>

    //Creates a new .ini and adds it to loadedInis
    void IniManager::createNewIni(const std::string& fileName){

    //File name error cases
    if(fileName.empty() || fileName.find_first_not_of(" \t\r\n") == std::string::npos){
        MessageBox(NULL, L"File name cannot be empty.", L"Invalid FIle Name", MB_OK | MB_ICONINFORMATION);
        return;
    }
    if (fileName.find_first_of("<>:\"/\\|?*") != std::string::npos){
        MessageBox(nullptr, L"File name contains invalid characters.", L"Invalid File Name", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string uniqueName = makeUniqueName(fileName);
    mINI::INIFile file((profilesDirectory / (uniqueName + ".ini")).string());
    mINI::INIStructure iniStructure;

    //Create converter
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    //Cyrcle through each monitors settings
    for(Monitor& monitor : monitorManager.getMonitorVector()){

        //Convert wstring t string
        std::string s = converter.to_bytes(monitor.getDeviceName());

        std::string monitorCategory = "Monitor";

        if(s.size() > 4){
            monitorCategory = ("Monitor" + s.substr(4));
        }
        else{
            monitorCategory = ("Monitor" + s);
        }

        //Fill with defaults
        iniStructure[monitorCategory]["tint"] = "0";
        iniStructure[monitorCategory]["intensity"] = "0";
        iniStructure[monitorCategory]["gamma"] = "1";
        iniStructure[monitorCategory]["filterToggle"] = "true";
    }

    //Don't update loadedInis until the file is successfully generated
    if(!file.generate(iniStructure)){
        std::cerr << "createNewIni(): file.generate failed to create the ini file.\n";
        return;
    }

    //Added name and strcuture to loadedInis
    IniData newIni = {uniqueName, iniStructure};
    loadedInis.push_back(std::move(newIni));
}


//Creates a new .ini based on a different .ini
void IniManager::duplicateIni(const std::string& sourceName, const std::string& fileName){

    //File name error cases
    if(fileName.empty() || fileName.find_first_not_of(" \t\r\n") == std::string::npos){
        MessageBox(NULL, L"File name cannot be empty.", L"Invalid FIle Name", MB_OK | MB_ICONINFORMATION);
        return;
    }
    if (fileName.find_first_of("<>:\"/\\|?*") != std::string::npos){
        MessageBox(nullptr, L"File name contains invalid characters.", L"Invalid File Name", MB_OK | MB_ICONWARNING);
        return;
    }

    std::string uniqueName = makeUniqueName(fileName);

    //Check if the file actaully exists!
    if(!std::filesystem::exists((profilesDirectory / (sourceName + ".ini")).string())){
        std::cerr << "duplicateIni(): Source file does not exist.\n";
        return;
    }

    //Find the stored source file and read its ini structure
    mINI::INIFile sourceFile((profilesDirectory / (sourceName + ".ini")).string());
    mINI::INIStructure iniStructure;

    //Check if the file was read correctly, could be corrupted or inaccesible
    if(!sourceFile.read(iniStructure)){
        std::cerr << "duplicateIni(): Failed to read the source INI file.\n";
        return;
    }

    mINI::INIFile newFile((profilesDirectory / (uniqueName + ".ini")).string());

    //Don't update loadedInis until the file is successfully generated
    if(!newFile.generate(iniStructure)){
        std::cerr << "duplicateIni(): file.generate failed to create the ini file.\n";
        return;
    }

    //Added name and structure to loadedInis
    IniData newI = {uniqueName, iniStructure};
    loadedInis.push_back(std::move(newI));
}

//Initializes INIManager, finds default /Profiles location, reads all .ini files, and adds their name and structure to loadedInis.
bool IniManager::initialize(){
    //Finds default /Profiles location
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString profiles = appData + "/Profiles";

    //Trys to make path
    if(!QDir().mkpath(profiles)){
        std::cerr << "profilesDirectory(): Could not access default profies directory.";
        return false;
    }

    profilesDirectory = std::filesystem::path(profiles.toStdString());

    //Fills loadedInis with structures from .ini files from /Profiles
    loadedInis.clear();


    //Check if iterator errors
    std::error_code error;
    std::filesystem::directory_iterator iterator(profilesDirectory,error);

    if(error){
        std::cerr << "IniManager::initialize(): Could not iterate through profiles directory.\n";
        return false;
    }

    //Add each .ini
    for(const auto& entry : iterator){

        //Check if current file is a .ini file
        if(!entry.is_regular_file() || entry.path().extension() != ".ini"){
            std::cerr << "IniManager::initialize(): File " + (entry.path().filename()).string() + ", it is not a .ini file.\n";
            continue;
        }

        mINI::INIFile file(entry.path());
        mINI::INIStructure data;

        //Check if readable
        if(!file.read(data)){
            std::cerr << "IniManager::initialize(): Failed to read the source INI file.\n";
            continue;
        }

        //Add file to loadedInis
        IniData newI = {entry.path().stem().string(),data};
        loadedInis.push_back(std::move(newI));
    }
    return true;
}

//Checks if the file name already exists in profile directory, if it does then it creates an alternative by adding a number to the end
std::string IniManager::makeUniqueName(const std::string& baseName)
{
    //Checks if given name is empty
    if (baseName.empty())
    {
        std::cerr << "makeUniqueName(): baseName was empty, returning ""\n";
        return "";
    }

    namespace fs = std::filesystem;

    //Checks if the filename already exits, if it doesn't, baseName is good to go
    if (!fs::exists(profilesDirectory / (baseName + ".ini"))){
        return baseName;
    }


    //Else it adds a number to the end
    int number = 1;

    while (true){
        std::string candidate = baseName + " (" + std::to_string(number) + ")";

        if (!fs::exists(profilesDirectory / (candidate + ".ini"))){
            return candidate;
        }

        ++number;
    }
}


//Deletes .ini from loadedInis and files
void IniManager::deleteIni(const std::string& fileName){

    //Check if able to remove the file
    if(!std::filesystem::remove(std::filesystem::path(profilesDirectory / (fileName + ".ini")))){
        std::cerr << "deleteIni(): Failed to remove file from the file system\n";
        return;
    }

    //Remove ini data from loadedInis
    for(size_t i = 0; i<loadedInis.size(); i++){
        if((loadedInis[i].iniFilename) == fileName){
            loadedInis.erase(loadedInis.begin() + i);
            return;
        }
    }
}

std::vector<IniData>& IniManager::getLoadedInis(){
    return loadedInis;
}

const std::vector<IniData>& IniManager::getLoadedInis() const{
    return loadedInis;
}
