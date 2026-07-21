#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QPushButton>
#include <QInputDialog>

#include <iostream>

// Initializes the main window and connects the UI
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), im(mm)
{

    ui->setupUi(this);

    // Disable maximizing
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    // Initialize managers
    mm.initialize();
    im.initialize();

    // Check if any monitors were found
    if (mm.getMonitorVector().empty())
    {
        currentMonitorIndex = -1;
        ui->monitorSelector->setEnabled(false);
    }
    else
    {
        currentMonitorIndex = 0;

        // Fill monitor selector
        for (const Monitor &monitor : mm.getMonitorVector())
        {
            std::wstring deviceName = monitor.getDeviceName();

            // Remove DISPLAY prefix
            if (deviceName.size() > 4)
            {
                deviceName = deviceName.substr(4);
            }

            ui->monitorSelector->addItem(QString::fromStdWString(deviceName));
        }

        ui->monitorSelector->setCurrentIndex(currentMonitorIndex);
    }

    // Fill INI selector
    updateINIS();

    // Load the active monitor settings
    if (currentMonitorIndex >= 0)
    {
        loadActiveSettingsIntoSliders();
        applyActiveSettings();
    }

    updateColorPreview();

    // Put reset button in the top right of the tab selector
    QWidget *container = new QWidget(ui->tabSelector);
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(ui->resetButton);
    container->setLayout(layout);
    container->adjustSize();
    ui->tabSelector->setCornerWidget(container, Qt::TopRightCorner);

    // Button signals
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::resetButtonClicked);
    connect(ui->newIniButton, &QPushButton::clicked, this, &MainWindow::newProfile);
    connect(ui->duplicateIniButton, &QPushButton::clicked, this, &MainWindow::duplicateINIS);
    connect(ui->deleteIniButton, &QPushButton::clicked, this, &MainWindow::deleteINI);

    // Slider signals
    connect(ui->tintSlider, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);
    connect(ui->intensitySlider, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);
    connect(ui->gammaSlider, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);

    // Checkbox signals
    connect(ui->globalToggle, &QCheckBox::toggled, this, &MainWindow::onGlobalToggle);
    connect(ui->filterToggle, &QCheckBox::toggled, this, &MainWindow::onFilterToggle);

    // Combo box signals
    connect(ui->monitorSelector, &QComboBox::currentIndexChanged, this, &MainWindow::onMonitorChanged);
}

// Restores the original monitor colors before closing
MainWindow::~MainWindow()
{
    mm.restoreAllGammaRamps();
    delete ui;
}

// Loads the active monitor settings into the UI
void MainWindow::loadActiveSettingsIntoSliders()
{

    // Check if the current monitor exists
    if (!hasValidMonitor())
    {
        return;
    }

    // Prevent signals while changing UI values
    QSignalBlocker tintBlocker(ui->tintSlider);
    QSignalBlocker intensityBlocker(ui->intensitySlider);
    QSignalBlocker gammaBlocker(ui->gammaSlider);
    QSignalBlocker filterBlocker(ui->filterToggle);

    const FilterSettings &settings = mm.getMonitor(currentMonitorIndex).getFilterSettings();

    ui->tintSlider->setValue(static_cast<int>(settings.tint));
    ui->intensitySlider->setValue(static_cast<int>(settings.intensity * 100.0f));
    ui->gammaSlider->setValue(static_cast<int>(settings.gamma * 100.0f));
    ui->filterToggle->setChecked(settings.enabled);
}

// Saves the UI values to the active monitor settings
void MainWindow::saveActiveSettingsFromSliders()
{

    // Check if the current monitor exists
    if (!hasValidMonitor())
    {
        return;
    }

    FilterSettings settings{
        static_cast<float>(ui->tintSlider->value()),
        static_cast<float>(ui->intensitySlider->value()) / 100.0f,
        static_cast<float>(ui->gammaSlider->value()) / 100.0f,
        ui->filterToggle->isChecked()};

    // Apply settings to every monitor
    if (ui->globalToggle->isChecked())
    {
        for (Monitor &monitor : mm.getMonitorVector())
        {
            monitor.setFilterSettings(settings);
        }
    }
    else
    {
        mm.getMonitor(currentMonitorIndex).setFilterSettings(settings);
    }
}

// Applies the saved filter settings to the monitors
void MainWindow::applyActiveSettings()
{

    // Check if the current monitor exists
    if (!hasValidMonitor())
    {
        return;
    }

    // Apply settings to every monitor
    if (ui->globalToggle->isChecked())
    {
        ui->resetButton->setText("Reset All");
        ui->monitorSelector->setEnabled(false);

        for (Monitor &monitor : mm.getMonitorVector())
        {
            if (monitor.getFilterSettings().enabled)
            {
                monitor.applyFilter();
            }
            else
            {
                monitor.restoreGammaRamp();
            }
        }
    }
    else
    {
        ui->resetButton->setText("Reset " + QString::number(currentMonitorIndex + 1));
        ui->monitorSelector->setEnabled(true);

        Monitor &monitor = mm.getMonitor(currentMonitorIndex);

        if (monitor.getFilterSettings().enabled)
        {
            monitor.applyFilter();
        }
        else
        {
            monitor.restoreGammaRamp();
        }
    }
}

