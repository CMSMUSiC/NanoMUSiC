#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "TCanvas.h"
#include "TH1.h"
#include "TLegend.h"
#include <optional>

#include "fmt/format.h"

class Debugger
{
  public:
    unsigned long event_counter;
    TH1F h_pass;
    TH1F h_total;
    Debugger()
        : event_counter(0),
          h_pass(TH1F("h_pass", "h_pass", 10, 0., 13000.)),
          h_total(TH1F("h_total", "h_total", 10, 0., 13000.))
    {
    }

    inline auto dump() -> void
    {
        fmt::print("\nSaving DEBUG Histogram ...\n");
        auto c = TCanvas("c");
        c.SetLogx();
        c.SetLogy();
        // c.SetLogx();
        this->h_total.SetLineColor(kBlue);
        this->h_pass.SetLineColor(kRed);
        this->h_total.Draw("hist");
        this->h_pass.Draw("same ep1");

        auto legend = TLegend(0.1, .9, 0.3, 1.);
        legend.AddEntry(&(this->h_pass), "Pass", "lp");
        legend.AddEntry(&(this->h_total), "Total", "lp");
        legend.Draw();

        c.SaveAs("h_debug.png");
        this->h_pass.Print();
        this->h_total.Print();
        fmt::print("... done.\n");
    }

    inline auto fill(float value, bool pass) -> void
    {
        this->h_total.Fill(value);
        if (pass)
        {
            this->h_pass.Fill(value);
        }
    }
};
using debugger_t = std::optional<Debugger>;

inline auto make_debugger() -> std::optional<Debugger>
{
    return Debugger();
}

#endif // !DEBUG_HPP