#pragma once

#include <QMainWindow>
#include <QPoint>

#include "INIManager.h"
#include "MonitorManager.h"

class QCloseEvent;
class QEvent;
class QKeyEvent;
class QSystemTrayIcon;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

  private slots:
    void onFilterControlsChanged();
    void onMonitorChanged(int index);
    void onSyncMonitorsChanged();
    void onProfileChanged();
    void onTraySettingChanged();
    void onRunAtStartupChanged();

    void resetFilters();
    void createProfile();
    void duplicateProfile();
    void deleteProfile();
    void importProfile();
    void openProfilesFolder();
    void saveAllSettings();

    void setPeekHotkeyButtonPressed();
    void setToggleFilterHotkeyButtonPressed();

  private:
    Ui::MainWindow *ui;
    MonitorManager monitorManager;
    IniManager iniManager;
    QSystemTrayIcon *trayIcon = nullptr;
    int currentMonitorIndex = -1;
    bool quitting = false;
    QVector<int> pressedKeys;
    QVector<int> setHotkeyPressedKeys;
    bool capturingHotkey = false;
    enum class HotkeyTarget
    {
        ToggleFilter,
        Peek
    };

    HotkeyTarget capturingHotkeyTarget = HotkeyTarget::ToggleFilter;
    bool toggleHotkeyActive = false;
    bool peekHotkeyActive = false;
    bool filterStateBeforePeek = false;
    bool globalKeyboardInputRegistered = false;
    bool draggingWindow = false;
    bool initialized = false;
    QPoint windowDragOffset;

    void setupTrayIcon();
    void connectSignals();
    void refreshMonitorSelector();
    void refreshProfileSelector();
    void refreshProfilesAndReload();
    void reloadCurrentProfile();
    void loadMonitorIntoControls();
    void saveControlsToMonitor();
    void loadGlobalControls();
    void saveGlobalControls();
    void saveCurrentProfile();
    void applyCurrentFilters();
    void showMainWindow();
    bool hasValidMonitor() const;
    bool sameKeys(const QVector<int> &pressed, const QVector<int> &hotkey) const;
    void startPeek();
    void stopPeek();
    void processKeyPress(int key);
    void processKeyRelease(int key);
    void registerGlobalKeyboardInput();
};
