#include "mainwindow.h"
#include "./ui_mainwindow.h"


const std::map<QString, QVector<QString>> MainWindow::AllowedExtensions_= {
    {".ods", {".xls", ".xlsx", ".csv"}},
    {".odt", {".doc", ".docx", ".txt"}},

    {".xls",  {".ods", ".csv"}},
    {".xlsx", {".ods", ".csv"}},

    {".txt",  {".odt", ".doc"}},
    {".csv",  {".ods", ".xls"}},
};

const std::map<QString, QString> MainWindow::LineEditStyle_=
{
{
 "valid",
 "border-color: rgb(0, 255, 0);"
 "border-width : 1.0px;"
 "border-style:inset;"
 "font: 7pt;"
}
,
{
 "invalid",
 "border-color: rgb(255, 0, 0);"
 "border-width : 1.0px;"
 "border-style:inset;"
 "font: 7pt;"
}
};


bool MainWindow::State::operator ()() {
    return Input &&
           Output &&
           Extensions &&
           Binary;
}


MainWindow::MainWindow(QTranslator* translator,
                       QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , translator_(translator) {
    ui->setupUi(this);
    this->setWindowFlag(Qt::WindowContextHelpButtonHint);
    this->setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    this->setWindowFlag(Qt::WindowMinimizeButtonHint, false);
    this->setWindowFlag(Qt::WindowMinMaxButtonsHint, false);

    // Set Log
    this->initWebEngineView_();
    this->Log_=new Logger( this);
    this->Log_->setWebEngineView(this->weView_);
    connect(this->Log_, &Logger::sendData, this,
            [&](const QString& str) {
        this->weView_->setHtml(str);
    });
    connect(this->Log_, &Logger::finished, this->Log_, &Logger::deleteLater);

    this->Log_->start();

    ui->tabWidget->setCurrentIndex(0);

    // Fill led labels with pixmaps
    this->initLeds_();

    // Fill CB with filetypes
    this->initComboBox_();

    this->disableStart_();
    this->disableLcdNumber_();
    this->disableProgessBar_();

    // Check permission of /var/tmp
    auto tmp_perms=std::filesystem::status("/var/tmp").permissions();
    if ((tmp_perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none) {
        this->confLocation_="/var/tmp/ms2od.conf";
        this->Log_->append(tr("Change default config file location to /var/tmp")+".", Logger::Level::Info);
    }
    else
        this->Log_->append(tr("User does not have permission to write config file in /var/tmp")+".", Logger::Level::Warning);

    // Load Conf file and update strings
    this->confAvailable_=this->loadConf(this->confLocation_);

    ui->lineEdit_dirOut->setText(this->OutDirPath_);
    ui->lineEdit_dirIn->setText(this->InDirPath_);
    ui->lineEdit_LOLoc->setText(this->LOPath_);
    ui->lineEdit_LogPath->setText(this->LogPath_);

    ui->lineEdit_LDLIB->setState(2);

    this->setFont(QFont(this->font().family(), 7));

    if (std::filesystem::exists(this->LOPath_.toStdString())) {
        this->State_.Binary=true;
        ui->lineEdit_LOLoc->setState(1);
    }
    else
        ui->lineEdit_LOLoc->setState(0);

    if (std::filesystem::exists(this->LogPath_.toStdString()))
        ui->lineEdit_LogPath->setState(1);
    else
        ui->lineEdit_LogPath->setState(2);

    if (this->confAvailable_) {
        this->updateComboBoxConv_();
        this->updateInDirPath_();
        this->updateOutDirPath_();
    }

    // Flag
    if (this->locale().language()==QLocale::French) {
        ui->pushButton_Flag->setIcon(this->iconFlagFr_);
        this->isEng_=false;
    }
    else {
        ui->pushButton_Flag->setIcon(this->iconFlagEn_);
        this->isEng_=true;
    }

    // Tree
    this->RebuildFolderTree_=true;
    ui->checkBox_RestoreFolderTree->setChecked(true);

    // LO output
    this->displayFileName_=true;
    ui->checkBox_DispLOOutput->setChecked(true);

    // Misc.
    ui->pushButton_showLog->setEnabled(false);

    // Extra connections
    connect(ui->dockWidget, &DockWidgetCloseDetect::closed, this,
            [&]() {
        ui->pushButton_showLog->setEnabled(true);
        this->Log_->append(tr("Log hidden")+".", Logger::Level::Info);
    });

    // Timer
    this->TimerStateCheck_=new QTimer(this);
    connect(this->TimerStateCheck_, &QTimer::timeout, this, &MainWindow::updateStateLed_);
    this->TimerStateCheck_->start(this->freqStateCheck_);

    // unused
    this->storeStyleSheet_();

    this->Log_->append(tr("Waiting for an order")+"...", Logger::Level::Info);

    qDebug() << "- " <<  " " << "MainWindow() done";


    // icons
    // https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
    if (!QIcon::fromTheme("folder").isNull()) {
        ui->pushButton_browseIn->setIcon(QIcon::fromTheme("folder"));
        ui->pushButton_browseOut->setIcon(QIcon::fromTheme("folder"));
        ui->pushButton_LOLoc->setIcon(QIcon::fromTheme("folder"));
        ui->pushButton_LogLoc->setIcon(QIcon::fromTheme("folder"));

        ui->pushButton_browseIn->setText("");
        ui->pushButton_browseOut->setText("");
        ui->pushButton_LOLoc->setText("");
        ui->pushButton_LogLoc->setText("");
    }

    if (!QIcon::fromTheme("document-save").isNull()) {
        ui->pushButton_LDLIB->setIcon(QIcon::fromTheme("document-save"));

        ui->pushButton_LDLIB->setText("");
    }

    //Fix this
    ui->label_RemainTime->addValue(1);
}

MainWindow::~MainWindow() {
    this->Log_->stop();
    this->Log_->wait();
    this->saveConf(this->confLocation_);
    delete ui;
}

void MainWindow::initWebEngineView_() {
    this->weView_ = new QWebEngineView(this);
    ui->dockWidget->setWidget(this->weView_);
    this->weView_->setMinimumHeight(200);
    this->weView_->setContextMenuPolicy(Qt::NoContextMenu);
    this->weView_->show();
}

void MainWindow::initLeds_() {
    qDebug() << "- " <<  " " << "initLeds_()";

    ui->label_LedIn->setScaledContents(true);
    ui->label_LedOut->setScaledContents(true);
    ui->label_LedBin->setScaledContents(true);
    ui->label_LedExt->setScaledContents(true);

    ui->label_LedIn->setMaximumSize(this->sizeLed_);
    ui->label_LedOut->setMaximumSize(this->sizeLed_);
    ui->label_LedBin->setMaximumSize(this->sizeLed_);
    ui->label_LedExt->setMaximumSize(this->sizeLed_);

    ui->label_LedIn->setPixmap(this->pmLedRed_);
    ui->label_LedOut->setPixmap(this->pmLedRed_);
    ui->label_LedBin->setPixmap(this->pmLedRed_);
    ui->label_LedExt->setPixmap(this->pmLedRed_);
}

void MainWindow::initComboBox_() {
    qDebug() << "- " <<  " " << "initComboBox_()";

    // set max max=5 due to LO port ( 65 535 )
    int max_thread=QThread::idealThreadCount() > 5  ? 5 : QThread::idealThreadCount();
    this->Log_->append<std::string>({tr("Set max threads to").toStdString(), " ", std::to_string(max_thread), "."}, Logger::Level::Info);

    ui->spinBox_ThreadNumbers->setMaximum( max_thread );
    ui->spinBox_ThreadNumbers->setValue(max_thread>1 ? max_thread/2 : 1);

    for(const auto &key: this->AllowedExtensions_)
       ui->comboBox_conversionIn->addItem(key.first);

    ui->comboBox_conversionIn->setPeriode(200);
    ui->comboBox_conversionIn->setDuration(800);
    ui->comboBox_conversionIn->start();
}


void MainWindow::storeStyleSheet_() {
    auto fill_with=[&](const auto &w) {
        this->backupStyleSheet_[w->objectName()]=w->styleSheet();
        this->backupPalette_[w->objectName()]=w->palette();
    };

    fill_with(ui->lineEdit_dirIn);
    fill_with(ui->lineEdit_dirOut);
    fill_with(ui->lineEdit_LOLoc);

    fill_with(ui->pushButton_start);
    fill_with(ui->pushButton_Pause);
}

bool MainWindow::browseFile(QString &path, LineEditColored &le) {
    qDebug() << "- " <<  " " << "browseFile(path, le): " << path << " " << le.objectName();

    QFileDialog fd(this, Qt::Dialog);
    fd.setDirectory(QString::fromStdString(std::filesystem::path(path.toStdString()).parent_path().string()));
    fd.selectFile(path);
    fd.setFileMode(QFileDialog::ExistingFile);

    if (!fd.exec()) return false;
    QString new_path=fd.selectedFiles()[0];

    if (!new_path.isEmpty()) {
        if (!std::filesystem::exists(new_path.toStdString())) {
            le.setState(0);
            this->Log_->append(tr("Binary does not exist "), Logger::Level::Info);
            return false;
        }

        path=new_path;
        le.setState(1);
        this->Log_->append<QString>({tr("Path changed to"), ": ", path, "."}, Logger::Level::Info);
        return true;
    }
    return false;
}

bool MainWindow::browseDir(QString &path, LineEditColored &le) {
    qDebug() << "- " <<  " " << "browseDir(path, le): " << path << " " << le.objectName();

    QFileDialog fd(this, Qt::Dialog);

    fd.setDirectory(QString::fromStdString(std::filesystem::path(path.toStdString()).parent_path().string()));
    fd.selectFile(path);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setOption(QFileDialog::ShowDirsOnly, true);

    if (!fd.exec()) return false;
    QString new_path=fd.selectedFiles()[0];

    if (!new_path.isEmpty()) {
        if (!std::filesystem::exists(new_path.toStdString())) {
            le.setState(0);
            this->Log_->append<QString>({tr("Path is invalid"), ": ", path, "."}, Logger::Level::Error);
            return false;
        }

        path=new_path;
        le.setText(new_path);
        le.setState(1);
        this->Log_->append<QString>({tr("Path changed to"), ": ", path, "."}, Logger::Level::Info);
        return true;
    }
    return false;
}

void MainWindow::enableBrowsing_(bool b) {
    qDebug() << "- " <<  " " << "enableBrowsing_()";

    ui->lineEdit_dirIn->setEnabled(b);
    ui->lineEdit_dirOut->setEnabled(b);
    ui->pushButton_browseIn->setEnabled(b);
    ui->pushButton_browseOut->setEnabled(b);
    ui->checkBox_RestoreFolderTree->setEnabled(b);
    QApplication::processEvents();
}

void MainWindow::disableBrowsing_(bool b) {
    qDebug() << "- " <<  " " << "disableBrowsing_()";

    this->enableBrowsing_(!b);
}

void MainWindow::enableStart_(bool b) {
    ui->pushButton_start->setEnabled(b);
}

void MainWindow::disableStart_(bool b) {
    this->enableStart_(!b);
}


void MainWindow::enableProgessBar_(bool b) {
    qDebug() << "- " <<  " " << "enableProgessBar_()";

    ui->progressBar->setEnabled(b);
    ui->progressBar->setVisible(b);
    ui->horizontalSpacer_ProgressBar->changeSize(QSizePolicy::Preferred ,QSizePolicy::Preferred);
}

void MainWindow::disableProgessBar_(bool b) {
    qDebug() << "- " <<  " " << "disableProgessBar_()";

    ui->progressBar->setEnabled(!b);
    ui->progressBar->setVisible(!b);
    ui->horizontalSpacer_ProgressBar->changeSize(QSizePolicy::Expanding ,QSizePolicy::Expanding);
}

void MainWindow::enableLcdNumber_(bool b) {
    qDebug() << "- " <<  " " << "enableLcdNumber_()";

    ui->lcdNumber->setEnabled(b);
    ui->lcdNumber->setVisible(b);

    ui->label_Counter->setEnabled(b);
    ui->label_Counter->setVisible(b);
}

void MainWindow::disableLcdNumber_(bool b) {
    qDebug() << "- " <<  " " << "disableLcdNumber_()";

    this->enableLcdNumber_(!b);
}

void MainWindow::blurWidgets(bool b, std::vector<QWidget*> exclusion_vector) {
    auto exclude=[](const std::vector<QWidget*> &vector, const QWidget* widget) {
        return std::any_of(vector.begin(), vector.end(), [widget](const QWidget* w) {
            return w==widget;
        });
    };

    for(auto &w: this->findChildren<QLabel*>()) if (!exclude(exclusion_vector, w)) this->blurThis(w, b);
    for(auto &w: this->findChildren<QPushButton*>()) if (!exclude(exclusion_vector, w)) this->blurThis(w, b);
    for(auto &w: this->findChildren<QComboBox*>()) if (!exclude(exclusion_vector, w)) this->blurThis(w, b);
    for(auto &w: this->findChildren<QCheckBox*>()) if (!exclude(exclusion_vector, w)) this->blurThis(w, b);
    for(auto &w: this->findChildren<QProgressBar*>()) if (!exclude(exclusion_vector, w)) this->blurThis(w, b);
    for(auto &w: this->findChildren<QTabBar*>()) if (!exclude(exclusion_vector, w)) this->blurThis(w, b);

}


bool MainWindow::saveConf(const std::string &path) {
    qDebug() << "- " <<  " " << "saveConf(path): " << QString::fromStdString(path);

    std::fstream flux(path, std::ios::out | std::ios::ate);

    if (flux) {
        this->Log_->append<std::string>({tr("Writing configuration into").toStdString(), ": ", path, "."}, Logger::Level::Info);

        flux << "[InDirPath]" << std::endl;
        flux << this->InDirPath_.toStdString() << std::endl << std::endl;

        flux << "[OutDirPath]" << std::endl;
        flux << this->OutDirPath_.toStdString() << std::endl << std::endl;

        flux << "[LOPath]" << std::endl;
        flux << this->LOPath_.toStdString() << std::endl << std::endl;

        flux << "[LogPath]" << std::endl;
        flux << this->LogPath_.toStdString() << std::endl << std::endl;

        flux << "[freqStateCheck]" << std::endl;
        flux << this->freqStateCheck_ << std::endl << std::endl;

        flux << "[conversionIn]" << std::endl;
        flux << ui->comboBox_conversionIn->currentText().toStdString() << std::endl << std::endl;

        flux << "[conversionOut]" << std::endl;
        flux << ui->comboBox_conversionOut->currentText().toStdString() << std::endl << std::endl;

        flux << "[RebuildFolderTree]" << std::endl;
        flux << static_cast<int>(ui->checkBox_RestoreFolderTree->isChecked()) << std::endl << std::endl;

        flux << "[displayFileName]" << std::endl;
        flux << static_cast<int>(ui->checkBox_DispLOOutput->isChecked()) << std::endl << std::endl;

        flux.close();
        return true;
    }
    return false;
}

bool MainWindow::loadConf(const std::string &path) {
    qDebug() << "- " <<  " " << "loadConf(path): " << QString::fromStdString(path);

    bool status=true;

    if (!std::filesystem::exists(path)) return false;

    std::fstream flux(path, std::ios::in);

    if (flux) {

        this->Log_->append<QString>({tr("Loading config file"), "."}, Logger::Level::Info);

        std::string line;
        std::vector<std::string> data;
        while(std::getline(flux, line)) {
            data.emplace_back(line);
        }

        auto res_indir=std::find(data.begin(), data.end(), "[InDirPath]");
        auto pos_indir=res_indir-data.begin()+1;

        auto res_outdir=std::find(data.begin(), data.end(), "[OutDirPath]");
        auto pos_outdir=res_outdir-data.begin()+1;

        auto res_LOdir=std::find(data.begin(), data.end(), "[LOPath]");
        auto pos_LOdir=res_LOdir-data.begin()+1;

        auto res_Logdir=std::find(data.begin(), data.end(), "[LogPath]");
        auto pos_Logdir=res_Logdir-data.begin()+1;

        auto res_freq=std::find(data.begin(), data.end(), "[freqStateCheck]");
        auto pos_freq=res_freq-data.begin()+1;

        auto res_convin=std::find(data.begin(), data.end(), "[conversionIn]");
        auto pos_convin=res_convin-data.begin()+1;

        auto res_convout=std::find(data.begin(), data.end(), "[conversionOut]");
        auto pos_convout=res_convout-data.begin()+1;

        auto res_rebuildtree=std::find(data.begin(), data.end(), "[RebuildFolderTree]");
        auto pos_rebuildtree=res_rebuildtree-data.begin()+1;

        auto res_showFileName=std::find(data.begin(), data.end(), "[displayFileName]");
        auto pos_showFileName=res_showFileName-data.begin()+1;

        status = pos_indir    <   data.size() &&
                 pos_outdir   <   data.size( )&&
                 pos_LOdir    <   data.size() &&
                 pos_Logdir   <   data.size() &&
                 pos_convin   <   data.size() &&
                 pos_convout  <   data.size() &&
                 pos_rebuildtree <data.size() &&
                 pos_showFileName<data.size();

        if ( pos_indir   < data.size() ) this->InDirPath_=QString::fromStdString(data[pos_indir]);
        if ( pos_outdir  < data.size() ) this->OutDirPath_=QString::fromStdString(data[pos_outdir]);
        if ( pos_LOdir   < data.size() ) this->LOPath_=QString::fromStdString(data[pos_LOdir]);
        if ( pos_Logdir  < data.size() ) this->LogPath_=QString::fromStdString(data[pos_Logdir]);
        if ( pos_convin  < data.size() ) ui->comboBox_conversionIn->setCurrentText(QString::fromStdString(data[pos_convin]));
        if ( pos_convout < data.size() ) ui->comboBox_conversionOut->setCurrentText(QString::fromStdString(data[pos_convout]));

        if ( pos_rebuildtree < data.size() ) {
            try {
                this->RebuildFolderTree_=static_cast<bool>(std::stoi(data[pos_rebuildtree]));
                ui->checkBox_RestoreFolderTree->setChecked(this->RebuildFolderTree_);
            }
            catch (std::invalid_argument &e) {
                this->Log_->append<std::string>({tr("Cannot load config file").toStdString(), ": ",e.what(), "."}, Logger::Level::Info);
                status=false;
            }
        }

        if ( this->InDirPath_.size()  <2 ||
             this->OutDirPath_.size() <2 ||
             this->LOPath_.size()     <2 ) status = false;

        if ( pos_freq   <   data.size() ) {
            try {
                this->freqStateCheck_= std::stoi(data[pos_freq]);
            }
            catch(std::invalid_argument &e) {
                this->Log_->append<std::string>({tr("Cannot load config file").toStdString(), ": ", e.what(), "."}, Logger::Level::Info);
                status=false;
            }
        }

        if ( pos_showFileName < data.size() ) {
            try {
                this->displayFileName_=static_cast<bool>(std::stoi(data[pos_showFileName]));
                ui->checkBox_DispLOOutput->setChecked(this->displayFileName_);
            }
            catch (std::invalid_argument &e) {
                this->Log_->append<std::string>({tr("Cannot load config file").toStdString(), ": ", e.what(), "."}, Logger::Level::Info);
                status=false;
            }
        }
        flux.close();
    }
    return status;
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type()==QEvent::LanguageChange) {
        this->Log_->append(tr("Language changed."), Logger::Level::Info);
         ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}

void MainWindow::blurThis(QWidget* w, bool enable, qreal radius, qreal duration) {
    // if the widget has no effect or is not in the set, then add it.
    // else just toggle it

    bool found=false;
    int pos=0;
    for(const auto &[widget, effect]: this->effectList_) {
       pos++;
       if (widget==w) {
           found=true;
           pos-=1;
           break;
       }
    }

    if (!found) {
        qDebug() << "- " <<  " " << "MainWindow::blurThis(" << w->objectName() <<"): not found";

        QGraphicsBlurEffect* be=new QGraphicsBlurEffect(w);

        be->setBlurHints(QGraphicsBlurEffect::AnimationHint);
        be->setBlurRadius(2);
        w->setGraphicsEffect(be);

        this->effectList_.push_back({w, be});

        be->setEnabled(enable);
    }
    else {
        qDebug() << "- " <<  " " << "MainWindow::blurThis(" << w->objectName() <<"): found";

        qobject_cast<QGraphicsBlurEffect*>(std::get<1>(this->effectList_[pos]))->setEnabled(enable);
         qobject_cast<QGraphicsBlurEffect*>(std::get<1>(this->effectList_[pos]))->setBlurRadius(radius);
    }

}

void MainWindow::updateComboBoxConv_() {
    qDebug() << "- " <<  " " << "updateComboBoxConv_()";

    auto idx=ui->comboBox_conversionIn->currentText();

    ui->comboBox_conversionOut->clear();
    for(const auto& val: this->AllowedExtensions_.at(idx))
        ui->comboBox_conversionOut->addItem(val);

    ui->comboBox_conversionIn->setCurrentText(idx);

    this->enableBrowsing_();
    this->State_.Extensions=true;
}


void MainWindow::updateInDirPath_() {
    qDebug() << "- " <<  " " << "updateInDirPath_()";

    this->State_.Input=this->updateDirPath_(this->InDirPath_,
                                            *ui->lineEdit_dirIn);
    if (this->State_.Input) {

        ui->centralwidget->setEnabled(false);
        this->enableLcdNumber_();

        FileFinder* ff=new FileFinder();

        ff->setDirectory(this->InDirPath_);
        ff->setExtension(ui->comboBox_conversionIn->currentText());

        connect(ff, &FileFinder::done, this,
                [&](const std::vector<std::string> &vec_files) {
            this->inVector_=vec_files;
        });

        connect(ff, &FileFinder::updateCounter, this,
                [this](int counter) {
            ui->lcdNumber->display(counter);
            this->FileCounter_=counter;
        });

        connect(ff, &FileFinder::finished, this,
                [&]() {
            std::stringstream ss;
            ss << "\t";
            ss << this->FileCounter_;
            ss << " ";
            ss << tr("files found").toStdString();
            ss << "." << std::endl;

            this->Log_->append(ss.str(), Logger::Level::Info);

            ui->centralwidget->setEnabled(true);
        });

        this->Log_->append<QString>({tr("Search for files"), "..."}, Logger::Level::Info);

        ff->start();
    }
}



void MainWindow::updateOutDirPath_() {
    qDebug() << "- " <<  " " << "updateOutDirPath_()";

    this->State_.Output=this->updateDirPath_(this->OutDirPath_,
                                             *ui->lineEdit_dirOut);
}


bool MainWindow::updateDirPath_(QString &path, LineEditColored &le) {
    qDebug() << "- " <<  " " << "updateDirPath_(path, le): " << path << " " << le.objectName();

    QString new_path=path;

    if (!new_path.isEmpty()) {
        if (!std::filesystem::exists(new_path.toStdString())) {
            le.setState(0);
            this->Log_->append<QString>({tr("Path is invalid"), ": ", path, "."}, Logger::Level::Error);
            return false;
        }

        path=new_path;
        le.setText(new_path);
        le.setState(1);
        return true;
    }
    return false;
}


template<typename T>
std::vector<std::vector<T>> MainWindow::sliceVector_(const std::vector<T>& vec, int n) {
    std::vector<std::vector<T>> res{};

    qDebug() << "-\t" << "MainWindow::sliceVector_(): " << "n=" << n << " - " << "vec.size()=" << vec.size();

    int trim_size=static_cast<int>(std::floor(vec.size()/n));
    qDebug() << "-\t" << "MainWindow::sliceVector_(): " << "trim_size=" << trim_size;

    for(int i=0; i<n; i++) {
        std::vector<T> v{};
        for(int j=0; j<trim_size; j++) {
            v.emplace_back(vec[trim_size*i+j]);
        }
        res.emplace_back(v);
    }

    if (vec.size()-trim_size*n!=0) {
        std::vector<T> v{};

        // add additionnal strings to the last vector
        for(int i=trim_size*n; i<vec.size() ;i++)
            res[res.size()-1].emplace_back(vec[i]);

        qDebug() << "-\t" << "MainWindow::sliceVector_(): " << "vec.size()-trim_size*n=" << vec.size()-trim_size*n;
       }

    // test size
    size_t final_size=0;
    for(const auto &v: res) final_size+=v.size();

    if (final_size!=vec.size())
        this->Log_->append<QString>({tr("Trimming failed"), "."}, Logger::Level::Error);

    qDebug() << "-\t" << "MainWindow::sliceVector_(): " << "done with total size: " << final_size;

    return res;
}


// -------------------------------------
// TIMERS ------------------------------
// -------------------------------------

void MainWindow::updateStateLed_() {
    if (State_.Input) ui->label_LedIn->setPixmap(this->pmLedGreen_);
    else              ui->label_LedIn->setPixmap(this->pmLedRed_);

    if (State_.Input) ui->label_LedIn->setPixmap(this->pmLedGreen_);
    else              ui->label_LedIn->setPixmap(this->pmLedRed_);

    if (State_.Output)ui->label_LedOut->setPixmap(this->pmLedGreen_);
    else              ui->label_LedOut->setPixmap(this->pmLedRed_);

    if (State_.Binary)ui->label_LedBin->setPixmap(this->pmLedGreen_);
    else              ui->label_LedBin->setPixmap(this->pmLedRed_);

    if (State_.Extensions)
                      ui->label_LedExt->setPixmap(this->pmLedGreen_);
    else              ui->label_LedExt->setPixmap(this->pmLedRed_);

    this->enableStart_(State_());
}



// -------------------------------------
// SLOTS -------------------------------
// -------------------------------------

void MainWindow::on_pushButton_quit_clicked() {
    this->close();
}

void MainWindow::on_comboBox_conversionIn_currentTextChanged(const QString &index) {
        if (this->extFirstTimeSelected_) {
            ui->comboBox_conversionIn->removeItem(0);
            this->extFirstTimeSelected_=false;
        }

        std::stringstream ss{};

        ui->comboBox_conversionOut->clear();
        for(const auto& val: this->AllowedExtensions_.at(index)) {
            ui->comboBox_conversionOut->addItem(val);
        }

        this->enableBrowsing_();
        this->State_.Extensions=true;

        this->updateInDirPath_();
}

void MainWindow::on_dockWidget_topLevelChanged(bool topLevel) {
    ui->centralwidget->setMaximumSize(ui->centralwidget->sizeHint());
    this->setMaximumSize(this->sizeHint().width(), this->sizeHint().height());
}

void MainWindow::on_pushButton_LOLoc_clicked() {
    this->State_.Binary=this->browseFile(this->LOPath_,
                                         *ui->lineEdit_LOLoc);
}

void MainWindow::on_pushButton_browseIn_clicked() {
    this->State_.Input=this->browseDir(this->InDirPath_,
                                       *ui->lineEdit_dirIn);
    if (this->State_.Input) {

        ui->centralwidget->setEnabled(false);
        this->enableLcdNumber_();

        FileFinder* ff=new FileFinder();

        ff->setDirectory(this->InDirPath_);
        ff->setExtension(ui->comboBox_conversionIn->currentText());

        connect(ff, &FileFinder::done, this,
                [&](const std::vector<std::string> &vec_files) {
            this->inVector_=vec_files;
        });

        connect(ff, &FileFinder::updateCounter, this,
                [this](int counter) {
            ui->lcdNumber->display(counter);
            this->FileCounter_=counter;
        });

        connect(ff, &FileFinder::finished, this,
                [&]() {
            std::stringstream ss;
            ss << "\t";
            ss << this->FileCounter_;
            ss << " ";
            ss << tr("files found").toStdString();
            ss << "." << std::endl;

            this->Log_->append(ss.str(), Logger::Level::Info);

            ui->centralwidget->setEnabled(true);
        });

        this->Log_->append<QString>({tr("Search for files"), "..."}, Logger::Level::Info);
        ff->start();
    }
}


void MainWindow::on_pushButton_browseOut_clicked() {
    this->State_.Output=this->browseDir(this->OutDirPath_, *ui->lineEdit_dirOut);
}

void MainWindow::on_pushButton_LogLoc_clicked() {
    this->browseDir(this->LogPath_, *ui->lineEdit_LogPath);
}


void MainWindow::on_pushButton_LDLIB_clicked() {

}


void MainWindow::on_lineEdit_dirIn_editingFinished() {
    qDebug() << "- " <<  " " << "on_lineEdit_dirIn_editingFinished()";

    auto new_path=ui->lineEdit_dirIn->text();
    if (!new_path.isEmpty()) {
        if (!std::filesystem::exists(new_path.toStdString())) {
            ui->lineEdit_dirIn->setState(0);
            this->Log_->append<QString>({tr("Path is invalid"), ": ", new_path, "."}, Logger::Level::Error);
            return;
        }

        this->InDirPath_=new_path;
        ui->lineEdit_dirIn->setText(new_path);
        ui->lineEdit_dirIn->setState(1);
        this->Log_->append<QString>({tr("Path changed to:"), ": ", new_path, "."}, Logger::Level::Info);
        this->State_.Input=true;
    }
}


void MainWindow::on_lineEdit_dirOut_editingFinished() {
    qDebug() << "- " <<  " " << "on_lineEdit_dirOut_editingFinished()";

    auto new_path=ui->lineEdit_dirOut->text();
    if (!new_path.isEmpty()) {
        if (!std::filesystem::exists(new_path.toStdString())) {
            ui->lineEdit_dirOut->setState(0);
            this->Log_->append<QString>({tr("Path is invalid"), ": ", new_path, "."}, Logger::Level::Error);
            return;
        }

        this->OutDirPath_=new_path;
        ui->lineEdit_dirOut->setText(new_path);
        ui->lineEdit_dirOut->setState(1);
        std::cout << "something wrong here" << std::endl;
        this->Log_->append<QString>({tr("Path changed to:"), ": ", new_path, "."}, Logger::Level::Info);

        this->State_.Output=true;
    }
}


void MainWindow::on_lineEdit_LOLoc_editingFinished() {
    qDebug() << "- " <<  " " << "on_lineEdit_LOLoc_editingFinished()";

    auto new_path=ui->lineEdit_LOLoc->text();
    if (!new_path.isEmpty()) {
        if (!std::filesystem::exists(new_path.toStdString())) {
            ui->lineEdit_LOLoc->setState(0);
            this->Log_->append<QString>({tr("Path is invalid"), ": ", new_path, "."}, Logger::Level::Error);
            this->State_.Binary=false;
            return;
        }

        this->LOPath_=new_path;
        ui->lineEdit_LOLoc->setText(new_path);
        ui->lineEdit_LOLoc->setState(1);
        this->Log_->append<QString>({tr("Path changed to:"), ": ", new_path, "."}, Logger::Level::Info);

        this->State_.Binary=true;
    }
}


void MainWindow::on_lineEdit_LogPath_editingFinished() {
    qDebug() << "- " <<  " " << "on_lineEdit_LogPath_editingFinished()";

    auto new_path=ui->lineEdit_LogPath->text();
    if (!new_path.isEmpty()) {
        if (!std::filesystem::exists(new_path.toStdString())) {
            ui->lineEdit_LogPath->setState(2);
            this->Log_->append<QString>({tr("Path is invalid"), ": ", new_path, "."}, Logger::Level::Warning);
            return;
        }

        this->LogPath_=new_path;
        ui->lineEdit_LogPath->setText(new_path);
        ui->lineEdit_LogPath->setState(1);
        this->Log_->append<QString>({tr("Path changed to:"), ": ", new_path, "."}, Logger::Level::Info);
    }

}


void MainWindow::on_lineEdit_LDLIB_editingFinished() {
    qDebug() << "- " <<  " " << "on_lineEdit_LDLIB_editingFinished()";
}


void MainWindow::on_pushButton_About_clicked() {
    std::stringstream ss{};
    ss << "Qt version: " << QString(qVersion()).toStdString() << ".\n";
    this->Log_->append(ss.str());
}

void MainWindow::on_dockWidget_destroyed() { }

void MainWindow::on_dockWidget_visibilityChanged(bool visible) {
    ui->centralwidget->setMaximumSize(ui->centralwidget->sizeHint());
    this->setMaximumSize(this->sizeHint().width(), this->sizeHint().height());
}

void MainWindow::on_pushButton_showLog_clicked() {
    ui->dockWidget->setVisible(true);
    ui->pushButton_showLog->setEnabled(false);
}

void MainWindow::on_pushButton_start_clicked() {

    this->TimerStateCheck_->stop();

    this->disableBrowsing_();
    this->disableStart_();
    ui->pushButton_Stop->setEnabled(true);
    ui->pushButton_Pause->setEnabled(true);
    ui->comboBox_conversionIn->setEnabled(false);
    ui->comboBox_conversionOut->setEnabled(false);
    this->enableProgessBar_();
    ui->progressBar->setValue(0);
    ui->tabWidget->setTabEnabled(1, false);

    this->blurWidgets(true, { ui->pushButton_Pause,
                              ui->pushButton_quit,
                              ui->label_Counter,
                              ui->progressBar,
                              ui->pushButton_Stop,
                              ui->label_RemainTime });


    // Clean previous threads ---------------------
    for(const auto &dir: std::filesystem::directory_iterator("/tmp")) {
        if (dir.path().filename().string().find(this->tmpFolderPrefixConv_)!=std::string::npos) {
            std::error_code ec;
            std::filesystem::remove_all(dir, ec);
            if (ec) qDebug() << "-\t" << "Cannot remove " << dir.path().filename().c_str() << ": " << ec.message().c_str();
        }
    }

    for(auto &conv: this->vecConvert_) {
        qDebug() << "-\t" << "Clean thread idx=" << conv->getIdx();
        conv->resetIdx();
        delete conv;
    }

     this->vecConvert_.clear();
    // --------------------------------------------

    // Remaining time -----------------------------
    ui->label_RemainTime->reset();
    ui->label_RemainTime->setSupport(0., this->inVector_.size());
    ui->label_RemainTime->setSampleSize(50);
    ui->label_RemainTime->setMode(RemainingTimeLabel::Mode::Time);
    ui->label_RemainTime->setThreadNumber(this->ThreadNumber_);
    ui->label_RemainTime->start();
     // --------------------------------------------

    QApplication::processEvents();

    // Slicing ------------------------------------
    if (this->ThreadNumber_>1)
        this->Log_->append<QString>({tr("Slicing file list"), "..."});
    auto vec_list=this->sliceVector_(this->inVector_, this->ThreadNumber_);
    // --------------------------------------------

    this->FileCounter_=0;
    int k=0;
    for(const auto& list: vec_list) {
        k++;

        Convert* conv=new Convert();
        this->vecConvert_.push_back(conv); // used to pause all threads

        conv->setExtension(ui->comboBox_conversionOut->currentText());
        conv->setInVector(list);
        conv->setInput(this->InDirPath_);
        conv->setOutput(this->OutDirPath_);
        conv->setBinPath(this->LOPath_);
        conv->setRebuildTree(this->RebuildFolderTree_);
        conv->setTmpFolderPrefix(this->tmpFolderPrefixConv_);

        connect(conv, &Convert::finished,
                this,
                [&](){
            if (this->FileCounter_==this->inVector_.size()) {
                ui->tabWidget->setTabEnabled(1, true);
                ui->comboBox_conversionIn->setEnabled(true);
                ui->comboBox_conversionOut->setEnabled(true);
                this->enableBrowsing_();
                this->enableStart_();
                ui->pushButton_Pause->setDisabled(true);
                ui->pushButton_Stop->setEnabled(false);
                this->TimerStateCheck_->start(this->freqStateCheck_);
                ui->label_RemainTime->stop();
            }
            else
                this->Log_->append<QString>({tr("File Counter and file vector size do not match"), "."}, Logger::Level::Error);

            this->blurWidgets(false, { ui->pushButton_Pause,
                                      ui->pushButton_quit,
                                      ui->label_Counter,
                                      ui->progressBar,
                                      ui->label_RemainTime });

            this->setAttribute(Qt::WA_TranslucentBackground,false);

        });

        connect(conv, &Convert::progressionValue,
                this, [&](int val, int idx){
                    this->FileCounter_+=1;
                    ui->lcdNumber->display(this->FileCounter_);
                    ui->progressBar->setValue(static_cast<int>(100.0*this->FileCounter_/this->inVector_.size()));                
                    ui->label_RemainTime->addValue(this->FileCounter_, idx+1); // idx+1 used for average over threads
                    ui->label_RemainTime->setAccomplished(this->FileCounter_);
                });

        connect(conv, &Convert::progressionString,
                this, [this](const QString& str, int idx) {
                    if (this->displayFileName_)
                        this->Log_->append<QString>({"[", QString::number(idx), "] ", tr("Processing"), ": ", str});
                });

        conv->start();
    }
}


void MainWindow::on_checkBox_RestoreFolderTree_stateChanged(int arg1) {
    if (arg1==2) {
        this->Log_->append<QString>({tr("Directory trees will be restored"), "."});
        this->RebuildFolderTree_=true;
    }
    else
        this->RebuildFolderTree_=false;
}

void MainWindow::on_pushButton_reset_clicked() {
    this->Log_->append<QString>({tr("Resetting configuration"), "..."});
    std::error_code ec;
    std::filesystem::remove(this->confLocation_, ec);

    this->InDirPath_.clear();
    this->OutDirPath_.clear();
    this->LogPath_="./";

    ui->lineEdit_dirIn->setText("");
    ui->lineEdit_dirOut->setText("");
    ui->lineEdit_LogPath->setText("./");
    ui->lineEdit_LDLIB->setText("");

    ui->lineEdit_dirIn->setState(2);
    ui->lineEdit_dirOut->setState(2);
    ui->lineEdit_LDLIB->setState(2);

    this->State_.Input=false;
    this->State_.Output=false;

    ui->checkBox_DispLOOutput->setChecked(true);
    ui->checkBox_RestoreFolderTree->setChecked(true);

    this->displayFileName_=true;
    this->RebuildFolderTree_=true;

    disableStart_();
    disableProgessBar_();

    this->resetRequest=true;
}

void MainWindow::on_spinBox_ThreadNumbers_valueChanged(int arg1) {
    qDebug() << "- " <<  " " << "on_spinBox_ThreadNumbers_valueChanged(arg1): " << arg1;

    this->Log_->append<std::string>({tr("Changing the number of threads to").toStdString(),  " ", std::to_string(arg1),"..."});
    this->ThreadNumber_=arg1;
}

void MainWindow::on_pushButton_Flag_clicked() {
    this->isEng_=!this->isEng_;

    qApp->removeTranslator(this->translator_);

    if (this->isEng_) {
        ui->pushButton_Flag->setIcon(this->iconFlagEn_);
        this->Log_->append<QString>({tr("Toggle language to"),  ": ",tr("English"),"."});
    }
    else {
        if (this->translator_->load(":/i18n/fr_FR")) {
            qApp->installTranslator(this->translator_);
            ui->pushButton_Flag->setIcon(this->iconFlagFr_);
            this->Log_->append<QString>({tr("Toggle language to"),  ": ",tr("French"),"."});
        }
        else this->Log_->append<QString>({tr("Cannot change language to"), ": ", tr("French"),"."}, Logger::Level::Warning);
    }
    ui->retranslateUi(this);
}

void MainWindow::on_checkBox_DispLOOutput_stateChanged(int arg1) {
     qDebug() << "- " <<  " " << "on_checkBox_DispFileNames_stateChanged(arg1): " << arg1;

     this->Log_->append<QString>({tr("Toggle file names displaying"),"."});

     this->displayFileName_=!this->displayFileName_;
}


void MainWindow::on_pushButton_Pause_clicked() {

    this->pauseConvert_=!this->pauseConvert_;
    if (this->pauseConvert_) ui->pushButton_Pause->setText(tr("Resume"));
    else ui->pushButton_Pause->setText(tr("Pause"));

    if (this->pauseConvert_) {
        ui->pushButton_Pause->setStyleSheet("color: red;");

        this->Log_->append<QString>({tr("Pause"), " ", QString::number(this->vecConvert_.size()), " ",tr("thread"), "."});
        for(auto &conv: this->vecConvert_)
            if (conv!=nullptr) conv->pause();
        ui->label_RemainTime->pause();
    }
    else {
        ui->pushButton_Pause->setStyleSheet(this->backupStyleSheet_[ui->pushButton_Pause->objectName()]);

        this->Log_->append<QString>({tr("Resume"), " ", QString::number(this->vecConvert_.size()), " ",tr("thread"), "."});
        for(auto &conv: this->vecConvert_)
            if (conv!=nullptr) conv->resume();
        ui->label_RemainTime->resume();
    }
}


void MainWindow::on_pushButton_Quit2_clicked() {
    this->close();
}


void MainWindow::on_pushButton_Stop_clicked() {

    for(auto &conv: this->vecConvert_) {
        if (!conv->isRunning()) return;

        qDebug() << "-\t" << "Stop and clean thread idx=" << conv->getIdx();

        conv->stop();
        while(conv->isRunning()) QThread::msleep(250);
        conv->resetIdx();
        delete conv;
    }

    ui->pushButton_Stop->setEnabled(false);
    ui->pushButton_Pause->setEnabled(false);
}

