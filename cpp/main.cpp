#include "../h/mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QFile file(":form//style.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        a.setStyleSheet(stream.readAll());
        file.close();
    }

    w.show();
    return QApplication::exec();
}