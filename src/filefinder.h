#pragma once


#include <QPointer>

#include <QVector>
#include <QString>

#include <QThread>
#include <QTime>
#include <QTimer>

#include <QDebug>

#include "utils.h"
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


/**
 * @brief Find recursively all the files with the given extension and store paths into a std::vector<std::string> (InVector_)
 */
class FileFinder: public QThread {
    Q_OBJECT
public:
    explicit FileFinder(QObject *parent=nullptr);
    virtual ~FileFinder();

    void run();

    bool setDirectory(const QString &dir);

    /**
     * @brief setExtension
     * @param ext The extension with the dot (e.g. ".ext")
     * @return
     */
    bool setExtension(const QString &ext);

signals:
    void done(std::vector<std::string>);
    void updateCounter(int);

private:
    QObject *parent_;

    QString DirPath_{};
    QString Extension_{};

    std::vector<std::string> InVector_{};

    std::atomic<int> FileCounter_{};
};
