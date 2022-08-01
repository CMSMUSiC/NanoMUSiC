// template <typename T>
// T NanoAODReader::getVal(std::string valueName)
// {
//   return *(*(dynamic_cast<TTreeReaderValue<T> *>(fData[valueName].get())));
// }

// template <typename T>
// std::vector<T> NanoAODReader::getVec(std::string vectorName)
// {
//   auto array_temp_ = dynamic_cast<TTreeReaderArray<T> *>(fData[vectorName].get());
//   return std::vector<T>(array_temp_->begin(), array_temp_->end());
// }

// template <>
// std::vector<UInt_t> NanoAODReader::getVec<Bool_t>(std::string vectorName)
// {
//   auto array_temp_ = dynamic_cast<TTreeReaderArray<Bool_t> *>(fData[vectorName].get());
//   return std::vector<UInt_t>(array_temp_->begin(), array_temp_->end());
// }
