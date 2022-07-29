template <typename T>
T NanoAODReader::getVal(std::string valueName)
{
  return *(*(dynamic_cast<TTreeReaderValue<T> *>(fData[valueName].get())));
}

template <typename T>
RVec<T> NanoAODReader::getRVec(std::string vectorName)
{
  auto array_temp_ = dynamic_cast<TTreeReaderArray<T> *>(fData[vectorName].get());
  return RVec<T>(std::vector<T>(array_temp_->begin(), array_temp_->end()));
}