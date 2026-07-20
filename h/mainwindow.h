#pragma once

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "MonitorManager.h"
#include "ColorController.h"
#include "INIManager.h"

class otherClass;


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MonitorManager& getMonitorManager() {return mm;}
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
    void loadActiveSettingsIntoSliders();
    void saveActiveSettingsFromSliders();
    void applyActiveSettings();
    void onMonitorChanged(int index);
    void onSliderMoved();
    void resetButtonClicked();
    void onGlobalToggle();
    void onFilterToggle();
    void updateColorPreview();
    void newProfile();
    void updateINIS();
    void duplicateINIS();
    void deleteINI();





};



#endif // MAINWINDOW_H
