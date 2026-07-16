#pragma once

#define UNICODE
#define _UNICODE

#include <string>
#include <iostream>
#include "ini.h"
#include <filesystem>
#include <Windows.h>
#include <sstream>

class ConfigManager
{
public:
    bool createConfigFile()
    {
        std::cout << "File name (no .ini): \n";
        std::string filename;
        std::cin >> filename;
        std::filesystem::path PATH = std::filesystem::current_path() / "configs" / (filename + ".ini");
        mINI::INIFile file(filename + ".ini");
        mINI::INIStructure ini;

        float redFloat;
        std::cout << "Red float: \n";
        std::cin >> redFloat;
        ini["Config"]["RedFloat"] = redFloat;

        float greenFloat;
        std::cout << "Red float: \n";
        std::cin >> greenFloat;
        ini["Config"]["GreenFloat"] = greenFloat;

        float blueFloat;
        std::cout << "Blue float: \n";
        std::cin >> blueFloat;
        ini["Config"]["blueFloat"] = blueFloat;

        int vKey;
        std::cout << "Press desired hotkey: \n";

        bool x = true;

        while (true)
        {
            // Iterate through valid virtual-key code ranges (1 to 254)
            for (int vkey = 1; vkey <= 254; vkey++)
            {

                // Check if the current vkey is pressed
                if (GetAsyncKeyState(vkey) & 0x8000)
                {

                    vKey = vkey;
                    // Stop loop if Escape key is pressed
                    if (vkey == VK_ESCAPE)
                    {
                        vkey = 300;
                        x = false;
                    }

                    // Debounce delay to prevent flooding the console
                    Sleep(150);
                }
            }
            // Small yield to prevent 100% CPU usage
            Sleep(1);
        }
        ini["Config"]["Hotkey"] = vKey;

        return true;
    }
};