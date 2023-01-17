#include <iostream>
// #include <stdio.h>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"
#include "TFile.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

using namespace ROOT;
using namespace ROOT::VecOps;

extern "C"
{
    // std::vector<RVec<float>> *get_float_arrays(const char *tree_name, const char *file_path)
    // {
    //     auto _temp = new std::vector<RVec<float>>();
    //     auto df = RDataFrame(tree_name, std::vector<std::string>{file_path});
    //     *_temp = df.Take<RVec<float>>("Muon_pt").GetValue();

    //     return _temp;
    // }

    struct RVec_float
    {
        float *data;
        size_t len;
    };

    struct VecRVec_float
    {
        void *data;
        size_t len;
    };

    VecRVec_float *get_float_arrays(const char *tree_name, const char *file_path)
    // size_t get_float_arrays(const char *tree_name, const char *file_path)
    {
        auto _temp = new std::vector<RVec<float>>();
        auto df = RDataFrame(tree_name, std::vector<std::string>{file_path});
        *_temp = df.Take<RVec<float>>("Muon_pt").GetValue();

        auto _temp_res = new std::vector<RVec_float>(_temp->size());
        for (std::size_t i = 0; i < _temp->size(); i++)
        {
            (_temp_res->data())[i].data = (_temp->data())[i].data();
            (_temp_res->data())[i].len = (_temp->data())[i].size();
        }

        auto res = new VecRVec_float();
        res->data = _temp_res->data();
        res->len = _temp_res->size();

        // std::cout << "# res: [" << *static_cast<float *>(res->data) << " - " << *((static_cast<float *>(res->data)) + 1) << "]"
        //           << " \n# len: " << res->len << std::endl;
        return res;
    }

    TTreeReader *get_tree_reader(const char *tree_name, const char *file_path)
    {
        return new TTreeReader(tree_name, TFile::Open(file_path));
    }

    struct ValueReaders
    {
        TTreeReaderValue<Float_t> *px;
        TTreeReaderValue<Float_t> *py;
    };

    ValueReaders *get_value_readers(TTreeReader *_reader)
    {
        auto _value_readers = new ValueReaders{};
        _value_readers->px = new TTreeReaderValue<Float_t>(*_reader, "px");
        _value_readers->py = new TTreeReaderValue<Float_t>(*_reader, "py");
        return _value_readers;
    }

    bool next(TTreeReader *_reader)
    {
        return _reader->Next();
    }

    float get_value(TTreeReaderValue<Float_t> *value_reader)
    {
        return **value_reader;
    }

    void process_tree(TTreeReader *_reader)
    {
        std::cout << "Starting ..." << std::endl;
        // TFile *myFile = TFile::Open("$ROOTSYS/tutorials/hsimple.root");
        // TTreeReader myReader("ntuple", _file);
        TTreeReaderValue<Float_t> myPx(*_reader, "px");
        TTreeReaderValue<Float_t> myPy(*_reader, "py");
        double sum = 0.;
        while (_reader->Next())
        {
            // std::cout << "Value: " << *myPx + *myPy << std::endl;
            sum += *myPx + *myPy;
        }
        std::cout << "Done ... " << sum << std::endl;
    }
}