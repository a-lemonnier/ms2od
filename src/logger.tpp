
template<typename T>
void Logger::append(T msg, Level lvl) {
    QString new_data{};

    this->setLvlStr_(lvl);

    if constexpr (std::is_same<T, QString>::value)     new_data+=msg;
    if constexpr (std::is_same<T, std::string>::value) new_data+=QString::fromStdString(msg);
    if constexpr (std::is_same<T, const char*>::value) new_data+=QString(msg);

    auto str=this->getCommentIdx_()+QString::fromStdString(this->prefix_)+new_data+QString::fromStdString(this->suffix_);
    this->data_.push_back(str);

    this->newContent=true;
}

template<typename T,
typename V, typename>
void Logger::append(T msg, V val, Logger::Level lvl) {
    this->setLvlStr_(lvl);

    std::stringstream ss{};
    QString new_data{};

    if constexpr (std::is_same<T, QString>::value)     new_data+=msg;
    if constexpr (std::is_same<T, std::string>::value) new_data+=QString::fromStdString(msg);
    if constexpr (std::is_same<T, const char*>::value) new_data+=QString(msg);

    ss << ": " << val;
    new_data+=QString::fromStdString(ss.str());

    auto str=this->getCommentIdx_()+QString::fromStdString(this->prefix_)+new_data+QString::fromStdString(this->suffix_);
    this->data_.push_back(str);

    this->newContent=true;
}


template<typename T>
void Logger::append(std::initializer_list<T> msgs, Logger::Level lvl) {
    this->setLvlStr_(lvl);

    std::stringstream ss{};
    QString new_data{};

    if constexpr (std::is_same<T, QString>::value)
        new_data=std::accumulate(msgs.begin(), msgs.end(), QString());
    if constexpr (std::is_same<T, std::string>::value)
        for(const auto &msg: msgs) new_data+=QString::fromStdString(msg);
        if constexpr (std::is_same<T, const char*>::value)
            for(const auto &msg: msgs) new_data+=QString(msg);

            auto str=this->getCommentIdx_()+QString::fromStdString(this->prefix_)+new_data+QString::fromStdString(this->suffix_);
        this->data_.push_back(str);

    this->newContent=true;
}
