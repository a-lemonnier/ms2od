#pragma once

#include <QLabel>
#include <QObject>
#include <QWidget>

#include <QThread>
#include <QTimer>

#include <QHoverEvent>
#include <QtEvents>
#include <QResizeEvent>

#include "framechart.h"

#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <tuple>
#include <set>
#include <atomic>


class RemainingTimeLabel : public QLabel {
    Q_OBJECT

public:
    explicit RemainingTimeLabel(QWidget *parent=nullptr);

    virtual ~RemainingTimeLabel();

    enum Mode {
        Time,
        Speed,
    };

    /**
     * @brief addValue
     * @param x
     * @param idx The index of the thread given the value
     */
    void addValue(double x, int idx=0);

    void setThreadNumber(int n);

    void setAccomplished(int n);

    void setSupport(double min, double max);
    void setSampleSize(int N);

    void setMode(Mode mode);

    void start();
    void pause();
    void stop();
    void resume();

    void reset();

    bool eventFilter(QObject *watched, QEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    FrameChart* fc_;

    std::vector<std::vector<std::tuple<double, double>>> data_{};

    std::set<int> idxSet_{};
    int threadNumber_{};

    int maxValues_=200;

    double min_{}, max_{};
    double speed_{};
    double remainTime_{};

    std::chrono::time_point<std::chrono::high_resolution_clock> clockStart_{};

    std::atomic<Mode> mode_=Mode::Speed;
    Mode modeBackup_{};

    std::atomic<int> N_=50;
    std::atomic<int> NAccomplished_=0;

    std::vector<double> meanRemainTime_{};

    std::atomic<bool> stop_=false;
    std::atomic<bool> pause_=false;

    QTimer* timer_;

    int state_=0;

    void loop_();

    void displayMode_(const Mode &mode);

    std::pair<double, double> computeSpeedTime_(const std::vector<std::tuple<double, double>> &vec);

    /**
     * @brief Compute the means, clear the vector, and replace by the means
     * @param vec
     */
    void trimData_(std::vector<std::tuple<double, double>>& vec);

};

