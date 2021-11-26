#pragma once // MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QFileDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QTextCursor>
#include <QStyle>
#include <QLayout>

#include <QPointer>

#include <QVector>
#include <QString>

#include <QThread>
#include <QTime>
#include <QTimer>

#include <QPixmap>
#include <QIcon>
#include <QSize>
#include <QFontDatabase>

#include <QTranslator>
#include <QLocale>

#include <QWebEngineView>
#include <QWebEngineSettings>

#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsBlurEffect>

#include <QTextCursor>
#include <QScrollEvent>

#include <QtGlobal>
#include <QDebug>

#include "lineeditcolored.h"
#include "blinkableCB.h"
#include "dockwidgetclosedetect.h"
#include "remainingtimelabel.h"

#include "utils.h"
#include "filefinder.h"
#include "convert.h"
#include "logger.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unordered_set>
#include <string>
#include <sstream>
#include <map>
#include <atomic>
#include <memory>
#include <algorithm>
#include <cmath>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class Convert;
class FileFinder;
class Logger;


/**
 * @todo remove tmp files, fix lineedit In when manually changing the path (does not update files), fix plot axis
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:

    /**
     * @todo Use the result of loadConf tu update widgets !!!!
     */
    MainWindow(QTranslator* translator=nullptr,
               QWidget *parent = nullptr);
    ~MainWindow();

    /**
     * @brief Check the file existance and toggle the stylesheet as function of the result
     * @param path
     * @param le the QLineEdit for style application
     * @return true if file exists
     */
    bool browseFile(QString &path, LineEditColored &le);

    /**
     * @brief Check the directory existance and toggle the stylesheet as function of the result
     * @param path
     * @param le the QLineEdit for style application
     * @return
     */
    bool browseDir(QString &path, LineEditColored &le);

    /**
     * @brief Unhide the Log if closed.
     */
    void showLog();

private slots:
    void on_pushButton_LOLoc_clicked();
    void on_pushButton_browseIn_clicked();
    void on_pushButton_browseOut_clicked();
    void on_pushButton_Pause_clicked();
    void on_pushButton_Quit2_clicked();

    void on_lineEdit_dirIn_editingFinished();
    void on_lineEdit_dirOut_editingFinished();
    void on_lineEdit_LOLoc_editingFinished();
    void on_lineEdit_LogPath_editingFinished();

    void on_dockWidget_topLevelChanged(bool topLevel);
    void on_dockWidget_destroyed();
    void on_dockWidget_visibilityChanged(bool visible);

    void on_comboBox_conversionIn_currentTextChanged(const QString &index);

    void on_pushButton_quit_clicked();
    void on_pushButton_About_clicked();
    void on_pushButton_showLog_clicked();
    void on_pushButton_start_clicked();
    void on_pushButton_reset_clicked();
    void on_pushButton_Flag_clicked();
    void on_pushButton_LogLoc_clicked();

    void on_checkBox_RestoreFolderTree_stateChanged(int arg1);
    void on_checkBox_DispLOOutput_stateChanged(int arg1);

    void on_spinBox_ThreadNumbers_valueChanged(int arg1);

    void on_pushButton_LDLIB_clicked();

    void on_lineEdit_LDLIB_editingFinished();

    void on_pushButton_Stop_clicked();

private:
    Ui::MainWindow *ui;

    // States & Timers ----------------
    struct State {
        bool operator()();

        std::atomic<bool> Input=false;
        std::atomic<bool> Output=false;
        std::atomic<bool> Extensions=false;
        std::atomic<bool> Binary=false;
    };

    State state_{}; //!< Used to know when the program is able to start conversion

    QSize sizeLed_{16,16};
    QIcon iconLedRed_{":/icons/circle_red.svg"};
    QIcon iconLedGreen_{":/icons/circle_green.svg"};
    QPixmap pixmapLedRed_{":/icons/circle_red.svg"};
    QPixmap pixmapLedGreen_{":/icons/circle_green.svg"};

    int stateCheckFrequency_=1000; //!< Led update frequency (ms)
    QTimer* timerStateCheck_{};

    void updateStateLed_();
    // --------------------------------

    // ComboBox -----------------------
    bool extFirstTimeSelected_=false;
    void updateComboBoxConv_();
    int threadNumber_{1};
    // --------------------------------

    // File Counter -------------------
    std::atomic<int> fileCounter_{};
    QTimer* timerUpdateCounter_{};
    void startCounter_();
    void stopCounter_();
    void updateCounter_(); //!< This one feed the timer
    // --------------------------------

    // Location -----------------------
    QString pathInput_{};
    QString pathOutput_{};
    QString pathLOBinary_="/usr/bin/libreoffice";
    void updatePathInput_();
    void updatePathOuput_();
    void updatePathLOBinary_();
    bool updateDirPath_(QString &path, LineEditColored *le);
    // --------------------------------

    // File List ----------------------
    bool displayLOOutput_=false; //!< Display filename in Log
    std::vector<std::string> pathsVector_{}; //!< List of to be conv files
    bool rebuildFolderTree_=false; //!< Copy the existing directory.
    template<typename T>
    std::vector<std::vector<T>> sliceVector_(const std::vector <T>& vec, int n=1);
    // --------------------------------

    // Conversion ---------------------
    const std::string LOTmpDirectoryPrefix_{"LO_Process"};
    bool pauseConversion_=false;
    std::vector<Convert*> convertThreadVector_;
    // --------------------------------

    // Misc. Init. -------------------
    static const std::map<QString, QVector<QString>> allowedExtensions_;

    void initLeds_();
    void initComboBox_();
    // --------------------------------

    // Log ----------------------------
    QString pathLogFile_{"./"};
    QFont logFont_{"DejaVu Sans Mono"};
    QPointer<Logger> Log_;
    QPointer<QWebEngineView> webEngineView_;
    void initWebEngineView_();
    // --------------------------------

    // Enabler / Disabler -------------
    void enableBrowsing_(bool b=true);
    void disableBrowsing_(bool b=true);

    void enableStart_(bool b=true);
    void disableStart_(bool b=true);

    void enableProgessBar_(bool b=true);
    void disableProgessBar_(bool b=true);

    void enableLcdNumber_(bool b=true);
    void disableLcdNumber_(bool b=true);

    void blurWidgets(bool b=true, std::vector<QWidget*> exclusion_vector=std::vector<QWidget*>());
    // --------------------------------

    // Save / Restore -----------------
    bool configAvailable_=false;
    bool resetRequested_=false;
    std::string pathConfigFile_="/tmp/ms2od.conf";
    bool saveConfig_(const std::string &path);
    bool loadConfig_(const std::string &path);
    // --------------------------------

    // Localization -------------------
    enum Lang { FR, EN, count };
    Lang selectedLang_{};

    QIcon iconFlagFr_{":/icons/fr.svg"};
    QIcon iconFlagEn_{":/icons/eng.svg"};

    bool isEng_{};

    QTranslator* translator_{};
    // --------------------------------

    // Events -------------------------
    void changeEvent(QEvent *event);
    // --------------------------------

    // G Effect -----------------------
    std::vector<std::tuple<QWidget*, QGraphicsEffect*>> effectsList_{};

    void blurWidget(QWidget* w, bool enable=true, qreal radius=2.0, qreal duration=0.0);
    // --------------------------------


    // Misc. --------------------------
    std::map<QString, QString> backupStyleSheet_{}; //!< Store stylesheet using objectName()
    std::map<QString, QPalette> backupPalette_{};
    void storeStyleSheet_();
    // --------------------------------

};

