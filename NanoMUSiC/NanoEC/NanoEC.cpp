#include "NanoEC.hpp"
#include <array>
#include <cstdlib>
#include <iostream>

#define PRINT(VAR) std::cout << VAR << std::endl

auto main() -> int
{

    // // std::array<std::tuple<xHisto<13000>, xHisto<13000>, xHisto<13000>>, 1000> a;
    // xHisto<13000> a;
    // a.fill(0.5, 23);
    // a.fill(0.5, 23);
    // a.fill(0.5, 23);
    // a.fill(0.5, 23);
    // fmt::print("Integal: {}\n", a.integral());

    // std::vector<xHisto<13000>> b(10000);
    // b.at(10).fill(0.5, 23);
    // b.at(10).fill(0.5, 23);
    // b.at(10).fill(0.5, 23);
    // b.at(10).fill(0.5, 23);
    // fmt::print("Integal: {}\n", b[10].integral());

    // for (auto &&x : a)
    // {
    //     auto [h1, h2, h3] = x;
    //     fmt::print("Integal: {}\n", h1.integral());
    // }

    // a.fill(1, 2, 3, 8);

    // PRINT(a);

    // auto event_classes_1 = NanoECCollection();
    // PRINT(event_classes_1);
    // event_classes_1.fill(1, 2, 3, 4, 5, 6);
    // PRINT(event_classes_1);

    // for (unsigned long i = 0; i < 10; i++)
    // {
    //     event_classes_1.fill(i, i, 2, 3, 4, i);
    // }

    auto event_classes_1 = NanoECCollection();
    PRINT("event_classes_1 - " << event_classes_1);

    for (unsigned long i = 0; i < 1000; i++)
    {
        event_classes_1.fill(i, std::to_string(i), 2, 3, 4, i + 1);
    }

    PRINT("event_classes_1 - " << event_classes_1);

    auto event_classes_2 = NanoECCollection();
    PRINT("event_classes_2 - " << event_classes_2);

    for (unsigned long i = 0; i < 500; i++)
    {
        event_classes_2.fill(i, std::to_string(2 * i + 1), 2, 3, 4, 10);
        // std::cout << i << " - " << std::to_string(2 * i + 1) << std::endl;
    }
    PRINT("event_classes_2 - " << event_classes_2);

    for (unsigned long i = 100; i < 350; i++)
    {
        event_classes_2.fill(i, std::to_string(2 * i), 2, 3, 4, i + 1);
    }

    PRINT("event_classes_2 - " << event_classes_2);

    event_classes_2.merge(event_classes_1);
    PRINT("Merged: " << event_classes_2);
    PRINT("Access [101]: " << event_classes_1.event_classes.at(50).size());
    for (auto &&[proc, ec] : event_classes_2.event_classes.at(50))
    {
        PRINT("proc");
        PRINT(proc);
        PRINT(ec);
    }
    PRINT("Access [101]: " << event_classes_2.event_classes.at(50).at("101"));
    // PRINT("Access [101]: " << event_classes_1.event_classes.at(50).at("101").mass.counts.at(1));
    // PRINT("Access [101]: " << event_classes_1.event_classes.at(50).at("101").mass.counts.at(2));
    // PRINT("Access [101]: " << event_classes_1.event_classes.at(50).at("101").mass.counts.at(3));
    // PRINT("Access [101]: " << event_classes_1.event_classes.at(50).at("101").mass.counts.at(4));
    // PRINT("Access [101]: " << event_classes_1.event_classes.at(50).at("101").mass.counts.at(5));
    // PRINT("Access [101]: " << event_classes_1.event_classes.at(50).at("101").mass.counts.at(6));
    // PRINT("Access [101]: " << event_classes_1.event_classes.at(50).at("101").mass.counts.at(6000));

    auto foo = NanoECCollection(true);
    foo.fill(1, "bar", 1, 1, 1, 1, true, 1, 1);
    foo.fill(1, "bar", 1, 1, 1, 1, true, 1, 2);
    foo.fill(1, "bar", 1, 1, 1, 1, true, 6, 2);
    foo.fill(1, "bar", 1, 1, 1, 1, true, 2, 2);
    foo.fill(1, "bar", 1, 1, 1, 1, true, 1, 6);
    PRINT(foo.event_classes.at(1).at("bar").mass.counts.at(1));

    return EXIT_SUCCESS;
}
