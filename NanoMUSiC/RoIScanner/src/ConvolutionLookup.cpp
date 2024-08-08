#include "ConvolutionLookup.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>

#include <libgen.h> // for dirname
#include <unistd.h> // for readlink

#include <fstream>
#include <iostream>

LookupTable::LookupTable(const bool debug)
    : debug(debug)
{
}

LookupTable::~LookupTable()
{
    deleteTable();
}

bool LookupTable::isLoaded() const
{
    return (buffer != nullptr);
}

void LookupTable::generate(const LookupOptions &new_options)
{
    if (buffer != nullptr)
    {
        // delete existing table using the old options
        deleteTable();
    }

    options = new_options;

    if (debug)
    {
        const double size_megabytes = expectedBinarySize() / 1e6;
        std::cerr << "Generating " << totalDataPoints() << " x " << totalBgPoints() << " x " << totalUncertPoints()
                  << " table. Expected binary size: " << round(size_megabytes * 10) / 10 << " MB" << std::endl;
    }

    // construct and initialize result object
    newTable(/* default_value = */ UNINITIALIZED);

    // create pool of threads
    BS::thread_pool pool(120);
    std::vector<std::future<double>> futures_buffer(totalPoints());

    // Hide cursor
    indicators::show_console_cursor(false);

    indicators::ProgressBar bar{
        indicators::option::BarWidth{51},
        indicators::option::Start{"["},
        indicators::option::Fill{"■"},
        indicators::option::Lead{"■"},
        indicators::option::Remainder{"-"},
        indicators::option::End{" ]"},
        indicators::option::PostfixText{"[ LUT generation - Launching tasks ]"},
        indicators::option::ForegroundColor{indicators::Color::cyan},
        indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

    //   ==========  here we fill table
    for (size_t data_index = 0; data_index < totalDataPoints(); data_index++)
    {
        for (size_t bg_index = 0; bg_index < totalBgPoints(); bg_index++)
        {
            for (size_t uncert_index = 0; uncert_index < totalUncertPoints(); uncert_index++)
            {
                futures_buffer[index(data_index, bg_index, uncert_index)] = pool.submit(
                    [&]()
                    {
                        double data, bg, uncert;
                        calcPoints(data_index, bg_index, uncert_index, data, bg, uncert);
                        const double p = compute_p_convolution(data, bg, uncert, prior(), /* debug = */ 0);
                        return p;
                    });
            }
        }

        const double progress = (double)data_index / totalDataPoints();
        if (debug)
        {
            bar.set_progress(100. * progress);
        }
    }

    bar.set_progress(100); // all done

    // Show cursor
    indicators::show_console_cursor(true);

    // wait for tasks to finish
    std::cout << termcolor::bold << termcolor::yellow << "[LUT generation] waiting for tasks to finish ... "
              << termcolor::reset << std::endl;
    pool.wait_for_tasks();
    std::cout << termcolor::bold << termcolor::green << "Done!\n" << termcolor::reset;

    // get results
    std::cout << termcolor::bold << termcolor::yellow << "[LUT generation] getting results and saving into buffer ... "
              << termcolor::reset << std::endl;
    for (size_t i = 0; i < futures_buffer.size(); i++)
    {
        buffer[i] = (futures_buffer[i]).get();
    }
    std::cout << termcolor::bold << termcolor::green << "Done!\n" << termcolor::reset;
}

double LookupTable::lookup(const double data, const double bg, const double uncert) const
{
    if (buffer == nullptr)
    {
        std::cerr << "No lookup table loaded." << std::endl;
        return UNINITIALIZED;
    }

    if (data == 0 && bg <= 0.02 && uncert <= 0.02)
    {
        return 0.97;
    }

    double data_index, bg_index, uncert_index;

    // data_index, bg_index and uncert_index are passed as reference
    // and modified by the function!
    calcIndices(data, bg, uncert, data_index, bg_index, uncert_index);

    if (data_index == options.data_log_block_start)
    {
        return BETWEEN_REGIONS;
    }

    if (debug)
    {
        std::cerr << "Looking up ( data=" << data << ", bg=" << bg << ", uncert=" << uncert << " ) at "
                  << "[ " << data_index << ", " << bg_index << ", " << uncert_index << " ]... " << std::endl;
    }

    const double p = interpolate(data_index, bg_index, uncert_index);

    if (p < 0.005)
    {
        if (debug)
        {
            std::cerr << "Miss reason: p-value too small." << std::endl;
        }
        return UNTRUSTED_P;
    }

    return p;
}

void LookupTable::writeFile(std::string filename)
{
    if (filename.empty())
    {
        filename = getDefaultLutPath();
        if (debug)
        {
            std::cerr << "Falling back to default file path " << filename << "." << std::endl;
        }
    }

    std::ofstream file(filename, std::ios::binary | std::ios::out);
    if (!file.good())
    {
        std::cerr << "Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    // store some meta info for sanity checking
    options._count = totalPoints();
    options._sizeof_float = sizeof(float);
    options._version = TABLE_VERSION;

    // write the options struct
    file.write(reinterpret_cast<char *>(&options), sizeof(options) / sizeof(char));

    // write the table data
    file.write(reinterpret_cast<char *>(buffer), totalPoints() * sizeof(float) / sizeof(char));

    file.close();
}

void LookupTable::readFile(std::string filename)
{
    if (filename.empty())
    {
        filename = getDefaultLutPath();
        // if (debug)
        // {
        //     std::cerr << "Falling back to default file path " << filename << "." << std::endl;
        // }
    }

    // std::cout << "LUT path: " << filename << std::endl;

    std::ifstream file(filename, std::ios::binary | std::ios::in);
    if (!file.good())
    {
        std::cerr << "Could not open file " << filename << " for reading." << std::endl;
        exit(1);
        return;
    }

    lastFile = filename;

    // delete existing table
    deleteTable();

    // read new options
    file.read(reinterpret_cast<char *>(&options), sizeof(options) / sizeof(char));

    // check table version
    assert(options._version == TABLE_VERSION);

    // if this assertion fails, the binary has been created for a different architecture
    assert(sizeof(float) == options._sizeof_float);

    // if this assertion fails, we probably have changed code in between
    assert(totalPoints() == options._count);

    // allocate new table
    newTable(0);

    // read table contents directly from file
    file.read(reinterpret_cast<char *>(buffer), totalPoints() * sizeof(float) / sizeof(char));

    // done
    file.close();
}

size_t LookupTable::expectedBinarySize() const
{
    const size_t header_bytes = sizeof(options) / sizeof(char);
    const size_t buffer_bytes = totalPoints() * sizeof(float) / sizeof(char);
    return header_bytes + buffer_bytes;
}

void LookupTable::printPoints() const
{
    for (size_t data_index = 0; data_index < totalDataPoints(); data_index++)
    {
        for (size_t bg_index = 0; bg_index < totalBgPoints(); bg_index++)
        {
            const size_t uncert_index = 0;
            double data, bg, uncert;
            calcPoints(data_index, bg_index, uncert_index, data, bg, uncert);

            std::cout << data << " " << bg << std::endl;
        }
    }
}

std::string LookupTable::getDefaultLutPath()
{
    char buffer[1024];

    // read current executable path from /proc/self
    // here, the buffer size is understated by 1 so we can still null-terminate it
    size_t bytes_read = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);

    // null-terminate buffer
    buffer[bytes_read] = '\0';

    // call dirname to obtain the binary directory
    // according to the documentation, we must not free() the result
    char *directory_cstr = dirname(buffer);

    // convert to strings for concatenation and return
    const std::string directory(directory_cstr);
    const std::string default_name("lookuptable.bin");

    return directory + "/" + default_name;
}

std::string LookupTable::lastLoadedFilename() const
{
    return lastFile;
}

void LookupTable::calcPoints(const double data_index,
                             const double bg_index,
                             const double uncert_index,
                             double &data_out,
                             double &bg_out,
                             double &uncert_out) const
{
    // Calculate (fractional) coordinates of points from indices (inverse of calcIndices)

    // Through the data-coordinate, the table is split into two parts: linear and exponential (in data)
    // The point of transition can be configured using options.data_log_block_start.
    // A decent value for data_log_block_start is 200.
    if (data_index < options.data_log_block_start)
    {
        // In the linear part, each index corresponds to one exact data value:
        // x = n
        data_out = data_index;
    }
    else
    {
        // In the logarithmic part, the point is calculated as (example numbers):
        // x = options.data_log_block_start * 1.05^n
        // where n is a number starting from 0 (n = relative_index)
        const unsigned int relative_index = data_index - options.data_log_block_start;
        data_out = options.data_log_block_start * pow(options.data_log_block_factor, relative_index);
    }

    // Through the data-coordinate, the BG-table is also split into a two parts:
    // linear and calcFactor-ish part
    if (data_out < options.data_lower_block_size)
    {
        // In the linear part, BG is always interpolated between 0 and 2*data_lower_block_size
        // independently of the data!
        const double bg_step = options.data_lower_block_size / totalBgPoints();
        bg_out = 2. * bg_index * bg_step;
    }
    else
    {
        // In the second part, we use a factor that is most densely spaced at 1
        // (see calcFactor) and multiply it onto the data value
        // Relative index goes between -bg_npoints_down and +bg_npoints_up
        const int relative_index = bg_index - options.bg_npoints_down;
        const double bg_factor = calcFactor(relative_index, options.bg_factor_down, options.bg_factor_up);

        // For 0 data, bg_out == 0 would not make sense, so we take the factor directly
        bg_out = (data_out == 0) ? bg_factor : bg_factor * data_out;
    }

    // The relative uncertainty is plain exponential, e.g.: 1.01^n
    const double uncert_multiplier = std::pow(options.uncert_factor, uncert_index);
    const double relative_uncert = options.uncert_min * uncert_multiplier;

    // uncert_out is now the absolute uncert!
    uncert_out = bg_out * relative_uncert;
}

void LookupTable::calcIndices(const double data,
                              const double bg,
                              const double uncert,
                              double &data_index_out,
                              double &bg_index_out,
                              double &uncert_index_out) const
{

    // Inverse of calcPoints, read its comments to better understand calcIndices (this function)

    // Separation between data-linear and data-exponential part
    if (data < options.data_log_block_start)
    {
        data_index_out = data;
    }
    else
    {
        const unsigned int relative_index =
            log(data / options.data_log_block_start) / log(options.data_log_block_factor);
        data_index_out = relative_index + options.data_log_block_start;
    }

    // Separation between BG-(constant-)linear and BG-relative-calcFactor-ish part
    if (data < options.data_lower_block_size)
    {
        const double bg_step = options.data_lower_block_size / totalBgPoints();
        bg_index_out = bg / (2. * bg_step);
    }
    else
    {
        const double bg_factor = (data == 0) ? bg : bg / data;
        bg_index_out = unCalcFactor(bg_factor, options.bg_factor_down, options.bg_factor_up) + options.bg_npoints_down;
    }

    // Inverse of exponential uncertainty
    const double relative_uncert = uncert / bg;

    const double uncert_multiplier = relative_uncert / options.uncert_min;
    uncert_index_out = std::log(uncert_multiplier) / std::log(options.uncert_factor);
}

double LookupTable::interpolate(const double data_index, const double bg_index, const double uncert_index) const
{
    // interpolate in the bg_index, uncert_index plane
    // data_index will be casted to int and always hits exactly

    // split indices into an integer value (j,k) and a fractional remainder
    // (bg_weight, uncert_weight) using modf
    double bg_int_index;
    const double bg_weight = modf(bg_index, &bg_int_index);

    double uncert_int_index;
    const double uncert_weight = modf(uncert_index, &uncert_int_index);

    const int i = static_cast<int>(data_index);
    const int j = static_cast<int>(bg_int_index);
    const int k = static_cast<int>(uncert_int_index);

    // Check for out-of-bounds errors
    if ((i < 0) or ((i + 1) >= static_cast<int>(totalDataPoints())))
    {
        // data out of lookup bounds
        if (debug)
        {
            std::cerr << "Miss reason: Data out of bounds." << std::endl;
        }
        return OUT_OF_BOUNDS;
    }

    // require that all 4 points are within the lookup table
    // thus: bg_index + 1 and uncert_index + 1
    if ((j < 0) or ((j + 1) >= static_cast<int>(totalBgPoints())))
    {
        // bg out of lookup bounds
        if (debug)
        {
            std::cerr << "Miss reason: BG out of bounds." << std::endl;
        }
        return OUT_OF_BOUNDS;
    }

    if ((k < 0) or ((k + 1) >= static_cast<int>(totalUncertPoints())))
    {
        // uncert out of lookup bounds
        if (debug)
        {
            std::cerr << "Miss reason: Uncert out of bounds." << std::endl;
        }
        return OUT_OF_BOUNDS;
    }

    // Read all required values from the buffer
    // buffer is 1D, so we use the member index(i,j,k) to map 3D to 1D
    const double val_ijk = buffer[index(i, j, k)];
    const double val_iJk = buffer[index(i, j + 1, k)];
    const double val_ijK = buffer[index(i, j, k + 1)];
    const double val_iJK = buffer[index(i, j + 1, k + 1)];

    // Check whether any of the extracted values is invalid (negative)
    if ((val_ijk < 0) || (val_iJk < 0) || (val_ijK < 0) || (val_iJK < 0))
    {
        if (debug)
        {
            std::cerr << "Miss reason: Invalid table value." << std::endl;
        }
        return INVALID_P;
    }

    // Calculate the final value using bilinear interpolation
    // See https://en.wikipedia.org/wiki/Bilinear_interpolation#Unit_Square
    const double value = (1. - bg_weight) * (val_ijk * (1. - uncert_weight) + val_ijK * uncert_weight) +
                         bg_weight * (val_iJk * (1. - uncert_weight) + val_iJK * uncert_weight);

    return value;
}

size_t LookupTable::totalDataPoints() const
{
    // number of computed points (0...data_max)
    return options.data_npoints;
}

size_t LookupTable::totalBgPoints() const
{
    // number of computed BG points (down...+-0...up)
    return options.bg_npoints_up + options.bg_npoints_down + 1;
}

size_t LookupTable::totalUncertPoints() const
{
    // number of computed sigma points
    return options.uncert_npoints;
}

size_t LookupTable::totalPoints() const
{
    return totalDataPoints() * totalBgPoints() * totalUncertPoints();
}

PriorMode LookupTable::prior() const
{
    return static_cast<PriorMode>(options.prior);
}

double LookupTable::calcFactor(const double index, const double factor_down, const double factor_up)
{
    // Based on the index (which can be eigher positive or negative), calculate a
    // factor used for setting the BG points:

    if (index >= 0)
    {
        // positive indices: use e.g. 1.05^n (result is between 1 and infinity)
        return pow(factor_up, index);
    }
    else
    {
        // negative index: use e.g. 2 - 1.05^n (result is between 1 and -infinity)

        // Note that values less than 0 do not make sense, but I needed the
        // behavior of tight spacing around 1 and symmetry... and invertibility.
        return 2.0 - pow(factor_down, -index);
    }
}

double LookupTable::unCalcFactor(const double factor, const double factor_down, const double factor_up)
{
    // Inverse of calcFactor: determine index (double) from given factor.

    // Read comments in calcFactor to understand unCalcFactor
    if (factor == 1)
    {
        return 0;
    }
    else if (factor > 1)
    {
        return log(factor) / log(factor_up);
    }
    else
    {
        return -log(2.0 - factor) / log(factor_down);
    }
}

void LookupTable::newTable(const float default_value)
{
    // if this assertion fails, we have a memory leak
    assert(buffer == nullptr);

    //  memory allocation and initialization
    buffer = new float[totalPoints()];
    for (size_t i = 0; i < totalPoints(); i++)
    {
        buffer[i] = default_value;
    }
}

void LookupTable::deleteTable()
{
    if (buffer != nullptr)
    {
        delete[] buffer;
        buffer = nullptr;
    }
}

size_t LookupTable::index(const size_t data_index, const size_t bg_index, const size_t uncert_index) const
{
    // if one of these assertions fails, we have a segfault imminent
    // we don't / can't compare >= 0 because size_t is *always* >= 0
    assert(data_index < totalDataPoints());
    assert(bg_index < totalBgPoints());
    assert(uncert_index < totalUncertPoints());

    return (data_index * totalUncertPoints() * totalBgPoints() + bg_index * totalUncertPoints() + uncert_index);
}

// main function
double lookup_p_convolution(const double N_obs,
                            const double N_SM,
                            const double error_parameter,
                            const bool debug,
                            const char *filename)
{

    static LookupTable table(debug);
    if (not table.isLoaded())
    {
        table.readFile(filename);
    }

    return table.lookup(N_obs, N_SM, error_parameter);
}
