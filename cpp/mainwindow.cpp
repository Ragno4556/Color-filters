#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeySequence>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QStringList>
#include <QSystemTrayIcon>
#include <QTabBar>

#include <Shellapi.h>
#include <Windows.h>

#include <iostream>

namespace
{
QString hotkeyText(const QVector<int> &keys)
{
    QStringList names;
    names.reserve(keys.size());

    for (const int key : keys)
    {
        names.append(QKeySequence(key).toString());
    }

    return names.join(" + ");
}

int qtKeyFromVirtualKey(UINT virtualKey)
{
    if ((virtualKey >= '0' && virtualKey <= '9') || (virtualKey >= 'A' && virtualKey <= 'Z'))
    {
        return static_cast<int>(virtualKey);
    }

    if (virtualKey >= VK_F1 && virtualKey <= VK_F24)
    {
        return Qt::Key_F1 + static_cast<int>(virtualKey - VK_F1);
    }

    switch (virtualKey)
    {
    case VK_CONTROL:
    case VK_LCONTROL:
    case VK_RCONTROL:
        return Qt::Key_Control;
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
        return Qt::Key_Shift;
    case VK_MENU:
    case VK_LMENU:
    case VK_RMENU:
        return Qt::Key_Alt;
    case VK_LWIN:
    case VK_RWIN:
        return Qt::Key_Meta;
    case VK_SPACE:
        return Qt::Key_Space;
    case VK_TAB:
        return Qt::Key_Tab;
    case VK_RETURN:
        return Qt::Key_Return;
    case VK_ESCAPE:
        return Qt::Key_Escape;
    case VK_BACK:
        return Qt::Key_Backspace;
    case VK_DELETE:
        return Qt::Key_Delete;
    case VK_INSERT:
        return Qt::Key_Insert;
    case VK_HOME:
        return Qt::Key_Home;
    case VK_END:
        return Qt::Key_End;
    case VK_PRIOR:
        return Qt::Key_PageUp;
    case VK_NEXT:
        return Qt::Key_PageDown;
    case VK_LEFT:
        return Qt::Key_Left;
    case VK_RIGHT:
        return Qt::Key_Right;
    case VK_UP:
        return Qt::Key_Up;
    case VK_DOWN:
        return Qt::Key_Down;
    default:
        break;
    }

    const UINT character = MapVirtualKeyW(virtualKey, MAPVK_VK_TO_CHAR) & 0x7fffffffU;

    if (character >= 0x20 && character <= 0x7e)
    {
        return static_cast<int>(character);
    }

    return Qt::Key_unknown;
}
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), iniManager(monitorManager)
{
    ui->setupUi(this);
    ui->tabSelector->tabBar()->setExpanding(true);
    setWindowFlags((windowFlags() | Qt::FramelessWindowHint) & ~Qt::WindowMaximizeButtonHint);

    ui->appIconLabel->setPixmap(windowIcon().pixmap(16, 16));
    ui->titleBar->installEventFilter(this);
    ui->appIconLabel->installEventFilter(this);
    ui->appTitleLabel->installEventFilter(this);

    connect(ui->minimizeButton, &QToolButton::clicked, this, &MainWindow::showMinimized);
    connect(ui->closeButton, &QToolButton::clicked, this, &MainWindow::close);

    registerGlobalKeyboardInput();

    setupTrayIcon();
    initialized = monitorManager.initialize() && iniManager.initialize();

    if (initialized)
    {
        refreshMonitorSelector();
        refreshProfileSelector();
        loadGlobalControls();
        reloadCurrentProfile();
        connectSignals();
        trayIcon->setVisible(iniManager.globalSettings().exitToTaskbar);
    }
    else
    {
        ui->tabSelector->setEnabled(false);
        QMessageBox::critical(this, "Color Filters", "Color Filters could not initialize its displays or settings.");
    }
}

