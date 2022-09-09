#include "SkipEvents.hh"

#include <iostream>
#include <utility> // make_pair

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#pragma GCC diagnostic pop

#include "Tools/MConfig.hh"
#include "Tools/Tools.hh"

SkipEvents::SkipEvents(Tools::MConfig const &cfg)
    : m_fileList(initFilePaths(cfg)), m_skipRunsLumisEvents(initRunsLumisEvents()), m_dontSkip(std::make_pair(0, 0))
{
}

SkipEvents::Paths SkipEvents::initFilePaths(Tools::MConfig const &cfg) const
{
    Paths filePaths;

    bool runOnData = cfg.GetItem<bool>("General.RunOnData");
    if (runOnData)
    {
        // More than one file allowed!
        std::string const files = cfg.GetItem<std::string>("SkipEvents.FileList");
        std::vector<std::string> const fileList = Tools::splitString<std::string>(files, true);

        std::vector<std::string>::const_iterator file;
        for (file = fileList.begin(); file != fileList.end(); ++file)
        {
            Path const filePath = Tools::AbsolutePath(*file);

            filePaths.push_back(filePath);
        }
    }
    return filePaths;
}

SkipEvents::RunsLumisEvents SkipEvents::initRunsLumisEvents() const
{
    RunsLumisEvents runsLumisEvents;

    Paths::const_iterator filePath;
    for (filePath = m_fileList.begin(); filePath != m_fileList.end(); ++filePath)
    {

        std::ifstream file((*filePath).string().c_str());
        if (not file.is_open())
            throw std::runtime_error("Failed to open file: '" + (*filePath).string() + "'");

        unsigned int lineNum = 0;
        while (file.good() and not file.eof())
        {
            std::string line;
            getline(file, line);
            lineNum++;

            if (line.empty())
                continue;

            line = Tools::removeComment(line);
            boost::trim(line);

            if (line.empty())
                continue;

            // Use boost to split by colon or space:
            std::vector<std::string> split;
            boost::split(split, line, boost::is_any_of(" :"), boost::token_compress_on);

            if (split.size() == 3)
            {
                // Run, LumiSections and Events are all given.
                unsigned int const run = boost::lexical_cast<unsigned int>(split.at(0));
                unsigned int const lumi = boost::lexical_cast<unsigned int>(split.at(1));
                unsigned int const event = boost::lexical_cast<unsigned int>(split.at(2));

                runsLumisEvents[run][lumi].insert(event);
            }
            else if (split.size() == 2)
            {
                // Only Run and LumiSections are given.
                unsigned int const run = boost::lexical_cast<unsigned int>(split.at(0));
                unsigned int const lumi = boost::lexical_cast<unsigned int>(split.at(1));

                runsLumisEvents[run][lumi] = Events();
            }
            else if (split.size() == 1)
            {
                // Only Runs are given.
                unsigned int const run = boost::lexical_cast<unsigned int>(split.at(0));

                runsLumisEvents[run] = LumiSections();
            }
            else
            {
                std::stringstream err;
                err << "In file '" << (*filePath).string() << "' " << std::endl;
                err << "malformatted line no. " << lineNum << ":" << std::endl;
                err << line << std::endl;
                throw std::runtime_error(err.str());
            }
        }
    }

    return runsLumisEvents;
}

bool SkipEvents::skip(unsigned int const runNumber, unsigned int const lumiSection, unsigned int const eventNumber)
{
    if (runNumber == m_dontSkip.first)
    {
        return false;
    }
    else
    {
        if (lumiSection == m_dontSkip.second)
            return false;
    }

    RunsLumisEvents::const_iterator run = m_skipRunsLumisEvents.find(runNumber);
    if (run == m_skipRunsLumisEvents.end())
    {
        // runNumber not in map (i.e. not in file), cache it so the next event
        // with the same runNumber is passed immediately.
        m_dontSkip.first = runNumber;
    }
    else
    {
        // If no lumiSection are specified the entire run is skipped.
        if ((*run).second.empty())
            return true;

        // Found the runNumber, so check the lumiSections.
        LumiSections::const_iterator lumi = (*run).second.find(lumiSection);

        if (lumi == (*run).second.end())
        {
            // lumiSection not in map (i.e. not in file), cache it so the next
            // event with the same lumiSection is passed immediately.
            m_dontSkip.second = lumiSection;
        }
        else
        {
            // If no events are specified for a given lumiSection the entire
            // lumiSection is skipped.
            if ((*lumi).second.empty())
                return true;

            // Found the lumiSection, so check the eventNumber.
            Events::const_iterator event = (*lumi).second.find(eventNumber);

            if (event != (*lumi).second.end())
                return true;
        }
    }

    return false;
}
