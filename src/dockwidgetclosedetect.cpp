#include "dockwidgetclosedetect.h"


DockWidgetCloseDetect::DockWidgetCloseDetect(QWidget* parent):
    QDockWidget(parent) {
}

void DockWidgetCloseDetect::closeEvent(QCloseEvent* event) {
    emit closed();
    QWidget::closeEvent(event);
}

