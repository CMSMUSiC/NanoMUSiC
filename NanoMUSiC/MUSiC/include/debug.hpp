#ifndef DEBUG_HPP
#define DEBUG_HPP

#include "TH1.h"
#include <optional>

class Debugger
{
  public:
    unsigned long event_counter;
    TH1F h_pass;
    TH1F h_total;
    Debugger()
        : event_counter(0),
          h_pass(TH1F("h_pass", "h_pass", 10, 0., 500.)),
          h_total(TH1F("h_total", "h_total", 10, 0., 500.))
    {
    }
};
using debugger_t = std::optional<Debugger>;

inline auto make_debugger() -> std::optional<Debugger>
{
    return Debugger();
}

#endif // !DEBUG_HPP