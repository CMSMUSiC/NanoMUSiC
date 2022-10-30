# Handle NanoObjects

```C++
if (true)
{
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "Muons: " << NanoObject::Repr(muons) << std::endl;
    std::cout << "Muons Pt: " << NanoObject::Repr(NanoObject::Pt(muons)) << std::endl;
    std::cout << "Muons Eta: " << NanoObject::Repr(NanoObject::Eta(muons)) << std::endl;
    std::cout << "Muons Phi: " << NanoObject::Repr(NanoObject::Phi(muons)) << std::endl;
    std::cout << "Muons Mass: " << NanoObject::Repr(NanoObject::Mass(muons)) << std::endl;
    std::cout << "Muons E: " << NanoObject::Repr(NanoObject::E(muons)) << std::endl;
    std::cout << "Muons dxy: " << NanoObject::Repr(NanoObject::GetFeature<Float_t>(muons, "dxy")) << std::endl;
    std::cout << "Muons charge: " << NanoObject::Repr(NanoObject::GetFeature<Int_t>(muons,"charge")) << std::endl;
    std::cout << "Muons indices: " << NanoObject::Repr(NanoObject::Indices(muons)) << std::endl;
    std::cout << "Muons GetByIndex: " << NanoObject::GetByIndex(muons, 0) << std::endl;
}

if (muons.size() > 1)
{
    std::cout << "Selected: " << muons[1] << std::endl;
}


auto _test_where = NanoObject::Where<float>(
    muons,
    [](const auto &muon) {
        if (muon.pt() > 2.0)
        {
            return true;
        }
        return false;
    },
    [](const auto &muon) { return 9999.0; }, [](const auto &muon) { return -9999.0; });

std::cout << "_test_where: " << NanoObject::Repr(_test_where) << std::endl;

auto muon_filter = NanoObject::BuildMask(muons, [](const auto &muon) {
    if (muon.pt() > 2.0)
    {
        return true;
    }
    return false;
});

std::cout << "Filter: " << NanoObject::Repr(muon_filter) << std::endl;

auto filtered_muons = NanoObject::Filter(muons, muon_filter);
auto filtered_muons = muons[NanoObject::Pt(muons) > 25];
std::cout << "Filtered: " << NanoObject::Repr(filtered_muons) << std::endl;
std::cout << "Filtered - features: " << NanoObject::Repr(NanoObject::Pt(filtered_muons)) <<
std::endl; std::cout << "Filtered - features: " << NanoObject::Repr(NanoObject::GetFeature<Int_t>(filtered_muons, "charge")) << std::endl;


auto met_filter = met.pt() > 35.0;
std::cout << "++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
std::cout << "MET: " << met << std::endl;
std::cout << "MET - features: " << met.get<float>("significance") << std::endl;
std::cout << "MET - features: " << met.get<float>("MetUnclustEnUpDeltaX") << std::endl;
std::cout << "MET - index: " << met.index() << std::endl;

met.set("abc", true);
std::cout << "MET - set features: " << met.get<bool>("abc") << std::endl;

try
{
    std::cout << "MET - index: " << met.get<float>("dadas") << std::endl;
}
catch (const std::runtime_error &e)
{
    std::cerr << e.what() << '\n';
}

std::cout << "Filter: " << met_filter << std::endl;
if (met_filter)
{
    std::cout << "Filtered: " << met << std::endl;
    std::cout << "Filtered - features: " << met.get<float>("significance") << std::endl;
    std::cout << "Filtered - features: " << met.get<float>("MetUnclustEnUpDeltaX") << std::endl;
}
```