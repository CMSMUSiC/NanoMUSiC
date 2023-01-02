#include <iostream>
// #include <stdio.h>

#include "TFile.h"
#include "TH1F.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

extern "C"
{
    void *get_file()
    {
        void *_file = new TFile("$ROOTSYS/tutorials/hsimple.root");
        return _file;
    }

    void ttree_reader(void *_file)
    // int main()
    {
        std::cout << "Starting ..." << std::endl;
        // TFile *myFile = TFile::Open("$ROOTSYS/tutorials/hsimple.root");
        TTreeReader myReader("ntuple", static_cast<TFile *>(_file));
        TTreeReaderValue<Float_t> myPx(myReader, "px");
        TTreeReaderValue<Float_t> myPy(myReader, "py");
        while (myReader.Next())
        {
            std::cout << "Value: " << *myPx + *myPy << std::endl;
        }
        std::cout << "Done ..." << std::endl;
        // return 0;
        // printf("ASDASDASDAS!\n");
    }
}