MainWindow::~MainWindow()
{
    if (initialized)
    {
        if (peekHotkeyActive)
        {
            stopPeek();
        }

        saveAllSettings();
    }

    monitorManager.restoreAllGammaRamps();
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    const bool isTitleBar = watched == ui->titleBar || watched == ui->appIconLabel || watched == ui->appTitleLabel;

    if (!isTitleBar)
    {
        return QMainWindow::eventFilter(watched, event);
    }

    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->button() == Qt::LeftButton)
        {
            draggingWindow = true;
            windowDragOffset = mouseEvent->globalPosition().toPoint() - frameGeometry().topLeft();
            return true;
        }
    }
    else if (event->type() == QEvent::MouseMove && draggingWindow)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->buttons().testFlag(Qt::LeftButton))
        {
            move(mouseEvent->globalPosition().toPoint() - windowDragOffset);
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        draggingWindow = false;
        return true;
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setupTrayIcon()
{
    trayIcon = new QSystemTrayIcon(windowIcon(), this);
    trayIcon->setToolTip("Color Filters");

    QMenu *trayMenu = new QMenu(this);
    QAction *openAction = trayMenu->addAction("Open");
    QAction *quitAction = trayMenu->addAction("Quit");
    trayIcon->setContextMenu(trayMenu);

    connect(openAction, &QAction::triggered, this, [this]() { showMainWindow(); });

    connect(quitAction, &QAction::triggered, this,
            [this]()
            {
                quitting = true;
                trayIcon->hide();
                close();
            });

    connect(trayIcon, &QSystemTrayIcon::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason)
            {
                if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
                {
                    showMainWindow();
                }
            });
}

void MainWindow::connectSignals()
{
    connect(ui->tintSlider, &QSlider::valueChanged, this, &MainWindow::onFilterControlsChanged);
    connect(ui->intensitySlider, &QSlider::valueChanged, this, &MainWindow::onFilterControlsChanged);
    connect(ui->gammaSlider, &QSlider::valueChanged, this, &MainWindow::onFilterControlsChanged);
    connect(ui->filterToggle, &QCheckBox::toggled, this, &MainWindow::onFilterControlsChanged);

    connect(ui->monitorSelector, &QComboBox::currentIndexChanged, this, &MainWindow::onMonitorChanged);
    connect(ui->iniSelector, &QComboBox::currentIndexChanged, this, &MainWindow::onProfileChanged);
    connect(ui->globalToggle, &QCheckBox::toggled, this, &MainWindow::onSyncMonitorsChanged);
    connect(ui->exitToTaskbarToggle, &QCheckBox::toggled, this, &MainWindow::onTraySettingChanged);
    connect(ui->runAtStartupToggle, &QCheckBox::toggled, this, &MainWindow::onRunAtStartupChanged);

    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::resetFilters);
    connect(ui->newIniButton, &QPushButton::clicked, this, &MainWindow::createProfile);
    connect(ui->duplicateIniButton, &QPushButton::clicked, this, &MainWindow::duplicateProfile);
    connect(ui->deleteIniButton, &QPushButton::clicked, this, &MainWindow::deleteProfile);
    connect(ui->LoadIniButton, &QPushButton::clicked, this, &MainWindow::importProfile);
    connect(ui->folderButton, &QPushButton::clicked, this, &MainWindow::openProfilesFolder);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::saveAllSettings);
    connect(ui->toggleSetButton, &QPushButton::clicked, this, &MainWindow::setToggleFilterHotkeyButtonPressed);
    connect(ui->peekSetButton, &QPushButton::clicked, this, &MainWindow::setPeekHotkeyButtonPressed);
}

void MainWindow::refreshMonitorSelector()
{
    QSignalBlocker blocker(ui->monitorSelector);
    ui->monitorSelector->clear();

    for (const Monitor &monitor : monitorManager.getMonitorVector())
    {
        std::wstring displayName = monitor.getDeviceName();

        if (displayName.size() > 4)
        {
            displayName = displayName.substr(4);
        }

        ui->monitorSelector->addItem(QString::fromStdWString(displayName));
    }

    if (ui->monitorSelector->count() == 0)
    {
        currentMonitorIndex = -1;
        ui->monitorSelector->setEnabled(false);
        return;
    }

    currentMonitorIndex = 0;
    ui->monitorSelector->setCurrentIndex(currentMonitorIndex);
}

void MainWindow::refreshProfileSelector()
{
    QSignalBlocker blocker(ui->iniSelector);
    ui->iniSelector->clear();

    for (const std::string &profileName : iniManager.profiles())
    {
        ui->iniSelector->addItem(QString::fromStdString(profileName));
    }

    const QString currentProfile = QString::fromStdString(iniManager.globalSettings().currentProfile);
    const int profileIndex = ui->iniSelector->findText(currentProfile);

    if (profileIndex >= 0)
    {
        ui->iniSelector->setCurrentIndex(profileIndex);
    }
    else if (ui->iniSelector->count() > 0)
    {
        ui->iniSelector->setCurrentIndex(0);
    }
}

