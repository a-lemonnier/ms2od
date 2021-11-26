#pragma once

#include <QFrame>
#include <QVBoxLayout>

#include <QEvent>

#include <QPoint>

#include <QtCharts>
#include <QSplineSeries>
#include <QValueAxis>
#include <QAbstractAxis>

#include <QDebug>

#include <iostream>
#include <algorithm>

namespace Ui {
class FrameChart;
}

class FrameChart : public QFrame {
    Q_OBJECT

public:
    explicit FrameChart(QWidget *parent = nullptr);
    ~FrameChart();

    void addPoint(double x, double y);
    void reset();
    void show();

protected:
    void changeEvent(QEvent *e);

private:
    QWidget* parent_{};
    Ui::FrameChart *ui;

    double ymin_=0, ymax_=0, xmax_=0;

    QValueAxis* axisX_;
    QValueAxis* axisY_;

    QPoint position_{};

    QVBoxLayout *centralLayout_{};

    QChart *chart_;
    QChartView *chartView_;
    QSplineSeries *series_;

    std::vector<std::pair<double, double>> data_;

    void plot_();

    void initChart_();
    void initSerie_();
    void initAxis_();
};

