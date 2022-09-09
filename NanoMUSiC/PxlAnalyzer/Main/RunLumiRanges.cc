#include "RunLumiRanges.hh"

#include <fstream>
#include <sstream>

using namespace std;
using namespace lumi;

void LumiRanges::addRange(ID min, ID max)
{
    ranges.push_back(pair<ID, ID>(min, max));
}

bool LumiRanges::check(const ID LS) const
{
    // check for a range that contains the LS
    for (vector<range>::const_iterator r = ranges.begin(); r != ranges.end(); ++r)
    {
        if (r->first <= LS && r->second >= LS)
            return true;
    }

    // we found nothing
    return false;
}

RunLumiRanges::RunLumiRanges(const string &filename)
{
    // empty filename means accept all
    if (filename.empty())
    {
        empty = true;
        // nothing more to be done
        return;
    }
    else
    {
        empty = false;
    }

    // we just started, so the next check will be the first
    first = true;

    ifstream ifs(filename.c_str());
    // bail out if something is wrong with the stream
    if (!ifs.good())
        throw bad_config("Could not read run/lumi config: " + filename);

    // the config was successfully opened
    // if something goes wrong now, the config is wrong
    ifs.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);

    try
    {
        // read until we find the starting {
        char c = 0;
        while (c != '{')
            ifs >> c;

        while (!ifs.eof())
        {
            // read the next char, which must be a "
            ifs >> c;
            if (c != '"')
                throw bad_config(filename, c);

            ID run;
            // get the run ID
            ifs >> run;

            // read the next char, which must be a "
            ifs >> c;
            if (c != '"')
                throw bad_config(filename, c);

            // read the next char, which must be a :
            ifs >> c;
            if (c != ':')
                throw bad_config(filename, c);

            // read the next char, which must be a [
            ifs >> c;
            if (c != '[')
                throw bad_config(filename, c);

            // now we about to get ranges, so make an object
            LumiRanges lr;

            // loop to get the lumi ranges
            while (true)
            {
                // read the next char, which must be a [
                ifs >> c;
                if (c != '[')
                    throw bad_config(filename, c);

                // the next in the stream must be the lower section
                ID min;
                ifs >> min;

                // read the next char, which must be a ,
                ifs >> c;
                if (c != ',')
                    throw bad_config(filename, c);

                // now get the higher section
                ID max;
                ifs >> max;

                // we got two numbers, so add them
                lr.addRange(min, max);

                // read the next char, which must be a ]
                ifs >> c;
                if (c != ']')
                    throw bad_config(filename, c);

                // read the next char, which must be either a , or a ]
                ifs >> c;
                if (c == ']')
                    break; // this run is done
                else if (c != ',')
                    throw bad_config(filename, c);
            }

            // now store the ranges
            ranges[run] = lr;

            // read the next char, which must be either a , or a }
            ifs >> c;
            if (c == '}')
                break; // this config is done
            else if (c != ',')
                throw bad_config(filename, c);
        }
    }
    catch (ifstream::failure &e)
    {
        // something went wrong, let's check what exactly
        // we found an EOF before we expected it
        if (ifs.eof())
            throw bad_config(filename, "Unexpected EOF");
        // reading failed, but the stream itself is good
        if (ifs.fail() && !ifs.bad())
            throw bad_config(filename, "Read failed (probably an unexpected character)");
        // the stream is bad
        if (ifs.bad())
            throw bad_config("Error while reading: " + filename);
    }
}

bool RunLumiRanges::check(const ID run, const ID LS)
{
    if (empty)
    { // empty config means accept all
        last_state = true;
    }
    else if (last_run != run || first)
    {                  // we are either in a new run or in the first round
        first = false; // no matter if this was the first round, the next one will for sure not be the first

        // update the cache
        last_run = run;
        last_LS = LS;

        // look for the run
        last_range = ranges.find(run);

        if (last_range == ranges.end())
        {
            // the run was not found, hence reject it
            last_state = false;
        }
        else
        {
            // ok, we found that run, check the LS
            last_state = last_range->second.check(LS);
        }
    }
    else if (last_LS != LS)
    { // the run hasn't changed, but the LS
        // update the cache
        last_LS = LS;

        if (last_range == ranges.end())
        {
            // even with a new LS a rejected run won't be accepted
            last_state = false;
        }
        else
        {
            // new LS in an accepted run, hence check it and update the last state
            last_state = last_range->second.check(LS);
        }
    } // else {} //nothing to be done if nothing has changed

    return last_state;
}
