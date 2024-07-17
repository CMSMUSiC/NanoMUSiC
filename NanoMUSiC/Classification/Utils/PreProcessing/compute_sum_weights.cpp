#include <cstddef>
#include <optional>

#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

struct SumWeights
{
    double sum_genWeight;
    double sum_LHEWeight;
    long long raw_events;
    bool has_genWeight;
    bool has_LHEWeight_originalXWGTUP;

    SumWeights(double sum_genWeight,
               double sum_LHEWeight,
               long long raw_events,
               bool has_genWeight,
               bool has_LHEWeight_originalXWGTUP)
        : sum_genWeight(sum_genWeight),
          sum_LHEWeight(sum_LHEWeight),
          raw_events(raw_events),
          has_genWeight(has_genWeight),
          has_LHEWeight_originalXWGTUP(has_LHEWeight_originalXWGTUP)
    {
    }
};

struct Accumulator
{
    double weight;
    long long counter;

    Accumulator(double weight = 0., long long counter = 0)
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

auto process(const std::vector<std::string> &files) -> SumWeights
{
    ROOT::RDataFrame df("Events", files);

    auto colNames = df.GetColumnNames();
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

    auto aggregator = [](Accumulator acc, float w) -> Accumulator
    {
        return acc + Accumulator(w, 1);
    };

    auto merger = [](std::vector<Accumulator> &accumulators) -> void
    {
        for (std::size_t i = 1; i < accumulators.size(); ++i)
        {
            accumulators[0] += accumulators[i];
        }
    };

    auto aggregate = [&aggregator,
                      &merger](auto &df, bool has_genWeight, bool has_LHEWeight_originalXWGTUP) -> SumWeights
    {
        auto r_genWeight = df.Aggregate(aggregator, merger, "genWeight", Accumulator());
        auto r_LHEWeight_originalXWGTUP = df.Aggregate(aggregator, merger, "LHEWeight_originalXWGTUP", Accumulator());

        return SumWeights((*r_genWeight).weight,                //
                          (*r_LHEWeight_originalXWGTUP).weight, //
                          (*r_genWeight).counter,               //
                          has_genWeight,
                          has_LHEWeight_originalXWGTUP);
    };

    if (has_genWeight and has_LHEWeight_originalXWGTUP)
    {
        return aggregate(df, has_genWeight, has_LHEWeight_originalXWGTUP);
    }
    else if (has_genWeight and not(has_LHEWeight_originalXWGTUP))
    {
        auto modified_df = df.Define("LHEWeight_originalXWGTUP", "0.f");
        return aggregate(modified_df, has_genWeight, has_LHEWeight_originalXWGTUP);
    }
    else if (not(has_genWeight) and has_LHEWeight_originalXWGTUP)
    {
        auto modified_df = df.Define("genWeight", "0.f");
        return aggregate(modified_df, has_genWeight, has_LHEWeight_originalXWGTUP);
    }

    auto modified_df = df.Define("genWeight", "0.f").Define("LHEWeight_originalXWGTUP", "0.f");
    return aggregate(modified_df, has_genWeight, has_LHEWeight_originalXWGTUP);
}
