# How to access the class storage

The unique list of processed classes, per file is stored, as TObjString. This would give acces to it.

```C++
{
    TFile *_file0 = TFile::Open("test_mc_2017/"
                                "nano_music_DYJetsToLL_M-50_13TeV_AM_2017_"
                                "b271bb52f6a48b51c1f58e0b3cf91b1d526b33d99f160f35f005234aad8b21d1.root");

    auto l = (_file0->GetListOfKeys());
    std::cout << ((TObjString *)l->At(2))->GetString().View() << std::endl;
}
```