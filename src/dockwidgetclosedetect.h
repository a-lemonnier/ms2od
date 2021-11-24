#pragma once

#include <QDockWidget>

class DockWidgetCloseDetect : public QDockWidget {
    Q_OBJECT

public:
    DockWidgetCloseDetect(QWidget *parent = nullptr);

signals:
    void closed();

protected:
    void closeEvent(QCloseEvent *event);

};

