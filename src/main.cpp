#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QTranslator translator;
    if (translator.load(":/i18n/fr_FR")) app.installTranslator(&translator);

    MainWindow w(&translator);
    w.show();

    return app.exec();
}
