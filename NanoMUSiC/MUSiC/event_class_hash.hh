const std::map<unsigned int, std::string> event_class_objects = {
    {0, "muons"}, {1, "electrons"}, {2, "photons"}, {3, "taus"}, {4, "bjets"}, {5, "jets"}, {6, "met"}};

unsigned int get_event_class_hash(unsigned int n_muons, unsigned int n_electrons, unsigned int n_photons,
                                  unsigned int n_taus, unsigned int n_bjets, unsigned int n_jets, unsigned int n_met)
{
    if (n_muons >= 10)
    {
        n_muons = 9;
    }
    if (n_electrons >= 10)
    {
        n_electrons = 9;
    }
    if (n_photons >= 10)
    {
        n_electrons = 9;
    }
    if (n_taus >= 10)
    {
        n_taus = 9;
    }
    if (n_bjets >= 10)
    {
        n_bjets = 9;
    }
    if (n_jets >= 10)
    {
        n_jets = 9;
    }
    if (n_met > 1)
    {
        std::cerr << "ERROR: n_met > 1!!" << std::endl;
        exit(-1);
    }

    return static_cast<unsigned int>(pow(10, 0) * n_muons + pow(10, 1) * n_electrons + pow(10, 2) * n_photons +
                                     pow(10, 3) * n_taus + pow(10, 4) * n_bjets + pow(10, 5) * n_jets +
                                     pow(10, 6) * n_met);
}

void _collect_digits(std::vector<unsigned int> &digits, const unsigned long &num)
{
    if (num > 9)
    {
        _collect_digits(digits, num / 10);
    }
    digits.push_back(num % 10);
}

auto get_n_objects(unsigned int &event_class_hash)
{
    std::vector<unsigned int> digits;
    _collect_digits(digits, event_class_hash);
    std::reverse(digits.begin(), digits.end());

    std::map<std::string, unsigned int> mapped_multiplicities;

    for (unsigned int i = 0; i < event_class_objects.size(); i++)
    {
        if (event_class_objects.at(i) == "met" && digits.size() < 7)
        {
            mapped_multiplicities[event_class_objects.at(i)] = 0;
        }
        else
        {
            mapped_multiplicities[event_class_objects.at(i)] = digits[i];
        }
    }

    return mapped_multiplicities;
}