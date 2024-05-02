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
    bool has_genWeight;
    bool has_LHEWeight_originalXWGTUP;

    SumWeights(double sum_genWeight,
               double sum_genWeight_pass_generator_filter,
               double sum_LHEWeight,
               double sum_LHEWeight_pass_generator_filter,
               long long raw_events,
               long long pass_generator_filter,
               bool has_genWeight,
               bool has_LHEWeight_originalXWGTUP)
        : sum_genWeight(sum_genWeight),
          sum_genWeight_pass_generator_filter(sum_genWeight_pass_generator_filter),
          sum_LHEWeight(sum_LHEWeight),
          sum_LHEWeight_pass_generator_filter(sum_LHEWeight_pass_generator_filter),
          raw_events(raw_events),
          pass_generator_filter(pass_generator_filter),
          has_genWeight(has_genWeight),
          has_LHEWeight_originalXWGTUP(has_LHEWeight_originalXWGTUP)
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
    auto inflated_df =
        raw_df
            .Define("_LHEPart_pt",
                    [&raw_df]() -> std::string_view
                    {
                        return (raw_df.HasColumn("LHEPart_pt")) ? "LHEPart_pt"sv : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_eta",
                    [&raw_df]() -> std::string_view
                    {
                        return (raw_df.HasColumn("LHEPart_eta")) ? "LHEPart_eta"sv : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_phi",
                    [&raw_df]() -> std::string_view
                    {
                        return (raw_df.HasColumn("LHEPart_phi")) ? "LHEPart_phi"sv : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_mass",
                    [&raw_df]() -> std::string_view
                    {
                        return (raw_df.HasColumn("LHEPart_mass")) ? "LHEPart_mass"sv : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_incomingpz",
                    [&raw_df]() -> std::string_view
                    {
                        return (raw_df.HasColumn("LHEPart_incomingpz")) ? "LHEPart_incomingpz"sv
                                                                        : "ROOT::RVec<float>{}"sv;
                    }())
            .Define("_LHEPart_pdgId",
                    [&raw_df]() -> std::string_view
                    {
                        return (raw_df.HasColumn("LHEPart_pdgId")) ? "LHEPart_pdgId"sv : "ROOT::RVec<int>{}"sv;
                    }())
            .Define("_LHEPart_status",
                    [&raw_df]() -> std::string_view
                    {
                        return (raw_df.HasColumn("LHEPart_status")) ? "LHEPart_status"sv : "ROOT::RVec<int>{}"sv;
                    }());

    auto colNames = inflated_df.GetColumnNames();
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

    auto aggregate = [&aggregator, &merger, &generator_filter, &year_str](
                         auto &df, bool has_genWeight, bool has_LHEWeight_originalXWGTUP) -> SumWeights
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
                {"_LHEPart_pt",
                 "_LHEPart_eta",
                 "_LHEPart_phi",
                 "_LHEPart_mass",
                 "_LHEPart_incomingpz",
                 "_LHEPart_pdgId",
                 "_LHEPart_status",
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
                              (*r_genWeight_filtered).counter,
                              has_genWeight,
                              has_LHEWeight_originalXWGTUP);
        }
        return SumWeights((*r_genWeight).weight,                //
                          (*r_genWeight).weight,                //
                          (*r_LHEWeight_originalXWGTUP).weight, //
                          (*r_LHEWeight_originalXWGTUP).weight, //
                          (*r_genWeight).counter,               //
                          (*r_genWeight).counter,
                          has_genWeight,
                          has_LHEWeight_originalXWGTUP);
    };

    if (has_genWeight and has_LHEWeight_originalXWGTUP)
    {
        auto df = inflated_df;
        return aggregate(df, has_genWeight, has_LHEWeight_originalXWGTUP);
    }
    else if (has_genWeight and not(has_LHEWeight_originalXWGTUP))
    {
        auto df = inflated_df.Define("LHEWeight_originalXWGTUP", "0.f");
        return aggregate(df, has_genWeight, has_LHEWeight_originalXWGTUP);
    }
    else if (not(has_genWeight) and has_LHEWeight_originalXWGTUP)
    {
        auto df = inflated_df.Define("genWeight", "0.f");
        return aggregate(df, has_genWeight, has_LHEWeight_originalXWGTUP);
    }

    auto df = inflated_df.Define("genWeight", "0.f");
    df = df.Define("LHEWeight_originalXWGTUP", "0.f");
    return aggregate(df, has_genWeight, has_LHEWeight_originalXWGTUP);
}
