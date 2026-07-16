#define UNICODE
#define _UNICODE

#include <windows.h>
#include <vector>

#include "Monitor.h"
#include "MonitorManager.h"
#include "ConfigManager.h"

int main()
{

    ConfigManager manager;
    manager.createConfigFile();

    MonitorManager monitors;
    monitors.initialize();

    bool filterEnabled = false;
    bool wasKeyPressed = false;

    while (true)
    {
        bool isKeyPressed = (GetAsyncKeyState(VK_F8) & 0x8000);
        if (isKeyPressed && !wasKeyPressed)
        {
            if (filterEnabled)
            {
                monitors.restoreAll();
            }
            else
            {
                monitors.setGammaAll(1, 0, 0);
            }
            filterEnabled = !filterEnabled;
        }
        wasKeyPressed = isKeyPressed;
        Sleep(10);
    }
}
