#pragma once

#include <QMainWindow>

#include "MonitorManager.h"
#include "ColorController.h"
#include "INIManager.h"

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
    MonitorManager &getMonitorManager() { return mm; }
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    ColorController cc;
    QColor previewColor;
    int currentMonitorIndex;
    MonitorManager mm;
    IniManager im;

private slots:

    // Sliders
    void onSliderMoved();

    // Buttons
    void resetButtonClicked();

    // Toggles
    void onGlobalToggle();
    void onFilterToggle();

    // Ini
    void loadActiveSettingsIntoSliders();
    void saveActiveSettingsFromSliders();
    void newProfile();
    void updateINIS();
    void duplicateINIS();
    void deleteINI();

    // Monitors
    void onMonitorChanged(int index);
    void applyActiveSettings();

    // Colors
    void updateColorPreview();
};
