#include "utils.h"



namespace Utils {

ExtCom::ExtCom() { }

ExtCom::~ExtCom() { }

ExtCom* ExtCom::operator()() { return this; }


int ExtCom::exec() {
    std::stringstream ss{};

    ss << this->env_ << " ";
    ss << this->binary_;
    for(const auto& arg: this->commands_) ss << " " << arg ;

    std::array<char, 256> buffer{};
    std::unique_ptr<FILE, decltype(&pclose)>
            pipe(popen(ss.str().c_str(), "r"), pclose);

    if (!pipe) {
        this->value_=0;
        return false;
    }

    this->results_.clear();
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        this->results_ += buffer.data();

    this->value_=1;
    return true;
}



bool ExtCom::setBinary(const std::string& bin) {
    this->binary_=bin;
    return !bin.empty();
}

void ExtCom::setEnv(const std::string& env) {
    this->env_=env;
}

std::string ExtCom::getCmd() const {
    std::stringstream ss{};

    ss << this->binary_;
    for(const auto& arg: this->commands_) ss << " " << arg ;

    return ss.str();
}

const Message ExtCom::message() const {
    Message msg(this->results_, this->value_, std::clog);
    return msg;
}



}




namespace Utils {

Message::Message(std::string msg,
                                    int value,
                                    std::ostream& stream):
    msg_(msg)
  , value_(value)
  , stream_(stream) { }

Message* Message::operator()() { return this; }

std::string Message::message() const { return this->msg_;  }

int Message::value() const { return this->value_; }

template<typename T>
T Message::convert() const { return T{}; }

template<>
std::string Message::convert<std::string>() const { return std::to_string(this->value_); }

template<>
const char* Message::convert<const char*>() const {  return msg_.data(); }

#ifdef ENABLE_QT_STR
template<>
QString Message::convert<QString>() const {
    return QString::fromStdString(msg_);
}
#endif


void Message::print() const {
    this->stream_ << "- Message: " << std::endl
                  << this->msg_    << std::endl
                  << "[value: " << this->value_ << "]" << std::endl;
}


}



namespace Utils {

TriState::TriState(): state_(0) { }

void TriState::operator=(int val) {
    this->state_=(val>=0 && val<3) ? val : 0;
}

int TriState::operator()() const {
    return this->state_;
}


TriState& TriState::operator++() {
    this->state_+=this->state_>1 ? -2 : 1;
    return *this;
}

TriState TriState::operator++(int val) {
    TriState tmp_state{*this};
    ++*this;
    return tmp_state;
}

void TriState::display() {
   std::clog <<"state: " << this->state_ << std::endl;
}

}


#if defined(ENABLE_QT_STR) && defined(ENABLE_QT_EVENT)
namespace Utils {


QString eventToKey(const QEvent * ev) {
   QString str{};
    static int eventEnumIndex =
            QEvent::staticMetaObject.indexOfEnumerator("Type");

   if (ev)
      str=QEvent::staticMetaObject.enumerator(eventEnumIndex).valueToKey(ev->type());

   return str;
}


}
#endif


namespace Utils {

void generateRandomFile(const std::string &dirname,
                        const std::string &basename,
                        const std::string &ext,
                        size_t N,
                        size_t filelength) {

    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution nd(0, 9);

    for(int i=0; i<N; i++) {
        std::fstream flux(dirname+"/"+basename+std::to_string(i)+ext, std::ios::ate | std::ios::out);

        for(int j=0; j<filelength; j++) {
            flux << nd(engine);
            if ((j+1)%50==0) flux << std::endl;
        }
        flux.close();
    }

}




}


