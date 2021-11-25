#include "logger.h"

// -------------------------------------
// Logger ------------------------------
// -------------------------------------

Logger::Logger(QObject* parent):
QThread(parent),
parent_(parent) {
    qDebug() << "- " << " " << "Logger::Logger(): " /*<< te->objectName()*/;

    this->timer_=new QTimer(this);

    this->prefix_+="[<b style='color:#"+this->Blue_+";'>";
    this->prefix_+=" I ";
    this->prefix_+="</b>]";
    this->prefix_+="<sup style='float: right;font-size:8px;';>";
    this->prefix_+=std::to_string(this->data_.size());
    this->prefix_+="</sup>";
    this->prefix_+="<span style='margin-left: 20px;'>";

    this->suffix_="</span></div>";

    this->data_.push_back(QString::fromStdString(this->prefix_)+tr("Start Log...")+QString::fromStdString(this->suffix_));
}

Logger::~Logger() { }

void Logger::run() {
    qDebug() << "- " << " " << "Logger::run()";

    // Stop now and emit something
    this->stop_=!this->isWidgetDefined_;

    size_t count=0;

    if (!this->enableWebEngineView_) {
        while(!this->stop_) {
            QThread::msleep(this->freq_);
            count++;

            if (!this->data_.isEmpty()) {
                if (this->history_++>this->historyMax_) {
                    this->history_=0;
                    QTextCursor cursor = this->TE_->textCursor();
                    cursor.movePosition(QTextCursor::Start);

                    // disgusting
                    for(int i=0; i<this->data_.size(); i++) {
                        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 0);
                        cursor.select(QTextCursor::LineUnderCursor);
                        cursor.removeSelectedText();
                        cursor.deleteChar();
                    }
                    this->TE_->setTextCursor(cursor);
                }

                this->TE_->moveCursor (QTextCursor::End);
                QString strs{};

                std::for_each(this->data_.begin(), this->data_.end(),
                              [&strs](const QString &str) {return strs+=str+QString("<br>");});

                this->TE_->insertHtml("<div>"+strs+"</div>");
                this->TE_->moveCursor (QTextCursor::End);
            }

            this->data_.clear();

            while(this->pause_)
                QThread::msleep(this->freq_);
        }
    }
    else {
        while(!this->stop_) {
            QThread::msleep(this->freq_);

            count++;

            if (this->newContent && !this->data_.isEmpty()) {

                if (this->historyMax_< this->data_.size()) {
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
                    this->data_=QVector<QString>(this->data_.begin()-1+static_cast<int>(this->data_.size()*0.8),
                                                 this->data_.end());
#else
                    this->data_=this->data_.mid(static_cast<int>(this->data_.size()*0.8)-1, this->data_.size()-1);
#endif
                    this->append<QString>(tr("Max history reached: 80% removed"), Level::Warning);
                }

                auto reverse_data=this->data_;

                std::reverse(reverse_data.begin(), reverse_data.end());

                std::stringstream ss{};
                ss << this->getHeader();
                for(const auto &line: reverse_data)
                    ss << line.toStdString() << "<br>";
                ss << this->getFooter();

                emit(sendData(QString::fromStdString(ss.str())));

                // auto scroll if content changes
                emit(contentChanged());
                this->newContent=false;
            }

            while(this->pause_)
                QThread::msleep(this->freq_);

        }
    }
    emit(done());
}

void Logger::setTextEdit(QTextEdit* te) {
    if (te!=nullptr) {
        qDebug() << "- " << " " << "Logger::setTextEdit(te): " << te->objectName();

        this->TE_=te;
        this->isWidgetDefined_=true;
        this->enableWebEngineView_=false;
    }
}

void Logger::setWebEngineView(QWebEngineView* view) {
    if (view!=nullptr) {
        qDebug() << "- " << " " << "Logger::WebEngineView(view): " << view->objectName();

        this->weView_=view;
        this->isWidgetDefined_=true;
        this->enableWebEngineView_=true;
    }
}


Logger * Logger::operator() (){ return this; }

void Logger::stop() { this->pause_=false; this->stop_=true; }
void Logger::pause(bool b) { this->pause_ = b; }
void Logger::resume(bool b) { this->pause_ = !b; }


