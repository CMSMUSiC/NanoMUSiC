#include "NanoEC.hpp"
#include <cstdlib>

auto main() -> int
{

    auto event_classes_1 = NanoECCollection();
    fmt::print("Event class 1: {}\n", event_classes_1);

    for (unsigned long i = 0; i < 1000; i++)
    {
        // for (std::size_t j = 0; j < NanoECCollection::n_processes; j++)
        // {
        // fmt::print("i: {} \n", i);
        event_classes_1.fill(i, 2 * i, 2, 3, 4, i);
        // }
    }

    for (unsigned long i = 0; i < 1000; i++)
    {
        // for (std::size_t j = 0; j < NanoECCollection::n_processes; j++)
        // {
        // fmt::print("(Second round) i: {} \n", i);
        event_classes_1.fill(i, 2 * i, 2, 3, 4, i);
        // }
    }

    fmt::print("Event class 1: {}\n", event_classes_1);

    auto event_classes_2 = NanoECCollection();
    fmt::print("Event class 2: {}\n", event_classes_2);

    for (unsigned long i = 100; i < 300; i++)
    {
        // for (std::size_t j = 0; j < NanoECCollection::n_processes; j++)
        // {
        // fmt::print("i: {} \n", i);
        event_classes_2.fill(i, 2 * i, 2, 3, 4, i);
        // }
    }

    for (unsigned long i = 100; i < 350; i++)
    {
        // for (std::size_t j = 0; j < NanoECCollection::n_processes; j++)
        // {
        // fmt::print("(Second round) i: {} \n", i);
        event_classes_2.fill(i, 2 * i, 2, 3, 4, i);
        // }
    }

    fmt::print("Event class 2: {}\n", event_classes_2);

    event_classes_2.merge(event_classes_1);
    fmt::print("Event class: {}\n", event_classes_1);

    // for (auto &&[class_hash, nanoec_array] : event_classes.event_classes)
    // {
    //     for (auto &&nanoec : nanoec_array)
    //     {
    //         fmt::print("Class: {} - NanoEC: {}\n", class_hash, nanoec);
    //     }
    // }

    return EXIT_SUCCESS;
}
