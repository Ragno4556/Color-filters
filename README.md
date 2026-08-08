# ColorFilters

ColorFilters is a lightweight Windows desktop app for applying adjustable color filters to one or more displays. It modifies each display's gamma ramp and restores the original values when the filter is disabled or the app exits.

## Features

- Adjustable tint, intensity, and gamma
- Independent or synchronized multi-display filtering
- Create, duplicate, import, delete, and switch profiles
- Configurable system-wide toggle and hold-to-peek hotkeys
- Optional Windows startup launch
- Optional system-tray operation
- Automatic restoration of original display gamma ramps

## Requirements

- Windows 10 or Windows 11
- Qt 6 with the Widgets module
- A C++17 compiler supported by Qt, such as MinGW-w64 or MSVC

## Build

1. Clone the repository:

   ```bash
   git clone https://github.com/Ragno4556/Color-filters.git
   cd Color-filters
   ```

2. Open `ColorFilters.pro` in Qt Creator.
3. Select a Qt 6 desktop kit.
4. Build and run the project.

From a configured Qt command prompt, you can also build with:

```bash
qmake ColorFilters.pro
mingw32-make
```

Use `nmake` instead of `mingw32-make` when building with MSVC.

## Data storage

Profiles and global settings are stored under the application's Windows AppData directory. ColorFilters does not modify the repository or require administrator access during normal use.

## Notes

- Global shortcuts are active while ColorFilters is running, including when it is minimized or hidden in the system tray.
- Some displays or graphics drivers may not support software gamma-ramp changes.
- Windows secure-desktop screens are not affected by the app or its shortcuts.

## License

ColorFilters is available under the [MIT License](LICENSE).