// Loads the settings for the newly selected monitor
void MainWindow::onMonitorChanged(int index)
{

    // Check if the selected monitor exists
    if (index < 0 || index >= static_cast<int>(mm.getMonitorVector().size()))
    {
        return;
    }

    currentMonitorIndex = index;
    loadActiveSettingsIntoSliders();
    applyActiveSettings();
}

// Saves and applies settings when a slider moves
void MainWindow::onSliderMoved()
{
    updateColorPreview();
    saveActiveSettingsFromSliders();
    applyActiveSettings();
}

// Resets the active filter settings to their defaults
void MainWindow::resetButtonClicked()
{

    // Check if the current monitor exists
    if (!hasValidMonitor())
    {
        return;
    }

    FilterSettings settings{};

    // Reset every monitor
    if (ui->globalToggle->isChecked())
    {
        settings.enabled = mm.getMonitor(currentMonitorIndex).getFilterSettings().enabled;

        for (Monitor &monitor : mm.getMonitorVector())
        {
            monitor.setFilterSettings(settings);
        }
    }
    else
    {
        Monitor &monitor = mm.getMonitor(currentMonitorIndex);
        settings.enabled = monitor.getFilterSettings().enabled;
        monitor.setFilterSettings(settings);
    }

    loadActiveSettingsIntoSliders();
    applyActiveSettings();
}

// Enables or disables global monitor settings
void MainWindow::onGlobalToggle()
{

    // Check if the current monitor exists
    if (!hasValidMonitor())
    {
        return;
    }

    // Copy the current settings to every monitor
    if (ui->globalToggle->isChecked())
    {
        FilterSettings settings = mm.getMonitor(currentMonitorIndex).getFilterSettings();

        for (Monitor &monitor : mm.getMonitorVector())
        {
            monitor.setFilterSettings(settings);
        }
    }

    loadActiveSettingsIntoSliders();
    applyActiveSettings();
}

// Enables or disables the current filter
void MainWindow::onFilterToggle()
{
    saveActiveSettingsFromSliders();
    applyActiveSettings();
}

// Updates the color preview using the tint slider
void MainWindow::updateColorPreview()
{
    int hue = ui->tintSlider->value();

    // Qt uses hue values from 0 to 359
    if (hue == 360)
    {
        hue = 0;
    }

    previewColor.setHsv(hue, 255, 255);
    ui->colorPreviewFrame->setStyleSheet(QString("background-color: %1;").arg(previewColor.name()));
}

// Creates a new INI profile
void MainWindow::newProfile()
{
    bool ok;
    QString fileName = QInputDialog::getText(this, "New Profile", "Profile name:", QLineEdit::Normal, "", &ok);

    // Return if the dialog was canceled
    if (!ok)
    {
        return;
    }

    im.createNewIni(fileName.toStdString());
    updateINIS();
}

// Updates the INI profile selector
void MainWindow::updateINIS()
{
    ui->iniSelector->clear();

    for (IniData &ini : im.getLoadedInis())
    {
        std::string &filename = ini.iniFilename;
        ui->iniSelector->addItem(QString::fromStdString(filename));
    }
}

// Creates a copy of the selected INI profile
void MainWindow::duplicateINIS()
{

    // Check if a profile is selected
    if (ui->iniSelector->currentIndex() < 0)
    {
        return;
    }

    std::string sourceName = ui->iniSelector->currentText().toStdString();
    bool ok;
    QString fileName = QInputDialog::getText(this, "New Profile", "Profile name:", QLineEdit::Normal, "", &ok);

    // Return if the dialog was canceled
    if (!ok)
    {
        return;
    }

    im.duplicateIni(sourceName, fileName.toStdString());
    updateINIS();
}

// Deletes the selected INI profile
void MainWindow::deleteINI()
{

    // Check if a profile is selected
    if (ui->iniSelector->currentIndex() < 0)
    {
        return;
    }

    std::string fileName = ui->iniSelector->currentText().toStdString();

    if (!fileName.empty())
    {
        im.deleteIni(fileName);
        ui->iniSelector->removeItem(ui->iniSelector->currentIndex());
    }
}

// Checks if the current monitor index is valid
bool MainWindow::hasValidMonitor() const
{
    return currentMonitorIndex >= 0 && currentMonitorIndex < static_cast<int>(mm.getMonitorVector().size());
}
