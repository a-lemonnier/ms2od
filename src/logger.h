#pragma once

#include <QWidget>
#include <QObject>

#include <QTextEdit>
#include <QWebEngineView>

#include <QString>
#include <QVector>

#include <QThread>
#include <QTimer>

#include <QTextCursor>

#include <QDebug>

#include <string>
#include <sstream>
#include <vector>
#include <type_traits>
#include <initializer_list>
#include <algorithm>

class Logger: public QThread {
    Q_OBJECT
public:
    enum class Level {
        Info = 0,
        Debug,
        Warning,
        Error,
        Fatal,
        count
    };

    explicit Logger(QObject *parent=nullptr);
    virtual ~Logger();

    Logger* operator()();

    void run();

    void stop();

    void setTextEdit(QTextEdit *te);
    void setWebEngineView(QWebEngineView *view);

    void setBackgroundColor(const std::string &color="#002d42");
    void setTextColor(const std::string &color="white");

    void setLevelThreshold(Level lvl);

    void enableTimeStamp(bool b=true);
    void disableTimeStamp(bool b=true);

    template<typename T>
    void append(T msg, Level lvl=Level::Info);

    template<typename T>
    void append(std::initializer_list<T> msgs, Level lvl=Level::Info);

    template<typename T,
    typename V,
    typename = typename std::enable_if<std::is_arithmetic<V>::value, V>::type>
    void append(T msg, V val, Level lvl=Level::Info);

    void pause(bool b=true);
    void resume(bool b=true);

signals:
    void done();
    void sendData(QString);
    void contentChanged();

private:
    QObject *parent_;

    QTextEdit* TE_{nullptr};
    QWebEngineView* webEngineView{nullptr};

    bool isWidgetDefined_=false;
    bool enableWebEngineView_{};

    QTimer * timer_;

    int loopPauseDuration_{500};

    size_t historyMax_{1000};
    size_t history_{0};

    bool stop_{false};
    bool pause_{false};

    bool showTimeStamp_{};

    Level levelMin_{};
    Level currentLvl_{};

    QVector<QString> data_{};

    std::string White_{"ffffff"};
    std::string Red_{"ff0055"};
    std::string Blue_{"0055ff"};
    std::string Black_{"000000"};
    std::string Orange_{"ff5500"};
    std::string Green_{"00ff55"};

    std::string prefix_{};
    std::string suffix_{"</span></div>"};

    std::string header_{};
    std::string footer_{};

    std::string textColor_{"white"};
    std::string backgroundColor_{"#002d42"};

    int currentVectorSize_{0};
    int previousVectorSize_{0};
    bool newContent_=false;

    void setLvlStr_(Level lvl);
    std::string getHeader() const;
    std::string getFooter() const;
    QString getCommentIdx_() const;

    void initWebEngineView_();

    void saveLog_(const QString& path);
};

#include "logger.tpp"
