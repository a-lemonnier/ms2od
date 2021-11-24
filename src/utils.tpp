
namespace Utils {

// ExtCom Template ----------------------

   template<class T>
   void ExtCom::setArgument( std::initializer_list<T> list) {
       for(const auto &s: list) {
           this->commands_.emplace_back(s);
       }
   }

}


namespace Utils {

// BackUpVal Template --------------------
template<typename K, typename T>
BackupVal<K, T>::BackupVal() { }


template<typename K, typename T>
void BackupVal<K, T>::setValue(const K& key, const T& val) {
    this->data_[key]=val;
}

template<typename K, typename T>
void BackupVal<K, T>::clear() {

}


}
