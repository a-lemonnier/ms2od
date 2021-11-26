#include "framechart.h"
#include "ui_framechart.h"

FrameChart::FrameChart(QWidget *parent) :
    QFrame(parent),
    parent_(parent),
    ui(new Ui::FrameChart) {
    ui->setupUi(this);

    this->setWindowFlag(Qt::Dialog);
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setWindowFlag(Qt::WindowDoesNotAcceptFocus);

    this->centralLayout_=new QVBoxLayout();

    this->initChart_();
    this->initSerie_();
    this->initAxis_();

    this->setLayout(this->centralLayout_);
}

FrameChart::~FrameChart() {
    delete ui;
}

void FrameChart::addPoint(double x, double y) {

    if (!this->chart_->series().empty())
        this->chart_->removeSeries(this->series_);

    this->chart_->removeAxis(this->axisX_);
    this->chart_->removeAxis(this->axisY_);

    this->series_->append(x,y);

    this->chart_->addSeries(this->series_);

    this->chart_->addAxis(this->axisX_, Qt::AlignBottom);
    this->chart_->addAxis(this->axisY_, Qt::AlignLeft);
    this->series_->attachAxis(this->axisX_);
    this->series_->attachAxis(this->axisY_);
}

void FrameChart::reset() {
    if (this->centralLayout_) delete this->centralLayout_;

    this->centralLayout_=new QVBoxLayout();
    this->setObjectName("reset layout");

    // Var --------------------------------------
    this->ymin_=0;
    this->ymax_=0;
    this->xmax_=0;

    this->position_=QPoint{};

    this->data_.clear();

    this->series_->clear();
    this->series_->append(0,0);
    // ------------------------------------------

    this->initChart_();
    this->initSerie_();
    this->initAxis_();

    this->setLayout(this->centralLayout_);
}

void FrameChart::show() {
    this->plot_();
    QWidget::show();
}

void FrameChart::changeEvent(QEvent *e) {
    QFrame::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FrameChart::plot_() {
    this->chart_->show();
}

void FrameChart::initChart_() {
    // Chart ------------------------------------
    this->chart_=new QChart();

    this->chart_->setTheme(QChart::ChartThemeBlueCerulean);

    this->chart_->setTitle(tr("Speed"));

    this->chart_->legend()->hide();

    this->chartView_ = new QChartView(this->chart_);
    this->chartView_->setRenderHint(QPainter::Antialiasing);

    this->centralLayout_->removeWidget(this->chartView_);
    this->centralLayout_->addWidget(this->chartView_);
    // ------------------------------------------
}

void FrameChart::initSerie_() {
    // Series -----------------------------------
    this->series_ = new QSplineSeries();
    this->series_->setName(tr("Speed"));
    this->series_->append(0,0);
    // ------------------------------------------

    // Rescale ----------------------------------
    connect(this->series_, &QSplineSeries::pointAdded, this,  [&](int i){
        auto x = this->series_->at(i).x();
        auto y = this->series_->at(i).y();

        if (y<this->ymin_) this->ymin_=y;
        if (y>this->ymax_) this->ymax_=y;
        if (x>this->xmax_) this->xmax_=x;

        this->axisY_->setRange(this->ymin_, this->ymax_*1.2);
        this->axisX_->setRange(0, this->xmax_);

    });
    // ------------------------------------------
}

void FrameChart::initAxis_() {
    // Axis -------------------------------------
    this->axisX_=new QValueAxis;
    this->axisY_=new QValueAxis;

    this->axisY_->setTitleText("<span style='font-size:10px;'>"+tr("Speed")+"</span> <span style='font-size:10px;'>(s<sup>-1</sup>)</span>");
    this->axisX_->setTitleText("<span style='font-size:11px;'>"+tr("Duration")+"</span> <span style='font-size:10px;'>(s)</span>");

    this->chart_->addAxis(this->axisX_, Qt::AlignBottom);
    this->chart_->addAxis(this->axisY_, Qt::AlignLeft);
    this->series_->attachAxis(this->axisX_);
    this->series_->attachAxis(this->axisY_);
    // ------------------------------------------
}
