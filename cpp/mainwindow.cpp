#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QToolButton>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QPushButton>
#include <QInputDialog>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), im(mm)
{
    ui->setupUi(this);

    // Disables maximizing
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    // Initialize stuff MainWindow owns
    mm.initialize();
    im.initialize();

    // Check if the monitor vector was initialized correctly before filling the combo box
    if (mm.getMonitorVector().empty())
    {
        currentMonitorIndex = -1;
        ui->monitorSelector->setEnabled(false);
    }

    else
    {
        currentMonitorIndex = 0;

        // Fill monitorSelector
        for (const Monitor &monitor : mm.getMonitorVector())
        {
            std::wstring deviceName = monitor.getDeviceName();
            if (deviceName.size() > 4)
            {
                deviceName = deviceName.substr(4);
            }
            ui->monitorSelector->addItem(QString::fromStdWString(deviceName));
        }
        ui->monitorSelector->setCurrentIndex(currentMonitorIndex);
    }

    // fill iniSelector
    updateINIS();

    // Set everything else: sliders, toggles, color preview
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

    // SIGNALS
    // Buttons
    connect(ui->resetButton, &QPushButton::clicked, this, &MainWindow::resetButtonClicked);
    connect(ui->newIniButton, &QPushButton::clicked, this, &MainWindow::newProfile);
    connect(ui->duplicateIniButton, &QPushButton::clicked, this, &MainWindow::duplicateINIS);
    connect(ui->deleteIniButton, &QPushButton::clicked, this, &MainWindow::deleteINI);

    // Sliders
    connect(ui->tintSlider, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);
    connect(ui->intensitySlider, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);
    connect(ui->gammaSlider, &QSlider::valueChanged, this, &MainWindow::onSliderMoved);

    // Checkboxes
    connect(ui->globalToggle, &QCheckBox::toggled, this, &MainWindow::onGlobalToggle);
    connect(ui->filterToggle, &QCheckBox::toggled, this, &MainWindow::onFilterToggle);

    // ComboBoxes
    connect(ui->monitorSelector, &QComboBox::currentIndexChanged, this, &MainWindow::onMonitorChanged);
}

MainWindow::~MainWindow()
{
    // Restore original monitor color before application closes
    mm.restoreAllGammaRamps();
    delete ui;
}

void MainWindow::loadActiveSettingsIntoSliders()
{
    // Check if index is greater than 0
    if (!hasValidMonitor())
    {
        return;
    }

    QSignalBlocker tintBlocker(ui->tintSlider);
    QSignalBlocker intensityBlocker(ui->intensitySlider);
    QSignalBlocker gammaBlocker(ui->gammaSlider);
    QSignalBlocker filterBlocker(ui->filterToggle);

    const FilterSettings &settings = mm.getMonitor(currentMonitorIndex).getFilterSettings();

    ui->tintSlider->setValue(
        static_cast<int>(settings.tint));

    ui->intensitySlider->setValue(
        static_cast<int>(settings.intensity * 100.0f));

    ui->gammaSlider->setValue(
        static_cast<int>(settings.gamma * 100.0f));
    ui->filterToggle->setChecked(settings.enabled);
}

void MainWindow::saveActiveSettingsFromSliders()
{
    // Check if index is greater than 0
    if (!hasValidMonitor())
    {
        return;
    }

    FilterSettings settings{
        static_cast<float>(ui->tintSlider->value()),
        static_cast<float>(ui->intensitySlider->value()) / 100.0f,
        static_cast<float>(ui->gammaSlider->value()) / 100.0f,
        ui->filterToggle->isChecked()};

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

void MainWindow::applyActiveSettings()
{

    // Check if index is greater than 0
    if (!hasValidMonitor())
    {
        return;
    }

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
        if (mm.getMonitor(currentMonitorIndex).getFilterSettings().enabled)
        {
            mm.getMonitor(currentMonitorIndex).applyFilter();
        }
        else
        {
            mm.getMonitor(currentMonitorIndex).restoreGammaRamp();
        }
    }
}

void MainWindow::onMonitorChanged(int index)
{

    // Check if index is greater than 0
    if (index < 0 ||
        index >= static_cast<int>(mm.getMonitorVector().size()))
    {
        return;
    }
    currentMonitorIndex = index;
    loadActiveSettingsIntoSliders();
    applyActiveSettings();
}

void MainWindow::onSliderMoved()
{
    updateColorPreview();
    saveActiveSettingsFromSliders();
    applyActiveSettings();
}

void MainWindow::resetButtonClicked()
{
    // Check if index is greater than 0
    if (!hasValidMonitor())
    {
        return;
    }

    FilterSettings settings{};

    if (ui->globalToggle->isChecked())
    {
        settings.enabled =
            mm.getMonitor(currentMonitorIndex)
                .getFilterSettings()
                .enabled;

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

void MainWindow::onGlobalToggle()
{
    // Check if index is greater than 0
    if (!hasValidMonitor())
    {
        return;
    }

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

void MainWindow::onFilterToggle()
{
    saveActiveSettingsFromSliders();
    applyActiveSettings();
}

void MainWindow::updateColorPreview()
{
    int hue = ui->tintSlider->value();

    // Qt uses hue values from 0 to 359.
    // Hue 360 is equivalent to hue 0.
    if (hue == 360)
    {
        hue = 0;
    }
    previewColor.setHsv(hue, 255, 255);

    ui->colorPreviewFrame->setStyleSheet(
        QString("background-color: %1;")
            .arg(previewColor.name()));
}

void MainWindow::newProfile()
{
    bool ok;
    QString fileName = QInputDialog::getText(this, "New Profile", "Profile name:", QLineEdit::Normal, "", &ok);

    if (!ok)
    {
        return;
    }

    im.createNewIni(fileName.toStdString());
    updateINIS();
}

void MainWindow::updateINIS()
{
    ui->iniSelector->clear();
    for (IniData &ini : im.getLoadedInis())
    {
        std::string &filename = ini.iniFilename;
        ui->iniSelector->addItem(QString::fromStdString(filename));
    }
}

void MainWindow::duplicateINIS()
{
    if (ui->iniSelector->currentIndex() < 0)
    {
        return;
    }

    std::string sourceName = (ui->iniSelector->currentText()).toStdString();
    bool ok;
    QString fileName = QInputDialog::getText(this, "New Profile", "Profile name:", QLineEdit::Normal, "", &ok);

    if (!ok)
    {
        return;
    }

    im.duplicateIni(sourceName, fileName.toStdString());
    updateINIS();
}
void MainWindow::deleteINI()
{
    if (ui->iniSelector->currentIndex() < 0)
    {
        return;
    }

    if (!ui->iniSelector->currentText().toStdString().empty())
    {
        im.deleteIni(ui->iniSelector->currentText().toStdString());
        ui->iniSelector->removeItem(ui->iniSelector->currentIndex());
    }
}

bool MainWindow::hasValidMonitor() const
{
    return currentMonitorIndex >= 0 && currentMonitorIndex < static_cast<int>(mm.getMonitorVector().size());
}
