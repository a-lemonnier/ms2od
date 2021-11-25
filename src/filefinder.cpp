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

    this->fileCounter_=0;
    fs::path path(this->path_.toStdString());

    for(auto file=
        fs::recursive_directory_iterator(
            path,
            fs::directory_options::skip_permission_denied);
        file != fs::recursive_directory_iterator();
        ++file) {

        if ((*file).path().extension().string()==this->extension_.toStdString()) {
            emit(updateCounter(this->fileCounter_++));
            this->filesVector_.emplace_back((*file).path());
        }
    }

    std::sort(this->filesVector_.begin(), this->filesVector_.end());

    emit(done(this->filesVector_));
    emit(updateCounter(this->fileCounter_));
}

bool FileFinder::setDirectory(const QString &dir) {
    qDebug() << "- " <<  " " << "FileFinder::setDirectory(dir): " << dir;

    // check existance
    this->path_=dir;
    return true;
}

bool FileFinder::setExtension(const QString &ext) {
    qDebug() << "- " <<  " " << "FileFinder::setExtension(ext): " << ext;

    // check dot presence
    this->extension_=ext;
    return true;
}