void MainWindow::refreshProfilesAndReload()
{
    refreshProfileSelector();
    reloadCurrentProfile();
}

void MainWindow::reloadCurrentProfile()
{
    const std::string &profileName = iniManager.globalSettings().currentProfile;

    if (!profileName.empty())
    {
        iniManager.loadProfile(profileName);
    }

    loadMonitorIntoControls();
    applyCurrentFilters();
}

void MainWindow::loadMonitorIntoControls()
{
    if (!hasValidMonitor())
    {
        return;
    }

    QSignalBlocker tintBlocker(ui->tintSlider);
    QSignalBlocker intensityBlocker(ui->intensitySlider);
    QSignalBlocker gammaBlocker(ui->gammaSlider);
    QSignalBlocker filterBlocker(ui->filterToggle);

    const FilterSettings &settings = monitorManager.getMonitor(currentMonitorIndex).getFilterSettings();

    ui->tintSlider->setValue(static_cast<int>(settings.tint));
    ui->intensitySlider->setValue(static_cast<int>(settings.intensity * 100.0f));
    ui->gammaSlider->setValue(static_cast<int>(settings.gamma * 100.0f));
    ui->filterToggle->setChecked(settings.enabled);
}

void MainWindow::saveControlsToMonitor()
{
    if (!hasValidMonitor())
    {
        return;
    }

    const FilterSettings settings{static_cast<float>(ui->tintSlider->value()), static_cast<float>(ui->intensitySlider->value()) / 100.0f,
                                  static_cast<float>(ui->gammaSlider->value()) / 100.0f, ui->filterToggle->isChecked()};

    if (ui->globalToggle->isChecked())
    {
        for (Monitor &monitor : monitorManager.getMonitorVector())
        {
            monitor.getFilterSettings() = settings;
        }

        return;
    }

    monitorManager.getMonitor(currentMonitorIndex).getFilterSettings() = settings;
}

void MainWindow::loadGlobalControls()
{
    const GlobalSettings &settings = iniManager.globalSettings();

    QSignalBlocker trayBlocker(ui->exitToTaskbarToggle);
    QSignalBlocker startupBlocker(ui->runAtStartupToggle);

    ui->exitToTaskbarToggle->setChecked(settings.exitToTaskbar);
    ui->runAtStartupToggle->setChecked(settings.runAtStartup);
    ui->toggleText->setText(hotkeyText(settings.toggleFilterHotkey));
    ui->peekText->setText(hotkeyText(settings.peekHotkey));
}

void MainWindow::saveGlobalControls()
{
    GlobalSettings &settings = iniManager.globalSettings();
    settings.exitToTaskbar = ui->exitToTaskbarToggle->isChecked();
    settings.runAtStartup = ui->runAtStartupToggle->isChecked();
    settings.currentProfile = ui->iniSelector->currentText().toStdString();
}

void MainWindow::saveCurrentProfile()
{
    if (!iniManager.saveProfile(iniManager.globalSettings().currentProfile))
    {
        std::cerr << "MainWindow::saveCurrentProfile: Could not save the current profile.\n";
    }
}

void MainWindow::applyCurrentFilters()
{
    if (!hasValidMonitor())
    {
        return;
    }

    if (ui->globalToggle->isChecked())
    {
        ui->resetButton->setText("Reset All");
        ui->monitorSelector->setEnabled(false);

        for (Monitor &monitor : monitorManager.getMonitorVector())
        {
            if (monitor.getFilterSettings().enabled)
            {
                if (!monitor.applyFilter())
                {
                    std::cerr << "MainWindow::applyCurrentFilters: Could not apply a monitor filter.\n";
                }
            }
            else
            {
                if (!monitor.restoreGammaRamp())
                {
                    std::cerr << "MainWindow::applyCurrentFilters: Could not restore a monitor gamma ramp.\n";
                }
            }
        }

        return;
    }

    ui->resetButton->setText("Reset " + QString::number(currentMonitorIndex + 1));
    ui->monitorSelector->setEnabled(true);

    Monitor &monitor = monitorManager.getMonitor(currentMonitorIndex);

    if (monitor.getFilterSettings().enabled)
    {
        if (!monitor.applyFilter())
        {
            std::cerr << "MainWindow::applyCurrentFilters: Could not apply the filter.\n";
        }
    }
    else
    {
        if (!monitor.restoreGammaRamp())
        {
            std::cerr << "MainWindow::applyCurrentFilters: Could not restore the gamma ramp.\n";
        }
    }
}

