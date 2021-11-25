#pragma once

#include <QMainWindow>
#include <QPointer>

#include <QVector>
#include <QString>

#include <QThread>
#include <QTime>
#include <QTimer>

#include <QDebug>

#include "lineeditcolored.h"
#include "blinkableCB.h"
#include "dockwidgetclosedetect.h"
#include "remainingtimelabel.h"

#include "utils.h"
#include "filefinder.h"
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
#include <memory>

/**
 * @brief The Convert class converts a filetype into another one using LibreOffice.
 */
class Convert: public QThread {
    Q_OBJECT
public:
    explicit Convert(QObject *parent=nullptr);
    virtual ~Convert();

    void run();

    void stop();
    void pause();
    void resume();

    void setExtension(const QString& ext);
    void setOutput(const QString& output);
    void setInput(const QString& input);
    void setBinPath(const QString& binpath);
    void setInVector(const std::vector<std::string> &invec);

    void setSharedCounter(int* counter);

    void setRebuildTree(bool b=true);

    void setTmpFolderPrefix(const std::string &prefix);

    size_t getIdx() const;
    size_t getGlobalIdx() const;
    void resetIdx();

protected:
    size_t idx_{};
    std::shared_ptr<size_t> globalIdx_;

signals:
    void done(int);
    void progressionPerCentValue(int, int);
    void progressionString(QString, int);
    void progressionValue(int, int);

    void newIdx(int);

private:
    QObject *parent_;

    QString extension_{};
    QString output_{};
    QString input_{};

    QString binPath_{};

    std::string tmpDirectoryPrefix_{"LO_Process"};

    int* sharedCounter_{};
    int progressionValue_{};
    QString progressionString_{};

    std::vector<std::string> filesVector_{};
    std::vector<std::string> newFolderTree_{};

    bool rebuildTree_=false;

    bool pause_=false;
    bool stop_=false;

    size_t newIdx_();

    void makeTree_();

};
