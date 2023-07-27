#include "Tools.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "boost/algorithm/string.hpp"
#pragma GCC diagnostic pop

std::string Tools::musicAbsPath(std::string relPath)
{
    if (relPath.substr(0, 1) == "/")
        return relPath;
    std::string output;
    char *pPath = std::getenv("PXLANALYZER_BASE");
    if (pPath != NULL)
    {
        output = std::string(pPath) + "/" + relPath;
    }
    else
    {
        std::cout << "FATAL: PXLANALYZER_BASE not set!" << std::endl;
        output = "";
    }
    return output;
}

std::string Tools::removeComment(std::string line, char const commentChar)
{
    if (line.empty())
    {
        return line;
    }

    std::string::size_type pos = line.find_first_of(commentChar);
    if (pos != std::string::npos)
    {
        line.erase(pos);
    }

    boost::trim(line);

    return line;
}

std::string Tools::random_string(size_t length)
{
    auto randchar = []() -> char
    {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

std::map<int, std::string> Tools::pdg_id_type_map(bool useBJet)
{
    std::map<int, std::string> outMap = std::map<int, std::string>();
    outMap.emplace(11, "Ele");
    outMap.emplace(12, "Nu_ele");
    outMap.emplace(13, "Muon");
    outMap.emplace(14, "Nu_muon");
    outMap.emplace(15, "Tau");
    outMap.emplace(16, "Nu_tau");

    // treat all quarks as jets
    outMap.emplace(1, "Jet");
    outMap.emplace(2, "Jet");
    outMap.emplace(3, "Jet");
    outMap.emplace(4, "Jet");
    outMap.emplace(5, "b");
    outMap.emplace(6, "Top");
    outMap.emplace(7, "Jet");
    outMap.emplace(8, "Jet");
    // Bosons
    outMap.emplace(21, "Gluon");
    outMap.emplace(9, "Gluon");
    outMap.emplace(22, "Gamma");
    outMap.emplace(23, "Z");
    outMap.emplace(24, "W");
    outMap.emplace(25, "H");
    return outMap;
}

std::vector<std::string> Tools::getParticleTypeAbbreviations(bool isRec)
{
    std::vector<std::string> partList;
    partList.push_back("Ele");
    partList.push_back("Muon");
    partList.push_back("Tau");
    partList.push_back("Gamma");
    partList.push_back("FatJet");
    partList.push_back("Jet");
    partList.push_back("MET");

    if (!isRec)
    {
        // 3rd generation
        partList.push_back("Top");
        partList.push_back("b");
        // neutrinos
        partList.push_back("Nu_ele");
        partList.push_back("Nu_muon");
        partList.push_back("Nu_tau");
        // bosons
        partList.push_back("Gluon");
        partList.push_back("Z");
        partList.push_back("W");
        partList.push_back("H");
    }
    return partList;
}
