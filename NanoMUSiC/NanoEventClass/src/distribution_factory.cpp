
#include "distribution_factory.hpp"

auto make_distribution(const NanoEventClass &ec, const std::string &distribution_name) -> std::shared_ptr<Distribution>
{
    return std::make_shared<Distribution>(ec, distribution_name);
}

auto distribution_factory(NanoEventClassCollection &ec_collection, bool counts_only)
    -> std::vector<std::shared_ptr<Distribution>>
{
    auto pool = BS::thread_pool(100);

    // std::vector<std::string> all_distributions = {"counts", "sum_pt", "invariant_mass", "met"};
    std::vector<std::string> all_distributions = {"counts"};
    if (counts_only)
    {
        all_distributions = {"counts"};
    }

    std::vector<std::future<std::shared_ptr<Distribution>>> future_distributions;

    fmt::print("[Distribution Factory] Launching threads ...\n");
    for (auto &&ec_name : ec_collection.get_classes())
    {
        for (auto &&distribution_name : all_distributions)
        {
            if (not(distribution_name == "met" and ec_name.find("MET") == std::string::npos))
            {
                future_distributions.push_back(
                    pool.submit(make_distribution, ec_collection.get_class(ec_name), distribution_name));
            }
        }
    }

    fmt::print("[Distribution Factory] Waiting ...\n");
    for (auto &&fut : future_distributions)
    {
        fut.wait();
    }

    fmt::print("[Distribution Factory] Collecting results ... ");
    std::vector<std::shared_ptr<Distribution>> distributions;
    for (auto &&fut : future_distributions)
    {
        distributions.push_back(fut.get());
    }
    fmt::print("done.\n");

    return distributions;
}
