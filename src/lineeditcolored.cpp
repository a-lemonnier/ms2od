#include "lineeditcolored.h"


LineEditColored::LineEditColored(QWidget* parent):
    parent_(parent) {

        if ( this->parent_ != nullptr ) {
                this->color_= parent_->palette().color ( QPalette::Base );
                this->palette_= parent_->palette();

        }
        else {
            this->color_= this->palette().color ( QPalette::Base );
            this->palette_=this->palette().color ( QPalette::Base);
        }

        this->setMouseTracking(true);
        this->installEventFilter(this);

        this->setAutoFillBackground(true);

        this->colors_[0]=this->DefCol0;
        this->colors_[1]=this->DefCol1;
        this->colors_[2]=this->DefCol2;

        this->paletteA_.setColor(QPalette::Base,  this->colors_[2].first);
        this->paletteB_.setColor(QPalette::Base,  this->colors_[2].second);

        this->updatePalette_();
        this->setPalette ( this->paletteA_ );

        this->blurEffect_=new QGraphicsBlurEffect(this);

        this->blurEffect_->setBlurHints(QGraphicsBlurEffect::QualityHint);
        this->blurEffect_->setBlurRadius(2);
        this->blurEffect_->setEnabled(false);

        this->setGraphicsEffect(this->blurEffect_);
}

LineEditColored::~LineEditColored() { }

void LineEditColored::setColor(std::pair<QColor, QColor> cc, int state) {
    if (state>=0 && state<3) this->colors_[state]=cc;
    this->updatePalette_();
    this->setPalette ( this->paletteA_ );
}

void LineEditColored::setState(int state) {
    this->state_=state;
    this->updatePalette_();
    this->setPalette ( this->paletteA_ );
}


bool LineEditColored::eventFilter(QObject* obj, QEvent* event) {

    if (event->type()==QEvent::Enter)  {
        this->updatePalette_();
        this->setPalette ( this->paletteB_ );
    }

    if ( event->type() ==QEvent::Leave ||
        static_cast<QKeyEvent*> ( event )->key() ==Qt::Key_Return )  {
            this->updatePalette_();
            this->setPalette ( this->paletteA_ );
    }

    return QWidget::eventFilter(obj, event);
}

void LineEditColored::updatePalette_() {
    this->paletteA_.setColor(QPalette::Base,  this->colors_[this->state_()].first);
    this->paletteB_.setColor(QPalette::Base,  this->colors_[this->state_()].second);

}

void LineEditColored::resetPalette() {
    this->setPalette ( this->palette_);
}

void LineEditColored::setBlured(bool b) {
    this->blurEffect_->setEnabled(b);
}

