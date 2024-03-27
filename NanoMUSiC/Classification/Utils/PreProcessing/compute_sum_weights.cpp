#include "ROOT/RDataFrame.hxx"
#include <cstddef>
#include <optional>

#include "GeneratorFilters.hpp"
#include "ROOT/RVec.hxx"

struct SumWeights
{
    double sum_genWeight;
    double sum_genWeight_pass_generator_filter;
    double sum_LHEWeight;
    double sum_LHEWeight_pass_generator_filter;
    long long raw_events;
    long long pass_generator_filter;

    SumWeights(double sum_genWeight,
               double sum_genWeight_pass_generator_filter,
               double sum_LHEWeight,
               double sum_LHEWeight_pass_generator_filter,
               long long raw_events,
               long long pass_generator_filter)
        : sum_genWeight(sum_genWeight),
          sum_genWeight_pass_generator_filter(sum_genWeight_pass_generator_filter),
          sum_LHEWeight(sum_LHEWeight),
          sum_LHEWeight_pass_generator_filter(sum_LHEWeight_pass_generator_filter),
          raw_events(raw_events),
          pass_generator_filter(pass_generator_filter)
    {
    }
};

struct Accumulator
{
    float weight;
    long long counter;

    Accumulator(float weight = 0., long long counter = 0)
        : weight(weight),
          counter(counter)
    {
    }

    Accumulator operator+(const Accumulator &other) const
    {
        return Accumulator(other.weight + weight, other.counter + counter);
    }

    Accumulator &operator+=(const Accumulator &other)
    {
        weight += other.weight;
        counter += other.counter;
        return *this;
    }
};

auto process(const std::vector<std::string> &files,
             const std::string &year_str,
             const std::optional<std::string> &generator_filter) -> SumWeights
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame raw_df("Events", files);

    auto colNames = raw_df.GetColumnNames();
    auto has_genWeight = false;
    auto has_LHEWeight_originalXWGTUP = false;
    for (auto &&colName : colNames)
    {
        if (colName == "genWeight")
        {
            has_genWeight = true;
        };
        if (colName == "LHEWeight_originalXWGTUP")
        {
            has_LHEWeight_originalXWGTUP = true;
        };
    }

    auto aggregator = [](Accumulator acc, float x) -> Accumulator
    {
        return acc + Accumulator(x, 1);
    };

    auto merger = [](std::vector<Accumulator> &accumulators) -> void
    {
        for (std::size_t i = 1; i < accumulators.size(); ++i)
        {
            accumulators[0] += accumulators[i];
        }
    };

    auto aggregate = [&aggregator, &merger, &generator_filter, &year_str](auto &df) -> SumWeights
    {
        auto r_genWeight = df.Aggregate(aggregator, merger, "genWeight", Accumulator());
        auto r_LHEWeight_originalXWGTUP = df.Aggregate(aggregator, merger, "LHEWeight_originalXWGTUP", Accumulator());
        if (generator_filter)
        {
            auto filtered_df = df.Filter(
                [&generator_filter, &year_str](ROOT::RVec<float> LHEPart_pt,
                                               ROOT::RVec<float> LHEPart_eta,
                                               ROOT::RVec<float> LHEPart_phi,
                                               ROOT::RVec<float> LHEPart_mass,
                                               ROOT::RVec<float> LHEPart_incomingpz,
                                               ROOT::RVec<int> LHEPart_pdgId,
                                               ROOT::RVec<int> LHEPart_status,
                                               ROOT::RVec<float> GenPart_pt,
                                               ROOT::RVec<float> GenPart_eta,
                                               ROOT::RVec<float> GenPart_phi,
                                               ROOT::RVec<float> GenPart_mass,
                                               ROOT::RVec<int> GenPart_genPartIdxMother,
                                               ROOT::RVec<int> GenPart_pdgId,
                                               ROOT::RVec<int> GenPart_status,
                                               ROOT::RVec<int> GenPart_statusFlags) -> bool
                {
                    auto gen_filter_func = GeneratorFilters::get_filter(*generator_filter);
                    const auto lhe_particles = NanoAODGenInfo::LHEParticles(LHEPart_pt,
                                                                            LHEPart_eta,
                                                                            LHEPart_phi,
                                                                            LHEPart_mass,
                                                                            LHEPart_incomingpz,
                                                                            LHEPart_pdgId,
                                                                            LHEPart_status);
                    const auto gen_particles = NanoAODGenInfo::GenParticles(GenPart_pt,
                                                                            GenPart_eta,
                                                                            GenPart_phi,
                                                                            GenPart_mass,
                                                                            GenPart_genPartIdxMother,
                                                                            GenPart_pdgId,
                                                                            GenPart_status,
                                                                            GenPart_statusFlags);
                    auto year = get_runyear(year_str);
                    debugger_t debugger = std::nullopt;
                    return gen_filter_func(lhe_particles, gen_particles, year, debugger);
                },
                {"LHEPart_pt",
                 "LHEPart_eta",
                 "LHEPart_phi",
                 "LHEPart_mass",
                 "LHEPart_incomingpz",
                 "LHEPart_pdgId",
                 "LHEPart_status",
                 "GenPart_pt",
                 "GenPart_eta",
                 "GenPart_phi",
                 "GenPart_mass",
                 "GenPart_genPartIdxMother",
                 "GenPart_pdgId",
                 "GenPart_status",
                 "GenPart_statusFlags"});

            auto r_genWeight_filtered = filtered_df.Aggregate(aggregator, merger, "genWeight", Accumulator());
            auto r_LHEWeight_originalXWGTUP_filtered =
                filtered_df.Aggregate(aggregator, merger, "LHEWeight_originalXWGTUP", Accumulator());

            return SumWeights((*r_genWeight).weight,                         //
                              (*r_genWeight_filtered).weight,                //
                              (*r_LHEWeight_originalXWGTUP).weight,          //
                              (*r_LHEWeight_originalXWGTUP_filtered).weight, //
                              (*r_genWeight).counter,                        //
                              (*r_genWeight_filtered).counter);
        }
        return SumWeights((*r_genWeight).weight,                //
                          (*r_genWeight).weight,                //
                          (*r_LHEWeight_originalXWGTUP).weight, //
                          (*r_LHEWeight_originalXWGTUP).weight, //
                          (*r_genWeight).counter,               //
                          (*r_genWeight).counter);
    };

    if (has_genWeight and has_LHEWeight_originalXWGTUP)
    {
        auto df = raw_df;
        return aggregate(df);
    }
    else if (has_genWeight and not(has_LHEWeight_originalXWGTUP))
    {
        auto df = raw_df.Define("LHEWeight_originalXWGTUP", "0.f");
        return aggregate(df);
    }
    else if (not(has_genWeight) and has_LHEWeight_originalXWGTUP)
    {
        auto df = raw_df.Define("genWeight", "0.f");
        return aggregate(df);
    }

    auto df = raw_df.Define("genWeight", "0.f");
    df = df.Define("LHEWeight_originalXWGTUP", "0.f");
    return aggregate(df);
}
