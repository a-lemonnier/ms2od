#include "convert.h"



// ----------------------------------------------
// Convert --------------------------------------
// ----------------------------------------------


Convert::Convert(QObject *parent):
    QThread(parent),
    parent_(parent) {
    this->idx_=this->newIdx_();

    emit(newIdx(this->idx_));

    qDebug() << "- " <<  " " << "Convert::Convert(): idx=" << this->idx_;
}

Convert::~Convert() {
    this->resetIdx();
}

size_t Convert::getIdx() const {
    return this->idx_;
}

size_t Convert::getGlobalIdx() const {
    return *this->globalIdx_.get();
}


size_t Convert::newIdx_() {
    static std::shared_ptr<size_t> idx0{new size_t(0)};
    this->globalIdx_=idx0;
    return (*idx0)++;
}

void Convert::resetIdx() {
    *this->globalIdx_-= *this->globalIdx_ > 0 ? 1 : 0;
    qDebug() << "-\t" << "Convert::resetIdx_(): idx=" << this->idx_
             << " gidx=" << getGlobalIdx();
}


void Convert::run() {
    qDebug() << "- " <<  " " << "Convert::run()";

   if (this->rebuildTree_) this->makeTree_();

   std::string ext_dotless=this->extension_.toStdString();

   ext_dotless.erase(std::remove(ext_dotless.begin(), ext_dotless.end(), '.'), ext_dotless.end());

   std::vector<std::string> tmp_files{};

   int i=0;
   for(const auto &file: this->InVector_) {
       while(this->pause_)  QThread::msleep(250);

       if (this->stop_) {
            emit(this->progressionString(tr("Stop conversion"), this->getIdx()));
           break;
       }

       std::stringstream port{};

       // 10000 -> 59999
       port << getIdx()+1;
       port << std::setfill('0')<< std::setw(4) << (i)%9999;

       tmp_files.push_back("/tmp/"+this->tmpFilePrefix_+ port.str());

       Utils::ExtCom EC{};
       EC.setBinary(this->binPath_.toStdString());
       EC.setArgument({ // adjust port and process
           " -env:SingleAppInstance=\"false\" ",
           ("-env:UserInstallation=\"file:///tmp/"+this->tmpFilePrefix_+ port.str() +"\" ").c_str(),
           ("--accept=\"socket,host=localhost,port=" + port.str() + ";urp;\" ").c_str(),
            "--norestore",
           "--headless ",
//           "--nolockcheck",
           "--convert-to ",
           ext_dotless.c_str(),
          (" \""+file+"\"").c_str(),
           "--outdir ",
        this->rebuildTree_ ? ("\""+this->NewFolderTree_[i]+"\"").c_str() : ("\""+this->output_.toStdString()+"\"").c_str()
       });

       EC.setEnv("LD_LIBRARY_PATH=/usr/lib64/libreoffice/program");

       if (!EC.exec()) {
           std::stringstream ss{};
           ss << tr("Cannot convert file").toStdString()
              << " " << file << " :"
              << "\n\tCommand: "
              << EC.getCmd() << ".";

           emit(this->progressionString(QString::fromStdString(ss.str()), this->getIdx()));
           break;
       }

       auto res=EC.message().convert<QString>();

       i++;
       emit(this->progressionValue(i, this->getIdx()));
       emit(this->progressionPerCentValue(static_cast<int>(100.0*i/this->InVector_.size()), this->getIdx()));
       if (!res.isEmpty())
            emit(this->progressionString(res, this->getIdx()));
       else {
           std::cerr << "/!\\ " << EC.getCmd() << std::endl;
       }
   }

   // remove LO tmp files
   std::stringstream ss{};
   ss << "( idx=" << this->getIdx() << ", " << "id=" << std::hex << QThread::currentThreadId()  << " )";
   emit(this->progressionString(tr("Conversion done")+" "+QString::fromStdString(ss.str())+".", this->getIdx()));
   emit(this->progressionString(tr("Cleaning tmp files")+" "+QString::fromStdString(ss.str())+".", this->getIdx()));
   std::error_code ec{};
   for(const auto& file: tmp_files) {
    std::filesystem::remove_all(file, ec);
    if (ec)
        emit(this->progressionString(tr("Cannot delete tmp file: ")+QString::fromStdString(file+"."), this->getIdx()));
   }

}

void Convert::stop() {
    this->pause_=false;
    this->stop_=true;
}

void Convert::pause() {
    this->pause_=true;
}

void Convert::resume() {
    this->pause_=false;
}

void Convert::setExtension(const QString &ext) {
    this->extension_=ext;
}

void Convert::setOutput(const QString &output) {
    this->output_=output;
}

void Convert::setInput(const QString& input) {
    this->input_=input;
}

void Convert::setBinPath(const QString &binpath) {
    this->binPath_=binpath;
}

void Convert::setInVector(const std::vector<std::string> &invec) {
    this->InVector_=invec;
}

void Convert::setRebuildTree(bool b) {
    this->rebuildTree_=b;
}

void Convert::setTmpFolderPrefix(const std::string& prefix) {
    this->tmpFilePrefix_=prefix;
}

void Convert::makeTree_() {
    qDebug() << "- " <<  " " << "Convert::makeTree_()";

    emit(this->progressionString(tr("Building directory trees")+"...", this->getIdx()));

    std::unordered_set<std::string> set{};

    int i=0;
    for(const auto& file: this->InVector_) {
        i++;
        std::filesystem::path p(file);
        auto path=p.parent_path().string();

        path=path.substr(this->input_.toStdString().size());

        set.insert(path);

        std::string new_path=this->output_.toStdString()+path;

        this->NewFolderTree_.emplace_back(new_path);
     }

    i=0;
    for(const auto& folder: set) {
        i++;

        std::error_code ec;
        std::filesystem::create_directory(this->output_.toStdString()+folder, ec);

        emit(this->progressionString((tr("Creating")+": ")+QString::fromStdString(folder)+"("+QString::fromStdString(ec.message())+")", this->getIdx()));
    }
}












