void MainWindow::showMainWindow()
{
    showNormal();
    raise();
    activateWindow();
}

bool MainWindow::hasValidMonitor() const
{
    return currentMonitorIndex >= 0 && currentMonitorIndex < static_cast<int>(monitorManager.getMonitorVector().size());
}

void MainWindow::onFilterControlsChanged()
{
    saveControlsToMonitor();
    applyCurrentFilters();

    saveCurrentProfile();
}

void MainWindow::onMonitorChanged(int index)
{
    if (index < 0 || index >= static_cast<int>(monitorManager.getMonitorVector().size()))
    {
        return;
    }

    currentMonitorIndex = index;
    loadMonitorIntoControls();
    applyCurrentFilters();
}

void MainWindow::onSyncMonitorsChanged()
{
    if (!hasValidMonitor())
    {
        return;
    }

    if (ui->globalToggle->isChecked())
    {
        const FilterSettings settings = monitorManager.getMonitor(currentMonitorIndex).getFilterSettings();

        for (Monitor &monitor : monitorManager.getMonitorVector())
        {
            monitor.getFilterSettings() = settings;
        }
    }

    loadMonitorIntoControls();
    applyCurrentFilters();

    saveCurrentProfile();
}

void MainWindow::onProfileChanged()
{
    if (ui->iniSelector->currentIndex() < 0)
    {
        return;
    }

    iniManager.globalSettings().currentProfile = ui->iniSelector->currentText().toStdString();

    if (!iniManager.saveGlobalSettings())
    {
        QMessageBox::warning(this, "Color Filters", "The selected profile could not be saved.");
    }

    reloadCurrentProfile();
}

void MainWindow::onTraySettingChanged()
{
    saveGlobalControls();
    trayIcon->setVisible(iniManager.globalSettings().exitToTaskbar);
    if (!iniManager.saveGlobalSettings())
    {
        QMessageBox::warning(this, "Color Filters", "The tray setting could not be saved.");
    }
}

void MainWindow::onRunAtStartupChanged()
{
    QSettings startupSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    const QString appName = QCoreApplication::applicationName();
    const QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());

    if (ui->runAtStartupToggle->isChecked())
    {
        startupSettings.setValue(appName, QString("\"%1\"").arg(appPath));
    }
    else
    {
        startupSettings.remove(appName);
    }

    startupSettings.sync();

    if (startupSettings.status() != QSettings::NoError)
    {
        QSignalBlocker blocker(ui->runAtStartupToggle);
        ui->runAtStartupToggle->setChecked(!ui->runAtStartupToggle->isChecked());
        QMessageBox::warning(this, "Color Filters", "Windows startup settings could not be updated.");
        return;
    }

    saveGlobalControls();
    if (!iniManager.saveGlobalSettings())
    {
        QMessageBox::warning(this, "Color Filters", "The startup setting could not be saved.");
    }
}

void MainWindow::resetFilters()
{
    if (!hasValidMonitor())
    {
        return;
    }

    FilterSettings settings;
    settings.enabled = monitorManager.getMonitor(currentMonitorIndex).getFilterSettings().enabled;

    if (ui->globalToggle->isChecked())
    {
        for (Monitor &monitor : monitorManager.getMonitorVector())
        {
            monitor.getFilterSettings() = settings;
        }
    }
    else
    {
        monitorManager.getMonitor(currentMonitorIndex).getFilterSettings() = settings;
    }

    loadMonitorIntoControls();
    applyCurrentFilters();

    saveCurrentProfile();
}

