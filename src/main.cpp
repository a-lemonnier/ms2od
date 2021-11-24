#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <stdlib.h>

#include "utils.h"

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    QTranslator translator;

    const QStringList uiLanguages = QLocale::system().uiLanguages();

    for (const QString &locale : uiLanguages) {
        const QString baseName = "ms2od_" + QLocale(locale).name();
        if (translator.load(":/fr_FR" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    MainWindow w(&translator);
    w.show();
    return app.exec();
}
