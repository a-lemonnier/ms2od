#include "filefinder.h"

// ----------------------------------------------
// FileFinder -----------------------------------
// ----------------------------------------------

FileFinder::FileFinder(QObject *parent):
    QThread(parent),
    parent_(parent) {
    qDebug() << "- " <<  " " << "FileFinder::FileFinder()";
    qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
}

FileFinder::~FileFinder() { }

void FileFinder::run() {
    qDebug() << "- " <<  " " << "FileFinder::run()";

    namespace fs=std::filesystem;

    this->FileCounter_=0;
    fs::path path(this->DirPath_.toStdString());

    for(auto file=
        fs::recursive_directory_iterator(
            path,
            fs::directory_options::skip_permission_denied);
        file != fs::recursive_directory_iterator();
        ++file) {

        if ((*file).path().extension().string()==this->Extension_.toStdString()) {
            emit(updateCounter(this->FileCounter_++));
            this->InVector_.emplace_back((*file).path());
        }
    }

    std::sort(this->InVector_.begin(), this->InVector_.end());

    emit(done(this->InVector_));
    emit(updateCounter(this->FileCounter_));
}

bool FileFinder::setDirectory(const QString &dir) {
    qDebug() << "- " <<  " " << "FileFinder::setDirectory(dir): " << dir;

    // check existance
    this->DirPath_=dir;
    return true;
}

bool FileFinder::setExtension(const QString &ext) {
    qDebug() << "- " <<  " " << "FileFinder::setExtension(ext): " << ext;

    // check dot presence
    this->Extension_=ext;
    return true;
}