void Logger::setLvlStr_(Level lvl) {
    this->prefix_="<div class='msg'>";
    switch(lvl) {
        case Level::Info:
            this->prefix_+="[<b style='color:#"+this->Blue_+";'>";
            this->prefix_+=" I ";
            this->prefix_+="</b>]";
            this->prefix_+="<sup  style='float:right;font-size:8px;';>";
            this->prefix_+=std::to_string(this->data_.size());
            this->prefix_+="</sup>";
            this->prefix_+="<span style='margin-left: 20px;'>";
            break;
        case Level::Debug:
            this->prefix_+="[<b style='color:#"+this->White_+";'>";
            this->prefix_+=" D ";
            this->prefix_+="</b>]";
            this->prefix_+="<sup  style='float:right;font-size:8px;';>";
            this->prefix_+=std::to_string(this->data_.size());
            this->prefix_+="</sup>";
            this->prefix_+="<span style='margin-left:20px;color:#"+this->White_+";'>";
            break;
        case Level::Warning:
            this->prefix_+="[<b style='color:#"+this->Orange_+";'>";
            this->prefix_+=" W ";
            this->prefix_+="</b>]";
            this->prefix_+="<sup  style='float:right;font-size:8px;';>";
            this->prefix_+=std::to_string(this->data_.size());
            this->prefix_+="</sup>";
            this->prefix_+="<span style='margin-left:20px;color:#"+this->Orange_+";'>";
            break;
        case Level::Error:
            this->prefix_+="[<b style='color:#"+this->Red_+";'>";
            this->prefix_+=" E ";
            this->prefix_+="</b>]";
            this->prefix_+="<sup  style='float:right;font-size:8px;';>";
            this->prefix_+=std::to_string(this->data_.size());
            this->prefix_+="</sup>";
            this->prefix_+="<span style='margin-left:20px;color:#"+this->Red_+";'>";
            break;
        case Level::Fatal:
            this->prefix_+="[<b style='color:#"+this->Black_+";'>";
            this->prefix_+=" F ";
            this->prefix_+="</b>]";
            this->prefix_+="<sup  style='float:right;font-size:8px;';>";
            this->prefix_+=std::to_string(this->data_.size());
            this->prefix_+="</sup>";
            this->prefix_+="<span style='margin-left: 20px;'>";
            break;
        case Level::count:
            break;
        default:
            break;
    }
}


std::string Logger::getHeader() const {
    std::stringstream ss{};

    ss << "<html>";

    ss << "<style>";
    ss << ".msg {";
    ss << "    display:inline-block;";
    ss << "    position:relative;";
    ss << "    background-color: #004968;";
    ss << "border-right: solid grey 2px;";
    ss << "border-bottom: solid grey 1px;";
    ss << "border-radius: 10px 100px / 120px;;";
    ss << "    padding-top: 4px;";
    ss << "    padding-left: 4px;";
    ss << "    padding-right: 2px;";
    ss << "    padding-bottom: 4px;";
    ss << "    margin-bottom: 4px;";
    ss << "    margin-right: 0px;";
    ss << "    margin-top: 3px;";
    ss << "    width:99%;";
    ss << "}";

    ss << "</style>";

    ss << "<body style='";
    ss << "font-family: system-ui;";
    ss << "font-size:10px;";
    ss << "color:" << this->textColor_ << ";";
    ss << "background-color:" << this->backgroundColor_ << ";";
    ss << "'>";

    ss << "<div style='";
    ss << "float: left;";
    ss << "border: solid #0000ff;";
    ss << "border-width: 0 3px 3px 0;";
    ss << "display: inline-block;";
    ss << "padding: 3px;";
    ss << "transform: rotate(-135deg)  translate(-3px,5px);";
    ss << "-webkit-transform: rotate(-135deg) translate(-3px,5px);'";
    ss << "></div>";

    return ss.str();
}

std::string Logger::getFooter() const {
    std::stringstream ss{};

    ss << "<div id='scrollingContainer'></div>";
    ss << "</body>";
    ss << "</html>";

    return ss.str();
}

QString Logger::getCommentIdx_() const {
    return "<!--" + (QString::fromStdString(std::to_string(this->data_.size())) + "-->");
}

void Logger::saveLog(const QString& path) {
    qDebug() << "- " << " " << "Logger::saveLog(path): " << path;


}
