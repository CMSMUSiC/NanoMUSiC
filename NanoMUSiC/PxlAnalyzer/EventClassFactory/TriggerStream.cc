#include "EventClassFactory/TriggerStream.hh"

// void getTrigger(set<string>& trigs, const pxl::Event* const event);
// void addEntry(const pxl::Event* const event);
//  pxl::EventView* RecEvtView = event->getObjectOwner().findObject< pxl::EventView >("Rec");
void TriggerStream::addEntry(const pxl::Event *const event)
{
    pxl::EventView *TrigEvtView = event->getObjectOwner().findObject<pxl::EventView>("Trig");
    string userRecord = TrigEvtView->getUserRecords().toString();
    int pos1 = 0;
    int pos2 = 0;
    while (userRecord.find("HLT", pos2) != string::npos)
    {
        pos1 = userRecord.find("HLT", pos2);
        pos2 = userRecord.find("\'", pos1);
        string trigger_name = userRecord.substr(pos1, pos2 - pos1);

        if (s_unique_trigs.find(trigger_name) != s_unique_trigs.end())
            ++s_unique_trigs[trigger_name];
        else
            s_unique_trigs.emplace(trigger_name, 1);
    }
}

void TriggerStream::writeUniqueTriggers()
{
    ofstream fout;
    fout.open("unique_triggers.txt");
    if (fout.is_open())
    {
        for (auto &iter : s_unique_trigs)
        {
            fout << iter.first << " " << iter.second << endl;
        }
    }
    else
    {
        cerr << "No File opened to write unique triggers!" << endl;
    }
}
