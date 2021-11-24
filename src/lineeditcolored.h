#pragma once

#include <QObject>
#include <QWidget>

#include <QLineEdit>
#include <QTimer>
#include <QFont>
#include <QColor>
#include <QPalette>

#include <QHoverEvent>
#include <QtEvents>
#include <QKeyEvent>

#include <QPainter>
#include <QPaintEvent>

#include <QGraphicsBlurEffect>

#include <QApplication>

#include "utils.h"


QT_BEGIN_NAMESPACE
namespace Ui { class LineEditColored; }
QT_END_NAMESPACE




class LineEditColored : public QLineEdit {
    Q_OBJECT
public:
    LineEditColored(QWidget* parent=nullptr);
    virtual ~LineEditColored();

    void setColor(std::pair<QColor, QColor> cc, int state);
    void setState(int state);

    void resetPalette();

    void setBlured(bool b);

    bool eventFilter ( QObject* obj,
                       QEvent* event );

private:
    QWidget* parent_{};

    Utils::TriState state_{};

    std::pair<QColor, QColor> colors_[3];

    QPalette palette_{};
    QPalette paletteA_{};
    QPalette paletteB_{};

    QColor color_{};
    QColor colorA_{Qt::darkBlue};

    QGraphicsBlurEffect* bEffect_;

    const std::pair<QColor, QColor> DefCol0={QColor(0xc5, 0, 0), QColor(0xd0, 0, 0)};
    const std::pair<QColor, QColor> DefCol1={QColor(0, 0xc0, 0), QColor(0, 0xd0, 0)};
    const std::pair<QColor, QColor> DefCol2={QColor(0, 0, 0xc7), QColor(0, 0, 0xd0)};

    bool firstLoop_=true;

    void updatePalette_();

};

