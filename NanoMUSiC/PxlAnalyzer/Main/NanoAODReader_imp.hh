template<typename T>
T NanoAODReader::getVal(std::string valueName)
{
  return *(*((TTreeReaderValue<T> *)(fData[valueName])));
}

template<typename T>
RVec<T> NanoAODReader::getRVec(std::string vectorName)
{
  auto array_temp_ = (TTreeReaderArray<T> *)(fData[vectorName]);
  return RVec<T>(std::vector<T>(array_temp_->begin(), array_temp_->end()));
}