void MainWindow::createProfile()
{
    bool accepted = false;
    const QString name = QInputDialog::getText(this, "New Profile", "Profile name:", QLineEdit::Normal, "", &accepted);

    if (!accepted)
    {
        return;
    }

    if (!iniManager.createProfile(name.toStdString()))
    {
        QMessageBox::warning(this, "Color Filters", "The profile could not be created. Check the name and try again.");
        return;
    }

    refreshProfilesAndReload();
}

void MainWindow::duplicateProfile()
{
    if (ui->iniSelector->currentIndex() < 0)
    {
        return;
    }

    bool accepted = false;
    const QString newName = QInputDialog::getText(this, "Duplicate Profile", "Profile name:", QLineEdit::Normal, "", &accepted);

    if (!accepted)
    {
        return;
    }

    if (!iniManager.duplicateProfile(ui->iniSelector->currentText().toStdString(), newName.toStdString()))
    {
        QMessageBox::warning(this, "Color Filters", "The profile could not be duplicated.");
        return;
    }

    refreshProfilesAndReload();
}

void MainWindow::deleteProfile()
{
    if (ui->iniSelector->currentIndex() < 0)
    {
        return;
    }

    if (!iniManager.deleteProfile(ui->iniSelector->currentText().toStdString()))
    {
        QMessageBox::warning(this, "Color Filters", "The profile could not be deleted.");
        return;
    }

    refreshProfilesAndReload();
}

void MainWindow::importProfile()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "Import INI Profile", QString(), "INI Files (*.ini)");

    if (fileName.isEmpty())
    {
        return;
    }

    if (!iniManager.importProfile(std::filesystem::path(fileName.toStdWString())))
    {
        QMessageBox::warning(this, "Color Filters", "The selected profile is invalid or could not be imported.");
        return;
    }

    refreshProfilesAndReload();
}

void MainWindow::openProfilesFolder()
{
    const HINSTANCE result = ShellExecuteW(nullptr, L"open", iniManager.profilesDirectory().c_str(), nullptr, nullptr, SW_SHOWNORMAL);

    if (reinterpret_cast<INT_PTR>(result) <= 32)
    {
        QMessageBox::warning(this, "Color Filters", "The profiles folder could not be opened.");
    }
}

