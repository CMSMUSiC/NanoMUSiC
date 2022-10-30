# Read data from NanoAODReader

```C++
// get NanoAODReader
auto nano_reader = NanoAODReader(*events_tree);


while (nano_reader.next()){                
    std::cout << nano_reader.getVal<UInt_t>("nMuon") << std::endl;
    std::cout << nano_reader.getVec<Float_t>("Muon_pt") << std::endl;
    std::cout << nano_reader.getVal<Bool_t>("HLT_Mu18_Mu9") << std::endl;
    }
```