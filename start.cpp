#define UNICODE
#define _UNICODE
#include <windows.h>
#include <iostream>
#include <array>    // Optional
#include <cstdint>  // Optional
#include <vector>
#include <dxgi1_6.h>
#include <wrl/client.h> // For Microsoft::WRL::ComPtr
#include <cstdlib>

class Monitor {
    
    private:
    std::wstring deviceName;
    WORD originalGamma[3][256];
    WORD currentGamma[3][256];
    HDC createHDC(){
        HDC hdc = CreateDC(L"DISPLAY", deviceName.c_str(), nullptr,nullptr);
        if(hdc == NULL){
            std::cout << "Function createHDC failed. HDC = nullptr\n";
            return nullptr;
        }
        std::wcout << "Function createHDC succeeded for " << deviceName << "\n";
        return hdc;
    };
    
    public:
    bool init(HMONITOR hmonitor){
        MONITORINFOEX monitorInfo{};
        monitorInfo.cbSize = sizeof(MONITORINFOEX);
        
        if(!GetMonitorInfo(hmonitor, &monitorInfo)){
            std::cout << "Function init failed to get monitor info\n"; 
            return false;
        }
        deviceName = monitorInfo.szDevice;
        std::wcout << "Function init successfully obtained monitor info for " << deviceName << "\n"; 
        
            
        HDC hdc = createHDC();

        if(hdc == nullptr){
            std::cout << "init: HDC was nullptr\n";
            return false;
        }

        if(!GetDeviceGammaRamp(hdc, originalGamma)){
            std::wcout << "init: gamma did not read correctly for " << deviceName << "\n";
            DeleteDC(hdc);
            return false;
        }
        std::copy(
            &originalGamma[0][0],
            &originalGamma[0][0] + 3 * 256,
            &currentGamma[0][0]
        );
        DeleteDC(hdc);
        return TRUE;
    }
    
    bool setGamma(float red, float blue, float green){
        if(red > 1.0 || blue > 1.0 || green > 1.0){
            std::cout << "setGamma: Float values cannot be greater than 1\n";
            return false;
        }
        HDC hdc = createHDC();
        if(hdc == nullptr){
            std::cout << "setGamma: HDC was nullptr\n";
            return false;
        }

        WORD tempGamma[3][256];
        std::copy(&originalGamma[0][0], &originalGamma[0][0] + 3 * 256, &tempGamma[0][0]);

        for(int i = 0;i<256;i++){
            tempGamma[0][i] = tempGamma[0][i] * red;
            tempGamma[1][i] = tempGamma[1][i] * green;
            tempGamma[2][i] = tempGamma[2][i] * blue;
        }

        if(!SetDeviceGammaRamp(hdc, tempGamma)){
            std::wcout << "setGamma: Failed to set gamma ramp for " << deviceName << "\n";
            DeleteDC(hdc);
            return false;
        }
        std::wcout << "Successfully set gamma ramp for " << deviceName << "\n";
        std::copy(&tempGamma[0][0], &tempGamma[0][0] + 3 * 256, &currentGamma[0][0]);
        DeleteDC(hdc);
        return true;
    }
    bool restoreGamma(){
        HDC hdc = createHDC();
        if(hdc == nullptr){
            std::cout << "restoreGamma: HDC is nullptr\n";
            return false;
        }

        WORD tempGamma[3][256];

        for(int i = 0;i<256;i++){
            tempGamma[0][i] = originalGamma[0][i];
            tempGamma[1][i] = originalGamma[1][i];
            tempGamma[2][i] = originalGamma[2][i];
        }

         if(!SetDeviceGammaRamp(hdc, tempGamma)){
            std::wcout << "restoreGamma: Failed to set gamma ramp for " << deviceName << "\n";
            DeleteDC(hdc);
            return false;
        }
        std::wcout << "restoreGamma: Successfully set gamma ramp for " << deviceName << "\n";
        std::copy(&tempGamma[0][0], &tempGamma[0][0] + 3 * 256, &currentGamma[0][0]);
        DeleteDC(hdc);
        return true;
    }
    

};


BOOL CALLBACK MonitorEnumProc(
    HMONITOR hmonitor,
    HDC hdcMonitor,
    LPRECT lprcMOnitor,
    LPARAM dwDATA)
{
    auto* monitors = reinterpret_cast<std::vector<Monitor>*>(dwDATA);

    Monitor monitor;

    if(monitor.init(hmonitor)){
        monitors->push_back(monitor);
    }

    return true;

}


int main(){
    std::vector<Monitor> monitors;
    EnumDisplayMonitors(nullptr,nullptr,MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));

    bool filterEnabled = false;
    bool previousPressed = false;
    

    while(true){
        bool currentKeyPressed = (GetAsyncKeyState(VK_F8) & 0x8000);

        if(currentKeyPressed && !previousPressed){
            if(!filterEnabled){
                for(int i = 0; i < monitors.size(); i++){
                    monitors[i].setGamma(1,0,0);
                }
                filterEnabled = true;
                std::cout << "main: Filter enabled\n";

            }
            else{
                for(int i = 0; i < monitors.size(); i++){
                    monitors[i].restoreGamma();
                }
                filterEnabled = false;
                std::cout << "Filter Disabled\n";
                
            }

        }
        previousPressed = currentKeyPressed;

        Sleep(10);
        


    }
    



    
}

