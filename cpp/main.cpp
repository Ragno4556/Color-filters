#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QPalette>
#include <QTextStream>

#include <cstdlib>
#include <exception>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication application(argc, argv);
    QApplication::setApplicationName("ColorFilters");
    QApplication::setApplicationDisplayName("Color Filters");
    QApplication::setStyle("Fusion");

    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#1b1b1b"));
    palette.setColor(QPalette::WindowText, QColor("#f0f0f0"));
    palette.setColor(QPalette::Base, QColor("#202020"));
    palette.setColor(QPalette::AlternateBase, QColor("#292929"));
    palette.setColor(QPalette::ToolTipBase, QColor("#292929"));
    palette.setColor(QPalette::ToolTipText, QColor("#f0f0f0"));
    palette.setColor(QPalette::Text, QColor("#f0f0f0"));
    palette.setColor(QPalette::Button, QColor("#303030"));
    palette.setColor(QPalette::ButtonText, QColor("#f0f0f0"));
    palette.setColor(QPalette::BrightText, QColor("#ffffff"));
    palette.setColor(QPalette::Link, QColor("#cf38ee"));
    palette.setColor(QPalette::Highlight, QColor("#b000d4"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    palette.setColor(QPalette::PlaceholderText, QColor("#888888"));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#777777"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#777777"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#777777"));
    application.setPalette(palette);

    try
    {
        QFile styleFile(":/form/style.qss");
        if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&styleFile);
            application.setStyleSheet(stream.readAll());
        }

        MainWindow window;
        window.show();
        return application.exec();
    }
    catch (const std::exception &error)
    {
        QMessageBox::critical(nullptr, "Color Filters", QString("The application encountered an unexpected error:\n%1").arg(error.what()));
    }
    catch (...)
    {
        QMessageBox::critical(nullptr, "Color Filters", "The application encountered an unknown error.");
    }

    return EXIT_FAILURE;
}
