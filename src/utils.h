#pragma once

#include <iostream>
#include <ostream>
#include <fstream>
#include <cstdio>

#include <random>

#include <memory>

#include <type_traits>

#include <map>

#include <initializer_list>
#include <type_traits>

#include <string>
#include <sstream>


#if __has_include(<QString>)
#define ENABLE_QT_STR
#include <QString>
#endif

#if __has_include(<QEvent>)
#define ENABLE_QT_EVENT
#include <QEvent>
#endif

#if __has_include(<QMetaEnum>)
#define ENABLE_QT_METAENUM
#include <QMetaEnum>
#endif


#include <vector>

namespace Utils {
    class ExtCom;
    class Message;
    struct TriState;
    template<typename K,typename T> class BackupVal;
}

class Utils::ExtCom {
public:

    typedef std::vector<std::string> ArgList;

    explicit ExtCom();
    virtual ~ExtCom();

    ExtCom* operator()();

    int exec();

    template<class T>
    void setArgument( std::initializer_list<T> list);

    bool setBinary(const std::string& bin);
    void setEnv(const std::string &env);

    std::string getCmd() const;

    const Message message() const;

private:
    std::string binary_{};
    ArgList commands_{};
    std::string results_{};
    int value_{};

    std::string env_{};

    std::stringstream ss_{};
};


class Utils::Message {
public:

    /**
     * @brief Construct a message.
     * @param msg
     * @param value
     * @param stream Specify the stream, e.g. cout, clog, cerr
     */
    explicit Message(std::string msg="",
                     int value=0,
                     std::ostream& stream=std::clog);

    Message* operator()();

    std::string message() const;
    int value() const;

    template<typename T> T convert() const;

    /**
     * @brief Display the message and the value
     */
    void print() const;

private:
    std::string msg_{};
    int value_{};
    std::ostream& stream_;
};


struct Utils::TriState {
public:
    TriState();

    void operator=(int val);

    int operator()() const;

    TriState& operator++();
    TriState operator++(int);

    void display();

private:
    int state_;
};


namespace Utils {

template<typename K, typename T>
class BackupVal {
public:
    explicit BackupVal();

    std::map<std::string,T> operator()() const;

    void setValue(const K& key, const T& val);
    void find(const K& key) const;
    void erase(const K& key);
    void clear();

private:
    std::map<K, T> data_{};

};


}


namespace Utils {

#if defined(ENABLE_QT_METAENUM) && defined(ENABLE_QT_EVENT)
QString eventToKey(const QEvent * ev);
#endif

}


namespace Utils {

void generateRandomFile(const std::string &dirname,
                        const std::string &basename,
                        const std::string &ext,
                        size_t N,
                        size_t filelength);

}

#include "utils.tpp"
