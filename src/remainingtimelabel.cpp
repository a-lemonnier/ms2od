#include "remainingtimelabel.h"



RemainingTimeLabel::RemainingTimeLabel(QWidget* parent):
    QLabel(parent),
    fc_(new FrameChart(this)) {

    this->installEventFilter(this);

    this->setText("");

    this->setToolTip(tr("Remaining time."));

    this->timer_=new QTimer(this);

    connect(this->timer_, &QTimer::timeout, this, &RemainingTimeLabel::loop_);
}

RemainingTimeLabel::~RemainingTimeLabel() {
    qDebug() << "-\tRemainingTimeLabel::~RemainingTimeLabel() called";
}

void RemainingTimeLabel::addValue(double x, int idx) {
    auto t=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    if (idx>0 && idx<this->data_.size()+1) {
        std::pair<double, double> res={t,x};
        this->data_[idx-1].push_back(res);
    }
    else
        std::cerr << "RemainingTimeLabel::addValue(x, idx): " << x << " " << idx << " boundary error" << std::endl;
}

void RemainingTimeLabel::setThreadNumber(int n) {
    this->threadNumber_=n;
}

void RemainingTimeLabel::setAccomplished(int n) {
    this->NAccomplished_=n;
}

void RemainingTimeLabel::setSupport(double min, double max) {
    qDebug() << "RemainingTimeLabel::setSupport(" << min << ", " << max << ")";
    this->min_=min;
    this->max_=max;
}

void RemainingTimeLabel::setSampleSize(int N) {
    this->N_=N;
}

void RemainingTimeLabel::setMode(Mode mode) {
    this->mode_=mode;
}

void RemainingTimeLabel::start() {
    this->data_=std::vector<std::vector<std::tuple<double, double>>>(this->threadNumber_);
    if (!this->timer_->isActive()) this->timer_->start(1000);
    this->clockStart_=std::chrono::high_resolution_clock::now();
}

void RemainingTimeLabel::pause() {
    qDebug() << "- " <<  " " << "RemainingTimeLabel::pause()";

    this->pause_=true;
    int remaining = this->timer_->remainingTime();
    this->timer_->stop();
    this->timer_->setInterval(remaining);
}

void RemainingTimeLabel::stop() {
    qDebug() << "- " <<  " " << "RemainingTimeLabel::stop()";

    this->pause_=false;
    this->timer_->stop();
}

void RemainingTimeLabel::resume() {
    qDebug() << "- " <<  " " << "RemainingTimeLabel::resume()";

    this->pause_=true;
    this->timer_->start();
}

void RemainingTimeLabel::reset() {
    qDebug() << "- " <<  " " << "RemainingTimeLabel::reset()";

    this->stop();
    while(this->timer_->isActive()) QThread::msleep(250);

    this->fc_->reset();
    this->data_.clear();

    this->min_=0.;
    this->max_=0.;
    this->speed_=0.;
    this->rtime_=0.;

    this->mode_=Mode::Speed;

    this->N_=50;
    this->NAccomplished_=0;

    this->stop_=false;
    this->pause_=false;

    this->state_=0;
}


void RemainingTimeLabel::loop_() {
    this->speed_=0.0;
    this->rtime_=0.0;
    for(auto &vec: this->data_) {
        if (vec.size()>2) {
            auto [v,t]=this->computeSpeedTime(vec);
            this->speed_+=v;
            this->rtime_+=t;
        }
        if (vec.size()>this->maxValues_)  this->trimData_(vec);
    }

    this->speed_/=this->threadNumber_;
    this->rtime_/=this->threadNumber_;

    this->displayMode_(this->mode_);

    std::chrono::duration<double> duration=std::chrono::high_resolution_clock::now()-this->clockStart_;
    this->fc_->addPoint(duration.count(), this->speed_);
}


bool RemainingTimeLabel::eventFilter(QObject* watched, QEvent* event) {
    if (event->type()==QEvent::Enter) {
        this->modeBackup_=this->mode_;
        if (this->mode_==Mode::Speed) {
            this->displayMode_(Mode::Time);
            this->mode_=Mode::Time;
        }
        else if (this->mode_==Mode::Time) {
            this->displayMode_(Mode::Speed);
            this->mode_=Mode::Speed;
        }
        this->fc_->show();
    }
    else if (event->type()==QEvent::Leave) {
        this->mode_=this->modeBackup_;
        if (this->mode_==Mode::Time) {
            this->displayMode_(Mode::Time);
        }
        else if (this->mode_==Mode::Speed) {
            this->displayMode_(Mode::Speed);
        }

        this->fc_->close();
    }
    return QWidget::eventFilter(watched, event);
}

void RemainingTimeLabel::resizeEvent(QResizeEvent* event) {
    auto pos=this->mapToGlobal(QPoint(0, 0));

    fc_->move(QPoint(pos.x()+30, pos.y()+30));
    QWidget::resizeEvent(event);
}

void RemainingTimeLabel::displayMode_(const Mode &mode) {
    std::stringstream ss{};

    if (this->mode_==Mode::Speed) {
        this->setToolTip(tr("Speed."));
        if (!std::isnan(this->speed_) && !std::isinf(this->speed_) && this->speed_>0) {
            ss << std::fixed << std::setprecision(2) << this->speed_  <<" s<sup>-1</sup>";
            this->setText(QString::fromStdString(ss.str()));
        }
        else this->setText(QString::fromStdString("~ min"));
    }
    else if (this->mode_==Mode::Time) {
        this->setToolTip(tr("Remaining time."));

        if (!std::isnan(this->rtime_) && !std::isinf(this->rtime_) && this->rtime_>0) {

            if (this->rtime_<1.0)  ss << static_cast<int>(this->rtime_*60.0) << " s";
            if (this->rtime_>1.0 &&
                this->rtime_<60.0) ss << static_cast<int>(this->rtime_) << " min";
            if (this->rtime_>60.0) ss << static_cast<int>(this->rtime_/60.0) << " h";

            this->setText(QString::fromStdString(ss.str()));
        }
        else this->setText(QString::fromStdString("~ min"));
    }
    else {
        this->setToolTip(tr("No input data."));
        this->setText("-");
    }
}

std::pair<double, double>
RemainingTimeLabel::computeSpeedTime(const std::vector<std::tuple<double, double>>& vec) {
    auto t0=std::get<0>(vec[0]);

    double dt=0.0, dx=0.0;
    int dim=vec.size();
    for(int i=0; i<dim-1; i++) {
           dx+=std::get<1>(vec[i+1])-std::get<1>(vec[i]);
    }
    dt=(std::get<0>(vec[vec.size()-1])-t0)*1.0e-6;

    auto speed= dt!=0 ? dx/dt : 0.;
    auto time=(this->max_-this->min_-this->NAccomplished_)/speed/60.;

    return {speed, time};
}

void RemainingTimeLabel::trimData_(std::vector<std::tuple<double, double> >& vec) {
    qDebug() << "- " <<  " " << "RemainingTimeLabel::trimData()";

    double mean_0=0, mean_1=0;

    for(const auto&[val_0, val_1]: vec ) {
        mean_0+=val_0;
        mean_1+=val_1;
    }

    mean_0/=vec.size();
    mean_1/=vec.size();

    vec.clear();
    vec.push_back({mean_0, mean_1});
}