void MainWindow::saveAllSettings()
{
    saveControlsToMonitor();
    saveGlobalControls();

    saveCurrentProfile();
    if (!iniManager.saveGlobalSettings())
    {
        std::cerr << "MainWindow::saveAllSettings: Could not save global settings.\n";
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (ui->exitToTaskbarToggle->isChecked() && QSystemTrayIcon::isSystemTrayAvailable() && !quitting)
    {
        hide();
        event->ignore();

        trayIcon->showMessage("Color Filters", "The app is still running in the system tray.");
        return;
    }

    event->accept();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (globalKeyboardInputRegistered)
    {
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (event->isAutoRepeat())
    {
        event->ignore();
        return;
    }

    processKeyPress(event->key());
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (globalKeyboardInputRegistered)
    {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    if (event->isAutoRepeat())
    {
        event->ignore();
        return;
    }

    processKeyRelease(event->key());
}

void MainWindow::processKeyPress(int key)
{
    if (capturingHotkey)
    {
        if (!setHotkeyPressedKeys.contains(key))
        {
            setHotkeyPressedKeys.append(key);
        }

        return;
    }

    if (!pressedKeys.contains(key))
    {
        pressedKeys.append(key);
    }

    const bool peekMatched = sameKeys(pressedKeys, iniManager.globalSettings().peekHotkey);

    if (peekMatched && !peekHotkeyActive)
    {
        startPeek();
    }

    const bool toggleMatched = sameKeys(pressedKeys, iniManager.globalSettings().toggleFilterHotkey);

    if (toggleMatched && !toggleHotkeyActive)
    {
        toggleHotkeyActive = true;
        ui->filterToggle->toggle();
    }
}

void MainWindow::processKeyRelease(int key)
{
    if (capturingHotkey)
    {
        capturingHotkey = false;
        releaseKeyboard();

        if (setHotkeyPressedKeys.empty())
        {
            loadGlobalControls();
            return;
        }

        if (capturingHotkeyTarget == HotkeyTarget::ToggleFilter)
        {
            iniManager.globalSettings().toggleFilterHotkey = setHotkeyPressedKeys;
        }
        else
        {
            iniManager.globalSettings().peekHotkey = setHotkeyPressedKeys;
        }
        saveAllSettings();
        loadGlobalControls();
        return;
    }

    const bool releasedPeekKey = iniManager.globalSettings().peekHotkey.contains(key);

    if (pressedKeys.contains(key))
    {
        pressedKeys.removeAll(key);
    }

    if (peekHotkeyActive && releasedPeekKey)
    {
        stopPeek();
    }

    if (!sameKeys(pressedKeys, iniManager.globalSettings().toggleFilterHotkey))
    {
        toggleHotkeyActive = false;
    }
}

void MainWindow::registerGlobalKeyboardInput()
{
    RAWINPUTDEVICE keyboardDevice{};
    keyboardDevice.usUsagePage = 0x01;
    keyboardDevice.usUsage = 0x06;
    keyboardDevice.dwFlags = RIDEV_INPUTSINK;
    keyboardDevice.hwndTarget = reinterpret_cast<HWND>(winId());

    globalKeyboardInputRegistered = RegisterRawInputDevices(&keyboardDevice, 1, sizeof(keyboardDevice)) == TRUE;

    if (!globalKeyboardInputRegistered)
    {
        std::cerr << "MainWindow: Could not register global keyboard input.\n";
    }
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    MSG *nativeMessage = static_cast<MSG *>(message);

    if (globalKeyboardInputRegistered && nativeMessage->message == WM_INPUT)
    {
        UINT dataSize = 0;
        const UINT sizeResult = GetRawInputData(reinterpret_cast<HRAWINPUT>(nativeMessage->lParam), RID_INPUT, nullptr, &dataSize, sizeof(RAWINPUTHEADER));

        if (sizeResult == static_cast<UINT>(-1) || dataSize == 0)
        {
            return QMainWindow::nativeEvent(eventType, message, result);
        }

        QByteArray data;
        data.resize(static_cast<qsizetype>(dataSize));

        if (GetRawInputData(reinterpret_cast<HRAWINPUT>(nativeMessage->lParam), RID_INPUT, data.data(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
        {
            const RAWINPUT *rawInput = reinterpret_cast<const RAWINPUT *>(data.constData());

            if (rawInput->header.dwType == RIM_TYPEKEYBOARD)
            {
                const RAWKEYBOARD &keyboard = rawInput->data.keyboard;
                const int key = qtKeyFromVirtualKey(keyboard.VKey);

                if (key != Qt::Key_unknown && keyboard.VKey != 255)
                {
                    if ((keyboard.Flags & RI_KEY_BREAK) != 0)
                    {
                        processKeyRelease(key);
                    }
                    else
                    {
                        processKeyPress(key);
                    }
                }
            }
        }

        *result = 0;
        return false;
    }

    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::setPeekHotkeyButtonPressed()
{
    setHotkeyPressedKeys.clear();
    pressedKeys.clear();

    capturingHotkeyTarget = HotkeyTarget::Peek;
    capturingHotkey = true;

    grabKeyboard();

    ui->peekText->setText("Press hotkey... ");
}

void MainWindow::setToggleFilterHotkeyButtonPressed()
{
    setHotkeyPressedKeys.clear();
    pressedKeys.clear();

    capturingHotkeyTarget = HotkeyTarget::ToggleFilter;
    capturingHotkey = true;

    grabKeyboard();

    ui->toggleText->setText("Press hotkey... ");
}

bool MainWindow::sameKeys(const QVector<int> &pressed, const QVector<int> &hotkey) const
{
    if (pressed.size() != hotkey.size())
        return false;

    for (int key : hotkey)
    {
        if (!pressed.contains(key))
            return false;
    }

    return true;
}
void MainWindow::startPeek()
{
    if (peekHotkeyActive)
        return;

    peekHotkeyActive = true;
    filterStateBeforePeek = ui->filterToggle->isChecked();

    {
        QSignalBlocker blocker(ui->filterToggle);
        ui->filterToggle->setChecked(!filterStateBeforePeek);
    }

    saveControlsToMonitor();
    applyCurrentFilters();
}

void MainWindow::stopPeek()
{
    if (!peekHotkeyActive)
        return;

    peekHotkeyActive = false;

    {
        QSignalBlocker blocker(ui->filterToggle);
        ui->filterToggle->setChecked(filterStateBeforePeek);
    }

    saveControlsToMonitor();
    applyCurrentFilters();
}
