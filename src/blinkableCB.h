#pragma once

#include <QWidget>
#include <QComboBox>

#include <QTimer>
#include <QFont>

QT_BEGIN_NAMESPACE
namespace Ui { class BlinkableComboBox; }
QT_END_NAMESPACE

class BlinkableComboBox: public QComboBox {
    Q_OBJECT
public:

    explicit BlinkableComboBox(QWidget* parent=nullptr, int duration=300,  int periode=50);
    virtual ~BlinkableComboBox();

    void setDuration(int ms);
    void setPeriode(int ms);

    void start();
protected:
    int count_{5};
    int periode_{};
    int duration_{};

    QString styleSheet_{};
    QString styleSheetA_{};

    QFont font_{};

    QTimer* timer_{};

    void blink();

};
