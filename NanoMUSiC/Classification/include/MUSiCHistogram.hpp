#include <cassert>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <optional>
#include <stdexcept>

#include "ROOT/RVec.hxx"
#include "TFile.h"
#include "TH1F.h"

using ROOT::RVec;

void check(bool condition, const std::string &message)
{
    if (!condition)
    {
        std::cerr << message << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

class MUSiCHistogram
{
    RVec<double> m_edges;
    RVec<double> m_counts;
    RVec<double> m_squared_weights;

  public:
    MUSiCHistogram(const RVec<double> &edges, const RVec<double> &counts, const RVec<double> &squared_weigths)
        : m_edges(edges),
          m_counts(counts),
          m_squared_weights(squared_weigths)
    {

        try
        {
            // Manually check the condition and throw if it fails
            check(m_edges.size() - 1 == m_counts.size(), "ERROR: Number of counts do not match with number of bins.");
            check(m_edges.size() - 1 == m_squared_weights.size(),
                  "ERROR: Number of counts weights do not match with number of bins.");
            check(std::is_sorted(m_edges.cbegin(), m_edges.cend()), "ERROR: Edges are not in ascending order.");
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "ERROR: Could not build MUSiCHistogram:" << e.what() << std::endl;
        }
    }

    auto size() const
    {
        return m_counts.size();
    }

    auto counts() const
    {
        return m_counts;
    }

    auto edges() const
    {
        return m_edges;
    }

    auto squared_weigths() const
    {
        return m_squared_weights;
    }

    auto errors() const
    {
        return ROOT::VecOps::sqrt(m_squared_weights);
    }

    auto add_inplace(const MUSiCHistogram &other)
    {
        check(size() == other.size(), "ERROR: Could not add histograms. They have incompatible edges.");
        auto good_edges = true;
        for (std::size_t i = 0; i < size(); i++)
        {
            if (m_edges[i] != other.edges()[i])
            {
                good_edges = false;
                break;
            }
        }

        check(good_edges, "ERROR: Could not add histograms. They have incompatible edges.");
        m_counts += other.counts();
        m_squared_weights += other.squared_weigths();
    }

    auto add_inplace_unsafe(const MUSiCHistogram &other)
    {
        m_counts += other.counts();
        m_squared_weights += other.squared_weigths();
    }

    static auto add(const MUSiCHistogram &h1, const MUSiCHistogram &h2)
    {
        auto hist = h1;
        hist.add_inplace(h2);
        return hist;
    }

    static auto add_unsafe(const MUSiCHistogram &h1, const MUSiCHistogram &h2)
    {

        auto hist = h1;
        hist.add_inplace_unsafe(h2);
        return hist;
    }

    auto scale_inplace(double scale)
    {
        m_counts *= scale;
        m_squared_weights *= std::pow(scale, 2);
    }

    auto widths() const
    {
        auto widths = RVec<double>{};
        widths.reserve(size());

        for (std::size_t i = 0; i < size(); i++)
        {
            widths.emplace_back(m_edges[i + 1] - m_edges[i]);
        }

        return widths;
    }

    auto scale_by_width_inplace(double scale)
    {
        auto this_widths = widths();
        m_counts *= scale / this_widths;
        m_squared_weights *= std::pow(scale, 2) / ROOT::VecOps::pow(this_widths, 2.);
    }

    template <typename TH1X = TH1F>
    auto to_root(const std::string &name) -> TH1X
    {
        auto histo = TH1X(name.c_str(), "", size() - 1, m_edges.data());
        histo.Sumw2();

        auto this_widths = widths();
        for (std::size_t i = 0; i < size(); i++)
        {
            auto bin_center = m_edges[i] + this_widths[i] / 2.;
            auto root_bin_idx = histo.FindBin(bin_center);
            histo.SetBinContent(root_bin_idx, histo.GetBinContent(root_bin_idx) + m_counts[i]);
            histo.SetBinError(root_bin_idx, std::sqrt(m_squared_weights[i]));
        }

        return histo;
    }

    template <typename TH1X = TH1F>
    static auto from_root(const TH1X &histo) -> MUSiCHistogram
    {
        auto n_bins = histo.GetNbinsX();

        RVec<double> edges;
        edges.reserve(n_bins);

        RVec<double> counts;
        counts.reserve(n_bins);

        RVec<double> squared_weights;
        squared_weights.reserve(n_bins);

        for (std::size_t i = 1; i <= histo.GetNbinsX(); i++)
        {
            edges.emplace_back(histo.GetBinLowEdge(i));
            counts.emplace_back(histo.GetBinContent(i));
            squared_weights.emplace_back(std::pow(histo.GetBinError(i), 2));
        }
        edges.emplace_back(histo.GetBinLowEdge(n_bins + 1));

        return MUSiCHistogram(edges, counts, squared_weights);
    }

    template <typename TH1X = TH1F>
    static auto from_root(TH1X *histo) -> MUSiCHistogram
    {
        return MUSiCHistogram::from_root(*histo);
    }

    // template <typename TH1X = TH1F>
    // static auto from_rootfile(const std::string &file_path, const std::string &name) -> MUSiCHistogram
    // {
    //     auto root_file = std::unique_ptr<TFile>(TFile::Open(file_path.c_str()));
    //     return MUSiCHistogram::from_root(root_file->Get<TH1X>(name.c_str()));
    // }

    auto print() const
    {
        std::cout << "Edges: " << m_edges << "\nCounts: " << m_counts << "\nErrors: " << errors() << std::endl;
    }
};

auto main() -> int
{
    auto bar = MUSiCHistogram({1, 2, 3}, {4, 5}, {5, 6});
    // auto baz = MUSiCHistogram({1, 2}, {4, 5}, {5, 6}); // should fail

    auto edges = RVec<double>{0., 7, 10.};
    auto h = TH1F("", "", edges.size() - 1, edges.data());
    h.Fill(0., 7.8);
    h.Fill(0., 9);
    h.Print("all");

    auto histo = MUSiCHistogram::from_root(&h);
    histo.print();

    histo.to_root("").Print("all");

    std::cout << "==============================================" << std::endl;

    h.Scale(2., "width");
    h.Print("all");

    histo.scale_by_width_inplace(2.);
    histo.print();

    return 0;
}
