#include <iomanip>
#include <iostream>

#include <limits>

#include <cassert>
#include <cmath>

#include "ConvolutionLookup.hpp"

// Main function for table generation
int main(int argc, char *argv[])
{
    LookupOptions options;

    // PARAMS
    const double data_max = 70000;

    options.prior = NORMAL_PRIOR;

    options.data_log_block_start = 100;
    options.data_log_block_factor = 1.02;

    options.bg_factor_down = 1.005;
    options.bg_factor_up = 1.005;

    const double bg_min = 0.1;
    const double bg_max = 5.0;

    options.uncert_factor = 1.05;
    options.uncert_min = 0.01;
    const double uncert_max = 2.0;

    options.data_lower_block_size = 10;
    // END PARAMS

    options.bg_npoints_down = log(2. - bg_min) / log(options.bg_factor_down);
    options.bg_npoints_up = log(bg_max) / log(options.bg_factor_up);

    options.uncert_npoints = log(uncert_max / options.uncert_min) / log(options.uncert_factor);

    options.data_npoints = options.data_log_block_start +
                           log(data_max / options.data_log_block_start) / log(options.data_log_block_factor);

    LookupTable lookupTable(/* debug = */ true);
    lookupTable.generate(options);

    if (argc >= 2)
    {
        lookupTable.writeFile(argv[1]);
    }
    else
    {
        lookupTable.writeFile();
    }

    return 0;
}
