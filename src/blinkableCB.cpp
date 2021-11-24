#include "blinkableCB.h"


//  BlinkableComboBox
// ------------------------


BlinkableComboBox::BlinkableComboBox (QWidget* parent,  int duration, int periode)  : QComboBox(parent) ,
periode_(periode),
duration_(duration) {
    this->count_= periode>0 ? duration/periode : 3;

    this->styleSheet_=this->styleSheet();
    this->styleSheetA_="color: blue;";

    this->font_=this->font();

    this->timer_=new QTimer();

    connect(this->timer_, &QTimer::timeout, this, &BlinkableComboBox::blink);
}

BlinkableComboBox::~BlinkableComboBox() {
    if (this->timer_->isActive()) this->timer_->stop();
}


void BlinkableComboBox::start() { this->timer_->start(this->periode_); }

void BlinkableComboBox::blink() {
    this->count_--;
    if (this->count_<0) {
        this->setStyleSheet(this->styleSheet_);
        this->setFont(this->font_);
        timer_->stop();
    }

    if (this->count_%2==0)  this->setStyleSheet(this->styleSheetA_);
    else this->setStyleSheet(this->styleSheet_);

}

void BlinkableComboBox::setDuration(int ms) { this->duration_=ms; }

void BlinkableComboBox::setPeriode(int ms) { this->periode_=ms;  }

