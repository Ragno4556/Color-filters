# ColorFilters

ColorFilters is a Windows desktop application that applies customizable color filters to one or more monitors by modifying the display gamma ramp.

The project is written in C++ using Qt and the Windows GDI API.

## Features

- Adjustable color tint
- Adjustable filter intensity
- Adjustable gamma
- Multi-monitor support
- Save and load filter profiles
- Duplicate profiles
- Hotkey support (planned)
- Start with Windows (planned)
- Start minimized (planned)

## How It Works

ColorFilters enumerates each connected monitor using the Windows API and stores its original gamma ramp.

When a filter is applied, the application generates a new gamma ramp using the selected tint, intensity, and gamma values, then applies it to each monitor individually.

The application uses the following Windows APIs:

- `EnumDisplayMonitors`
- `CreateDC`
- `GetDeviceGammaRamp`
- `SetDeviceGammaRamp`

When the filter is disabled, the original gamma ramps are restored.

## Project Structure

```
ColorFilters/
│
├── cpp/                Source files
├── h/                  Header files
├── form/               Qt Designer UI files
├── resources/          Icons and other resources
├── ColorFilters.pro    Qt project file
└── resources.qrc
```

## Requirements

- Windows 10 or Windows 11
- Qt 6
- C++17
- MinGW or MSVC

## Building

Clone the repository:

```bash
git clone https://github.com/Ragno4556/Color-filters.git
```

Open `ColorFilters.pro` in Qt Creator and build the project.

## Future Plans

- Global hotkeys
- System tray support
- Import and export profiles
- Additional startup options

## License

MIT License
